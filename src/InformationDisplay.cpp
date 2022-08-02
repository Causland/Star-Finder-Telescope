#include "InformationDisplay.hpp"
#include <algorithm>

const std::string IInformationDisplay::NAME{"InformationDisplay"};

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

void InformationDisplay::configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems)
{
   // Find each subsystem from the vector and store in the respective pointer
   // ICommandTerminal
   auto it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == ICommandTerminal::NAME; });
   if (it == subsystems.end())
   {
      myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Command Terminal interface pointer");
   }
   else
   {
      myCommandTerminal = std::dynamic_pointer_cast<ICommandTerminal>(*it);
      if (myCommandTerminal.expired())
      {
         myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Command Terminal");
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