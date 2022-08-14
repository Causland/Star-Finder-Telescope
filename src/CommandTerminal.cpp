#include "CommandTerminal.hpp"
#include "InformationDisplay.hpp"
#include "OpticsManager.hpp"
#include "PositionManager.hpp"
#include "StarTracker.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

const std::string CommandTerminal::NAME{"CommandTerminal"};

void CommandTerminal::start()
{
   myExitingFlag = false;
   myInputWaitingThread = std::thread(&CommandTerminal::cinWaitThreadLoop, this);
   myThread = std::thread(&CommandTerminal::threadLoop, this);
}

void CommandTerminal::stop()
{
   myExitingFlag = true;
   myCondVar.notify_one();
   myInputWaitingThread.join();
   myThread.join();
}

void CommandTerminal::configureSubsystems(const std::vector<std::shared_ptr<Subsystem>>& subsystems)
{
   // Find each subsystem from the vector and store in the respective pointer
   // IInformationDisplay
   auto it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == InformationDisplay::NAME; });
   if (it == subsystems.end())
   {
      myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Information Display pointer");
   }
   else
   {
      myInformationDisplay = std::dynamic_pointer_cast<InformationDisplay>(*it);
      if (myInformationDisplay.expired())
      {
         myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Information Display");
      }
   }
   // IOpticsManager
   it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == OpticsManager::NAME; });
   if (it == subsystems.end())
   {
      myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Optics Manager pointer");
   }
   else
   {
      myOpticsManager = std::dynamic_pointer_cast<OpticsManager>(*it);
      if (myOpticsManager.expired())
      {
         myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Optics Manager");
      }
   }
   // IPositionManager
   it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == PositionManager::NAME; });
   if (it == subsystems.end())
   {
      myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Position Manager pointer");
   }
   else
   {
      myPositionManager = std::dynamic_pointer_cast<PositionManager>(*it);
      if (myPositionManager.expired())
      {
         myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Position Manager");
      }
   }
   // IStarTracker
   it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == StarTracker::NAME; });
   if (it == subsystems.end())
   {
      myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Star Tracker pointer");
   }
   else
   {
      myStarTracker = std::dynamic_pointer_cast<StarTracker>(*it);
      if (myStarTracker.expired())
      {
         myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Star Tracker");
      }
   }
}

void CommandTerminal::threadLoop()
{
   while (!myExitingFlag)
   {
      myHeartbeatFlag = true;
      std::unique_lock lk(myMutex);
      if (!myCondVar.wait_for(lk, HEARTBEAT_UPDATE_INTERVAL_MS, [this](){ return !myCommandQueue.empty() || myExitingFlag; }))
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
         auto command = myCommandQueue.front();
         myCommandQueue.pop();

         interpretCommand(command);
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
         myLogger->log(mySubsystemName, LogCodeEnum::INFO, "User input: " + input);
         // Commands can be delimited by semicolons for sequential inputs
         std::istringstream iss(input);
         std::string command;
         while(std::getline(iss, command, ';'))
         {
            // Peek at the value of command to determine if exit
            if (input == "exit")
            {
               myExitingFlag = true;
               myCondVar.notify_one();
               break;
            }
            else
            {
               std::scoped_lock lk(myMutex);
               myCommandQueue.push(command);
            }
         }
         myCondVar.notify_one(); // Notify when done parsing all input
      } 
   }
}

