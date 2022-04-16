#include "CommandTerminal.hpp"
#include <algorithm>
#include <iostream>
#include <istream>

const std::string ICommandTerminal::NAME{"CommandTerminal"};

void CommandTerminal::start()
{
   myThread = std::thread(&CommandTerminal::threadLoop, this);
}

void CommandTerminal::stop()
{
   myExitingFlag = true;
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
      if (myInformationDisplay == nullptr)
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
      if (myOpticsManager == nullptr)
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
      if (myPositionManager == nullptr)
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
      if (myStarTracker == nullptr)
      {
         myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Star Tracker");
      }
   }
}

void CommandTerminal::threadLoop()
{
   while (!myExitingFlag)
   {
      // Things for command terminal to do
      // Parse commands into a queue for validation and processing
      std::string input;
      std::getline(std::cin, input);
      if (!input.empty())
      {
         // Commands can be delimited by semicolons for sequential inputs
         std::istringstream iss(input);
         std::string command;
         while(std::getline(iss, command, ';'))
         {
            if (validateCommand(command))
            {
               myCommandQueue.push(command);
            }
            else
            {
               myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Invalid command " + command);
            }
         }
         // Process each command and take the specific action
         while (!myCommandQueue.empty())
         {
            processCommand(myCommandQueue.front());
            myCommandQueue.pop();
         }
      }
   }
}

bool CommandTerminal::validateCommand(const std::string& command)
{
   std::cout << "validating " << command << "\n";
   return true;
}

bool CommandTerminal::processCommand(const std::string& command)
{
   std::cout << "processing " << command << "\n";
   return true;

}

