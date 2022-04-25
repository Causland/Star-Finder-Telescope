#include "CommandTerminal.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

const std::string ICommandTerminal::NAME{"CommandTerminal"};

void CommandTerminal::start()
{
   myThread = std::thread(&CommandTerminal::threadLoop, this);
}

void CommandTerminal::stop()
{
   myExitingFlag = true;
   myInputWaitingThread.join();
   myThread.join();
}

void CommandTerminal::configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems)
{
   // Find each subsystem from the vector and store in the respective pointer
   // IInformationDisplay
   auto it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == IInformationDisplay::NAME; });
   if (it == subsystems.end())
   {
      myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Information Display pointer");
   }
   else
   {
      myInformationDisplay = std::dynamic_pointer_cast<IInformationDisplay>(*it);
      if (myInformationDisplay.expired())
      {
         myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Information Display");
      }
   }
   // IOpticsManager
   it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == IOpticsManager::NAME; });
   if (it == subsystems.end())
   {
      myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Optics Manager interface pointer");
   }
   else
   {
      myOpticsManager = std::dynamic_pointer_cast<IOpticsManager>(*it);
      if (myOpticsManager.expired())
      {
         myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Optics Manager");
      }
   }
   // IPositionManager
   it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == IPositionManager::NAME; });
   if (it == subsystems.end())
   {
      myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Position Manager interface pointer");
   }
   else
   {
      myPositionManager = std::dynamic_pointer_cast<IPositionManager>(*it);
      if (myPositionManager.expired())
      {
         myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Postion Manager");
      }
   }
   // IStarTracker
   it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == IStarTracker::NAME; });
   if (it == subsystems.end())
   {
      myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Star Tracker interface pointer");
   }
   else
   {
      myStarTracker = std::dynamic_pointer_cast<IStarTracker>(*it);
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
      if (!myCondVar.wait_for(lk, HEARTBEAT_UPDATE_INTERVAL_MS, [this](){ return !myCommandQueue.empty(); }))
      {
         continue; // Heartbeat update interval timeout
      }
      if (myExitingFlag)
         break;

      // Process each command and take the specified action
      while (!myCommandQueue.empty())
      {
         auto command = myCommandQueue.front();
         myCommandQueue.pop();

         // Command processing may take time if associated with a move, so unlock mutex
         lk.unlock();
            interpretCommand(command);
         lk.lock();
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
               {
                  std::scoped_lock lk(myMutex);
                  myCommandQueue.push(command);
               }
            }
         }
         myCondVar.notify_one(); // Notify when done parsing all input
      } 
   }
}

bool CommandTerminal::interpretCommand(const std::string& command)
{
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
         positionManager->userChangePosition(CmdUserMove(theta, phi));
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
         starTracker->queryTarget(CmdSearchTarget(name, range));   
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

