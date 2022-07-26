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
      myHeartbeatFlag = true;
      std::unique_lock<std::mutex> lk(myMutex);
      if (!myCondVar.wait_for(lk, HEARTBEAT_UPDATE_INTERVAL_MS, [this](){ return myTargetUpdateFlag; }))
      {
         continue; // Heartbeat update interval timeout
      }
      if (myExitingFlag)
      {
         break;
      }

      if (myInTrackingMode)
      {
         continue; //TODO - Implement tracking over series of points
      }
      else
      {
         auto motionController = myMotionController.lock();
         motionController->moveHorizAngle(myTargetAzimuth);
         motionController->moveVertAngle(myTargetElevation);
         myTargetUpdateFlag = false;
      }
   }
}

void PositionManager::updatePosition(const CmdUpdatePosition& cmd)
{
   std::scoped_lock<std::mutex> lk(myMutex);
   myInTrackingMode = false;
   myTargetAzimuth = cmd.myThetaInDeg;
   myTargetElevation = cmd.myPhiInDeg;
   myTargetUpdateFlag = true;
   myCondVar.notify_one();
}

void PositionManager::trackTarget(const PositionTable& positions)
{
   std::scoped_lock<std::mutex> lk(myMutex);
   myInTrackingMode = true;
   myPositionTable = positions;
   myTargetUpdateFlag = true;
   myCondVar.notify_one();
}

void PositionManager::calibrate(const CmdCalibrate& cmd)
{
}