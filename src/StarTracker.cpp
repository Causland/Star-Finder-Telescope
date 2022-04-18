#include "StarTracker.hpp"
#include <algorithm>

const std::string IStarTracker::NAME{"StarTracker"};

void StarTracker::start()
{
   myThread = std::thread(&StarTracker::threadLoop, this);
}

void StarTracker::stop()
{
   myExitingFlag = true;
   myThread.join();
}

void StarTracker::configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems)
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
         myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Position Manager");
      }
   }
}

void StarTracker::threadLoop()
{
   while (!myExitingFlag)
   {
      // Things for Star Tracker to do
      // - Wait for particular star input
      //         - If tracking mode, set update frequency and query database for position at Hz
      //         - If position query mode, get the position from the database
      // - Inform position manager of new coordinates
   }
}

void StarTracker::pointToTarget(std::string targetName)
{

}

void StarTracker::trackTarget(std::string targetName, uint16_t updateFreqInHz)
{

}

void StarTracker::queryTargetPosition(std::string targetName)
{

}

void StarTracker::queryTargetsWithinRange(double rangeInLightMinutes)
{
   
}