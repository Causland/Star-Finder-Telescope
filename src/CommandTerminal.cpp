#include "CommandTerminal.hpp"
#include "InformationDisplay.hpp"
#include "Logger.hpp"
#include "OpticsManager.hpp"
#include "PositionManager.hpp"
#include "StarTracker.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

void CommandTerminal::start()
{
   myExitingFlag = false;
   myInputWaitingThread = std::thread{&CommandTerminal::cinWaitThreadLoop, this};
   myThread = std::thread{&CommandTerminal::threadLoop, this};
}

void CommandTerminal::stop()
{
   myExitingFlag = true;
   myCondVar.notify_one();
   myInputWaitingThread.join();
   myThread.join();
}

void CommandTerminal::configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                                           static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems)
{
   // Find each subsystem from the vector and store in the respective pointer
   // IInformationDisplay
   myInformationDisplay = std::dynamic_pointer_cast<InformationDisplay>(
                                    subsystems[static_cast<int>(SubsystemEnum::INFORMATION_DISPLAY)]);
   if (myInformationDisplay.expired())
   {
      LOG_ERROR("Could not cast to Information Display");
   }
   // IOpticsManager
   myOpticsManager = std::dynamic_pointer_cast<OpticsManager>(
                              subsystems[static_cast<int>(SubsystemEnum::OPTICS_MANAGER)]);
   if (myOpticsManager.expired())
   {
      LOG_ERROR("Could not cast to Optics Manager");
   }
   // IPositionManager
   myPositionManager = std::dynamic_pointer_cast<PositionManager>(
                                 subsystems[static_cast<int>(SubsystemEnum::POSITION_MANAGER)]);
   if (myPositionManager.expired())
   {
      LOG_ERROR("Could not cast to Position Manager");
   }
   // IStarTracker
   myStarTracker = std::dynamic_pointer_cast<StarTracker>(
                              subsystems[static_cast<int>(SubsystemEnum::STAR_TRACKER)]);
   if (myStarTracker.expired())
   {
      LOG_ERROR("Could not cast to Star Tracker");
   }
}

void CommandTerminal::threadLoop()
{
   while (!myExitingFlag)
   {
      myHeartbeatFlag = true;
      std::unique_lock lock{myMutex};
      if (!myCondVar.wait_for(lock, HEARTBEAT_UPDATE_INTERVAL_MS, [this](){ return !myCommandQueue.empty() || myExitingFlag; }))
      {
         continue; // Heartbeat update interval timeout
      }
      if (myExitingFlag)
      {
         break;
      }

      // Process each command and take the specified action
      while (!myCommandQueue.empty())
      {
         std::istringstream stream{myCommandQueue.front()};
         interpretCommand(stream);
         myCommandQueue.pop();
      }
   }
   *myExitingSignal = true;
}

void CommandTerminal::cinWaitThreadLoop()
{
   while(!myExitingFlag)
   {
      // Parse commands into a queue for validation and processing
      std::string input;
      std::getline(std::cin, input);
      if (!input.empty())
      {
         LOG_INFO("User input: " + input);
         auto infoDisp{myInformationDisplay.lock()};
         if (infoDisp != nullptr)
         {
            infoDisp->updateLastCommand(input);
         }

         // Commands can be delimited by semicolons for sequential inputs
         std::scoped_lock lock(myMutex);
         std::istringstream iss(input);
         std::string command;
         while(std::getline(iss, command, ';'))
         {
            if (command == "exit")
            {
               myExitingFlag = true;
            }

            myCommandQueue.push(std::move(command));
         }
         myCondVar.notify_one(); // Notify when done parsing all input
      } 
   }
}

void CommandTerminal::interpretCommand(std::istream& commandStream)
{
   // Extract base command and preprocess
   std::string command;
   commandStream >> command;

   std::transform(std::begin(command), std::end(command), std::begin(command),
                     [](auto c){ return std::tolower(c); }); // NOLINT(readability-identifier-length)

   if (command == "photo")
   {
      processPhotoCmd(commandStream);
   }
   else if (command == "video")
   {
      processVideoCmd(commandStream);
   }
   else if (command == "timelapse")
   {
      processTimelapseCmd(commandStream);
   }
   else if (command == "move")
   {
      processMoveCmd(commandStream);
   }
   else if (command == "focus")
   {
      processFocusCmd(commandStream);
   }
   else if (command == "follow")
   {
      processFollowCmd(commandStream);
   }
   else if (command == "goto")
   {
      processGoToCmd(commandStream);
   }
   else if (command == "search")
   {
      processSearchCmd(commandStream);
   }
   else if (command == "calibrate")
   {
      processCalibrateCmd(commandStream);
   }
   else 
   {
      LOG_ERROR("Unknown command name " + command);
   }
}

void CommandTerminal::processPhotoCmd(std::istream& commandStream)
{
   // Format -> photo <name>
   std::string name;
   commandStream >> name;
   auto opticsManager{myOpticsManager.lock()};
   if (opticsManager != nullptr)
   {
      opticsManager->takePhoto(CmdTakePhoto{name});
   }
   else 
   {
      LOG_ERROR("OpticsManager pointer has expired");
   }
}

