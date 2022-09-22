#include "CommandTerminal.hpp"
#include "InformationDisplay.hpp"
#include "Logger.hpp"
#include "OpticsManager.hpp"
#include "PositionManager.hpp"
#include "StarTracker.hpp"
#include <algorithm>

const std::string InformationDisplay::NAME{"InformationDisplay"};

void InformationDisplay::start()
{
   myThread = std::thread(&InformationDisplay::threadLoop, this);
}

void InformationDisplay::stop()
{
   myExitingFlag = true;
   myCondVar.notify_one();
   myThread.join();
}

void InformationDisplay::configureSubsystems(const std::vector<std::shared_ptr<Subsystem>>& subsystems)
{
   // Find each subsystem from the vector and store in the respective pointer
   // ICommandTerminal
   auto it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == CommandTerminal::NAME; });
   if (it == subsystems.end())
   {
      Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Command Terminal pointer");
   }
   else
   {
      myCommandTerminal = std::dynamic_pointer_cast<CommandTerminal>(*it);
      if (myCommandTerminal.expired())
      {
         Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Command Terminal");
      }
   }
   // IOpticsManager
   it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == OpticsManager::NAME; });
   if (it == subsystems.end())
   {
      Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Optics Manager pointer");
   }
   else
   {
      myOpticsManager = std::dynamic_pointer_cast<OpticsManager>(*it);
      if (myOpticsManager.expired())
      {
         Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Optics Manager");
      }
   }
   // IPositionManager
   it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == PositionManager::NAME; });
   if (it == subsystems.end())
   {
      Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Position Manager pointer");
   }
   else
   {
      myPositionManager = std::dynamic_pointer_cast<PositionManager>(*it);
      if (myPositionManager.expired())
      {
         Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Position Manager");
      }
   }
   // IStarTracker
   it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == StarTracker::NAME; });
   if (it == subsystems.end())
   {
      Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Star Tracker pointer");
   }
   else
   {
      myStarTracker = std::dynamic_pointer_cast<StarTracker>(*it);
      if (myStarTracker.expired())
      {
         Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Star Tracker");
      }
   }
}

void InformationDisplay::displaySearchResults(const std::string& displayString)
{
   std::cout << displayString << "\n";
}

void InformationDisplay::threadLoop()
{
   while (!myExitingFlag)
   {
      // Things for information display to do
      // Query desired telemetry information from multiple subsystems
      // Display information in an easily viewable format
      // Select what information is displayed
      myHeartbeatFlag = true;
      std::unique_lock<std::mutex> lk(myMutex);
      if (!myCondVar.wait_for(lk, HEARTBEAT_UPDATE_INTERVAL_MS, [this](){ return false || myExitingFlag; }))
      {
         continue; // Heartbeat update interval timeout
      }
      if (myExitingFlag)
      {
         break;
      }
   }
}