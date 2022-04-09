#include "CommandTerminal.hpp"
#include <algorithm>

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
      // - Process entered commands
      //      - Validate the command
      // - Pass along command to desired subsystem
      // - Process series of timed commands from file
   }
}