void CommandTerminal::processVideoCmd(std::istream& commandStream)
{
   // Format -> video <name> <duration>
   std::string name;
   uint64_t duration{0};
   if (commandStream >> name >> duration)
   {
      auto opticsManager{myOpticsManager.lock()};
      if (opticsManager != nullptr)
      {
         opticsManager->takeVideo(CmdTakeVideo{name, std::chrono::seconds{duration}});
      }
      else 
      {
         LOG_ERROR("OpticsManager pointer has expired");
      }
   }
   else 
   {
      LOG_ERROR("Unable to parse parameters for video command");
   }
}

void CommandTerminal::processTimelapseCmd(std::istream& commandStream)
{
   // Format -> timelapse <name> <duration> <rate>
   std::string name;
   uint64_t duration{0};
   double rate{0.0};
   if (commandStream >> name >> duration >> rate)
   {
      auto opticsManager{myOpticsManager.lock()};
      if (opticsManager != nullptr)
      {
         opticsManager->takeTimelapse(CmdTakeTimelapse{name, std::chrono::minutes{duration}, rate});
      }
      else 
      {
         LOG_ERROR("OpticsManager pointer has expired");
      }
   }
   else 
   {
      LOG_ERROR("Unable to parse parameters for timelapse command");
   }
}

void CommandTerminal::processMoveCmd(std::istream& commandStream)
{
   // Format -> move <theta> <phi>
   double theta{0.0};
   double phi{0.0};
   if (commandStream >> theta >> phi)
   {
      auto positionManager{myPositionManager.lock()};
      if (positionManager != nullptr)
      {
         positionManager->updatePosition(CmdUpdatePosition{Position{theta, phi}});
      }
      else 
      {
         LOG_ERROR("PositionManager pointer has expired");
      }
   }
   else 
   {
      LOG_ERROR("Unable to parse parameters for move command");
   }
}

void CommandTerminal::processFocusCmd(std::istream& commandStream)
{
   // Format -> focus <theta>
   double theta{0.0};
   if (commandStream >> theta)
   {
      auto opticsManager{myOpticsManager.lock()};
      if (opticsManager != nullptr)
      {
         opticsManager->userChangeFocus(CmdUserFocus{theta});
      }
      else 
      {
         LOG_ERROR("OpticsManager pointer has expired");
      }
   }
   else 
   {
      LOG_ERROR("Unable to parse parameters for focus command");
   }
}

void CommandTerminal::processFollowCmd(std::istream& commandStream)
{
   // Format -> follow <name> <duration>
   std::string name;
   uint64_t duration{0};
   if (commandStream >> name >> duration)
   {
      auto starTracker{myStarTracker.lock()};
      if (starTracker != nullptr)
      {
         starTracker->trackTarget(CmdFollowTarget{name, std::chrono::seconds{duration}});
      }
      else 
      {
         LOG_ERROR("StarTracker pointer has expired");
      }
   }
   else 
   {
      LOG_ERROR("Unable to parse parameters for follow command");
   }
}

void CommandTerminal::processGoToCmd(std::istream& commandStream)
{
   // Format -> goto <name>
   std::string name;
   commandStream >> name;
   auto starTracker{myStarTracker.lock()};
   if (starTracker != nullptr)
   {
      starTracker->pointToTarget(CmdGoToTarget{name});
   }
   else 
   {
      LOG_ERROR("StarTracker pointer has expired");
   }
}

void CommandTerminal::processSearchCmd(std::istream& commandStream)
{
   // Search can be used with range, name, brightness option which are then followed by parameters
   std::string option;
   commandStream >> option;
   double range{0.0};
   double luminosity{0.0};
   std::string name{"None"};
   if (option == "range")
   {
      if (!(commandStream >> range))
      {
         LOG_ERROR("Unable to parse parameters for search range command");
         return;
      }
   }
   else if (option == "name")
   {
      commandStream >> name;
   }
   else if (option == "brightness")
   {
      if (!(commandStream >> luminosity))
      {
         LOG_ERROR("Unable to parse parameters for search brightness command");
         return;
      }
   }
   else
   {
      LOG_ERROR("Unknown option " + option + " for search command");
   }
   auto starTracker{myStarTracker.lock()};
   if (starTracker != nullptr)
   {
      const SearchTargetParams params{name, range, luminosity};
      starTracker->searchForTargets(CmdSearchTarget{params}); 
   }
   else 
   {
      LOG_ERROR("StarTracker pointer has expired");
   }
}

void CommandTerminal::processCalibrateCmd(std::istream& commandStream)
{
   auto positionManager{myPositionManager.lock()};
   if (positionManager != nullptr)
   {
      positionManager->calibrate(CmdCalibrate{});
   }
   else 
   {
      LOG_ERROR("PositionManager pointer has expired");
   }
}

