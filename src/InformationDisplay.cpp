#include "InformationDisplay.hpp"
#include "Logger.hpp"
#include "PropertyManager.hpp"
#include <algorithm>
#include <exception>
#include <iomanip>

constexpr uint32_t SEC_PER_MIN{60};
const std::string InformationDisplay::NAME{"InformationDisplay"};

void InformationDisplay::start()
{
   // Get display refresh rate from the properties manager
   int64_t refreshRate{0};
   if (PropertyManager::getProperty("display_refresh_rate_ms", &refreshRate))
   {
      myRefreshRate = std::chrono::milliseconds(refreshRate);
   }
   else
   {
      LOG_ERROR(
                     "Unable to set property: display_refresh_rate_ms. Using default " + std::to_string(myRefreshRate.count()));
   }

   myThread = std::thread(&InformationDisplay::threadLoop, this);
}

void InformationDisplay::stop()
{
   myExitingFlag = true;
   myCondVar.notify_one();
   myThread.join();
}

void InformationDisplay::updateMotion(const Position& pos, const Velocity& vel)
{
   std::scoped_lock lk{myMutex};
   myPosition = pos;
   myVelocity = vel;
   myCondVar.notify_one();   
}

void InformationDisplay::updateSearchResults(const std::string& searchResults)
{
   std::scoped_lock lk{myMutex};
   mySearchResults = searchResults;
   myCondVar.notify_one();   
}

void InformationDisplay::updateLastCommand(const std::string& command)
{
   std::scoped_lock lk{myMutex};
   myLastCommand = command;
   myCondVar.notify_one();   
}

void InformationDisplay::threadLoop()
{
   while (!myExitingFlag)
   {
      myHeartbeatFlag = true;
      std::unique_lock<std::mutex> lk(myMutex);
      if (!myCondVar.wait_for(lk, std::min(myTimeToRefresh, HEARTBEAT_UPDATE_INTERVAL_MS), [this](){ return myExitingFlag.load(); }))
      {
         const auto now{std::chrono::system_clock::now()};
         const auto timeSinceRefresh{std::chrono::duration_cast<std::chrono::milliseconds>(now - myLastRefreshTime)};
         if (timeSinceRefresh >= myRefreshRate)
         {
            // We need to update the screen with new data
            updateDisplay();
            myLastRefreshTime = now;
         }
         myTimeToRefresh = myRefreshRate - timeSinceRefresh;
         continue;
      }
      if (myExitingFlag)
      {
         break;
      }
   }
}

void InformationDisplay::updateDisplay()
{
   myDisplayFile.open(DISPLAY_OUTPUT_FILE);
   if (!myDisplayFile.is_open())
   {
      LOG_ERROR("Unable to open display for writing");
      return;
   }

   auto now{std::chrono::system_clock::now()};
   auto uptime{std::chrono::duration_cast<std::chrono::seconds>(now - myStartTime)};

   // Format all the data and write to output file
   myDisplayFile << "=======================================\n"
                 << "    Star Finder Telescope Telemetry    \n"
                 << "=======================================\n"
                 << std::setfill('0')
                 << "Uptime: " << std::setw(2) << uptime.count() / SEC_PER_MIN << ":" 
                 << std::setw(2) << uptime.count() % SEC_PER_MIN << "\n"
                 << "---------------------------------------\n"
                 << "           Position Manager            \n"
                 << std::setfill(' ') << std::fixed << std::internal
                 << "    Az: " << std::setprecision(2) << std::setw(7) << myPosition.myAzimuth 
                 <<    "(deg)       El: " << std::setprecision(2) << std::setw(7) << myPosition.myElevation << "(deg)\n"
                 << "Vel_Az: " << std::setprecision(2) << std::setw(7) << myVelocity.myVelAzimuth 
                 << "(deg/s) Vel_El: " << std::setprecision(2) << std::setw(7) << myVelocity.myVelElevation << "(deg/s)\n"
                 << "---------------------------------------\n"
                 << "           Motion Controller           \n"
                 << myMotionController->getDisplayInfo() << "\n"
                 << "---------------------------------------\n"
                 << "              GPS Module               \n"
                 << myGpsModule->getDisplayInfo() << "\n"
                 << "---------------------------------------\n"
                 << "             Star Database             \n"
                 << myStarDatabase->getDisplayInfo() << "\n"
                 << "Last Search Result: " << mySearchResults << "\n"
                 << "---------------------------------------\n"
                 << "           Command Terminal            \n"
                 << "Last Command: " << myLastCommand << "\n"
                 << "---------------------------------------\n";

   // Flush the file and close
   myDisplayFile.flush();
   myDisplayFile.close();
}