bool CommandTerminal::interpretCommand(const std::string& command)
{
   // Commands are space delimited strings starting with a base command keyword (ie. photo/move)
   // followed by the necessary parameters for the command.
   auto pos = command.find(' ');
   std::string baseCommand = command.substr(0, pos);
   std::string params;
   if (pos == std::string::npos)
   {
      params = "";
   }
   else
   {
      params = command.substr(pos + 1);
   }
   if (!baseCommand.empty())
   {
      // Do processing based on selected command
      std::transform(baseCommand.begin(), baseCommand.end(), baseCommand.begin(),
         [](auto c){ return std::tolower(c); });
      if (baseCommand == "photo")
      {
         // Format -> photo <name>
         std::string name;
         if (!validateParameters(params, name))
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to process photo command");
            return false;
         }
         if (myOpticsManager.expired())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "OpticsManager pointer has expired");
            return false;
         }
         auto opticsManager = myOpticsManager.lock();
         opticsManager->takePhoto(CmdTakePhoto(name));
      }
      else if (baseCommand == "video")
      {
         // Format -> video <name> <duration>
         std::string name;
         uint64_t duration;
         if (!validateParameters(params, name, duration))
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to process video command");
            return false;
         }
         if (myOpticsManager.expired())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "OpticsManager pointer has expired");
            return false;
         }
         auto opticsManager = myOpticsManager.lock();
         opticsManager->takeVideo(CmdTakeVideo(name, std::chrono::seconds(duration)));
      }
      else if (baseCommand == "timelapse")
      {
         // Format -> timelapse <name> <duration> <rate>
         std::string name;
         uint64_t duration;
         double rate;
         if (!validateParameters(params, name, duration, rate))
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to process timelapse command");
            return false;
         }
         if (myOpticsManager.expired())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "OpticsManager pointer has expired");
            return false;
         }
         auto opticsManager = myOpticsManager.lock();
         opticsManager->takeTimelapse(CmdTakeTimelapse(name, std::chrono::minutes(duration), rate));
      }
      else if (baseCommand == "move")
      {
         // Format -> move <theta> <phi>
         double theta;
         double phi;
         if (!validateParameters(params, theta, phi))
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to process move command");
            return false;
         }
         if (myPositionManager.expired())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "PositionManager pointer has expired");
            return false;
         }
         auto positionManager = myPositionManager.lock();
         positionManager->updatePosition(CmdUpdatePosition(theta, phi));
      }
      else if (baseCommand == "focus")
      {
         // Format -> focus <theta>
         double theta;
         if (!validateParameters(params, theta))
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to process focus command");
            return false;
         }
         if (myOpticsManager.expired())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "OpticsManager pointer has expired");
            return false;
         }
         auto opticsManager = myOpticsManager.lock();
         opticsManager->userChangeFocus(CmdUserFocus(theta));
      }
      else if (baseCommand == "follow")
      {
         // Format -> follow <name> <duration>
         std::string name;
         uint64_t duration;
         if (!validateParameters(params, name, duration))
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to process follow command");
            return false;
         }
         if (myStarTracker.expired())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "StarTracker pointer has expired");
            return false;
         }
         auto starTracker = myStarTracker.lock();
         starTracker->trackTarget(CmdFollowTarget(name, std::chrono::seconds(duration)));       
      }
      else if (baseCommand == "goto")
      {
         // Format -> goto <name>
         std::string name;
         if (!validateParameters(params, name))
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to process goto command");
            return false;
         }
         if (myStarTracker.expired())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "StarTracker pointer has expired");
            return false;
         }
         auto starTracker = myStarTracker.lock();
         starTracker->pointToTarget(CmdGoToTarget(name));           
      }
      else if (baseCommand == "search")
      {
         // Search can be used with range, name, brightness option which are then followed by parameters
         pos = params.find(' ');
         std::string option = params.substr(0, pos);
         if (pos == std::string::npos)
         {
            params = "";
         }
         else
         {
            params = params.substr(pos + 1);
         }
         // Do processing based on option
         CmdSearchTarget cmd;
         double range = 0.0;
         double luminosity = 0.0;
         std::string name = "None";
         if (option == "range")
         {
            // Format -> search range <range>
            if (!validateParameters(params, range))
            {
               myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to process search range command");
               return false;
            }
         }
         else if (option == "name")
         {
            // Format -> search name <name>
            if (!validateParameters(params, name))
            {
               myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to process search name command");
               return false;
            }
         }
         else if (option == "brightness")
         {
            // Format -> search brightness <brightness>
            if (!validateParameters(params, luminosity))
            {
               myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to process search range command");
               return false;
            }
         }
         else
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unknown option for search command: " + command);
            return false;
         }
         if (myStarTracker.expired())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "StarTracker pointer has expired");
            return false;
         }
         auto starTracker = myStarTracker.lock();
         starTracker->searchForTargets(CmdSearchTarget(name, range, luminosity));   
      }
      else if (baseCommand == "calibrate")
      {
         if (!validateParameters(params))
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to process calibrate command");
            return false;
         }
         if (myPositionManager.expired())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "PositionManager pointer has expired");
            return false;
         }
         auto positionManager = myPositionManager.lock();
         positionManager->calibrate(CmdCalibrate());
      }
      else
      {
         myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unknown command " + command);
         return false;
      }
   }
   else
   {
      myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Attempted to process empty command");
      return false;
   }
   return true;
}

