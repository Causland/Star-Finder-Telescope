#include "PositionManager.hpp"
#include <algorithm>

const std::string IPositionManager::NAME{"PositionManager"};

void PositionManager::start()
{
   myThread = std::thread(&PositionManager::threadLoop, this);
}

void PositionManager::stop()
{
   myExitingFlag = true;
   myThread.join();
}

void PositionManager::configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems)
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
}

void PositionManager::threadLoop()
{
   while (!myExitingFlag)
   {
      // Things for position manager to do
      // - Listen for telescope position update commands from command interface
      // - Listen for star position update commands from star tracker
      // - Calculate motion required to move to new position
      // - Command the motion controller for telescope pointing
   }
}

void PositionManager::userChangePosition(double theta, double phi)
{

}

void PositionManager::pointAtTarget(StarPosition position)
{
   
}