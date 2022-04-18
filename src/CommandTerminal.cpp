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
   std::stringstream ss(command);
   std::string baseCommand;
   ss >> baseCommand;
   if (!baseCommand.empty())
   {
      // Do processing based on selected command
      std::transform(baseCommand.begin(), baseCommand.end(), baseCommand.begin(),
         [](auto c){ return std::tolower(c); });
      if (baseCommand == "photo")
      {
         // Format -> photo <name>
         std::string name;
         ss >> name;
         if (!ss.eof())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unexpected parameter for photo command: " + command);
            return false;
         }
         CmdTakePhoto cmd(name);
         processCommand(static_cast<Command>(cmd));
      }
      else if (baseCommand == "video")
      {
         // Format -> video <name>
         std::string name;
         ss >> name;
         if (!ss.eof())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unexpected parameter for video command: " + command);
            return false;
         }
         CmdTakeVideo cmd(name);
         processCommand(static_cast<Command>(cmd));
      }
      else if (baseCommand == "timelapse")
      {
         // Format -> timelapse <name>
         std::string name;
         ss >> name;
         if (!ss.eof())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unexpected parameter for timelapse command: " + command);
            return false;
         }
         CmdTakeTimelapse cmd(name);
         processCommand(static_cast<Command>(cmd));
      }
      else if (baseCommand == "move")
      {
         // Format -> move <theta> <phi>
         double theta;
         double phi;
         ss >> theta >> phi;
         if (ss.fail())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to parse parameters for move command: " + command);
            return false;
         }
         if (!ss.eof())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unexpected parameter for move command: " + command);
            return false;
         }
         CmdUserMove cmd(theta, phi);
         processCommand(static_cast<Command>(cmd));
      }
      else if (baseCommand == "focus")
      {
         // Format -> focus <theta>
         double theta;
         ss >> theta;
         if (ss.fail())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to parse parameter for focus command: " + command);
            return false;
         }
         if (!ss.eof())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unexpected parameter for focus command: " + command);
            return false;
         }
         CmdUserFocus cmd(theta);
         processCommand(static_cast<Command>(cmd));
      }
      else if (baseCommand == "follow")
      {
         // Format -> follow <name> <duration>
         std::string name;
         uint64_t duration;
         ss >> name >> duration;
         if (ss.fail())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to parse parameters for follow command: " + command);
            return false;
         }
         if (!ss.eof())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unexpected parameter for follow command: " + command);
            return false;
         }
         CmdFollowTarget cmd(name, std::chrono::seconds(duration));
         processCommand(static_cast<Command>(cmd));         
      }
      else if (baseCommand == "goto")
      {
         // Format -> goto <name>
         std::string name;
         ss >> name;
         if (!ss.eof())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unexpected parameter for goto command: " + command);
            return false;
         }
         CmdGoToTarget cmd(name);
         processCommand(static_cast<Command>(cmd));         
      }
      else if (baseCommand == "search")
      {
         std::string option;
         ss >> option;
         // Do processing based on option
         if (option == "range")
         {
            // Format -> search range <range>
            double range;
            ss >> range;
            if (ss.fail())
            {
               myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to parse parameter for search radius command: " + command);
               return false;
            }
            if (!ss.eof())
            {
               myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unexpected parameter for search radius command: " + command);
               return false;
            }
            CmdSearchTarget cmd("None", range);
            processCommand(static_cast<Command>(cmd));
         }
         else if (option == "name")
         {
            // Format -> search name <name>
            std::string name;
            ss >> name;
            if (!ss.eof())
            {
               myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unexpected parameter for search name command: " + command);
               return false;
            }
            CmdSearchTarget cmd(name, 0.0);
            processCommand(static_cast<Command>(cmd));
         }
         else
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unknown option for search command: " + command);
            return false;
         }
      }
      else if (baseCommand == "calibrate")
      {
         if (!ss.eof())
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unexpected parameter for calibrate command: " + command);
            return false;
         }
         CmdCalibrate cmd;
         processCommand(static_cast<Command>(cmd));
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

bool CommandTerminal::processCommand(const Command& command)
{
   myLogger->log(mySubsystemName, LogCodeEnum::INFO, "Command Type: " + std::to_string(static_cast<unsigned int>(command.myCommandType)));
   return true;

}

