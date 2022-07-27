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
         auto generalCommand = myCommandQueue.front();
         myCommandQueue.pop();

         switch (generalCommand.myCommandType)
         {
         case CommandTypeEnum::GOTO_TARGET:
         {
            auto command = static_cast<CmdGoToTarget&>(generalCommand);
            double azimuth = 0.0;
            double elevation = 0.0;
            auto gpsModule = myGpsModule.lock();
            gpsModule->getGpsPosition(&myGpsLat, &myGpsLong, &myGpsElev);
            auto starDatabase = myStarDatabase.lock();
            auto result = starDatabase->queryTargetPointing(command.myTargetName, std::chrono::system_clock::now(),
                                                               myGpsLong, myGpsLat, myGpsElev, &azimuth, &elevation);
            auto positionManager = myPositionManager.lock();
            positionManager->updatePosition(CmdUpdatePosition(azimuth, elevation));
            break;
         }
         case CommandTypeEnum::FOLLOW_TARGET:
         {
            auto command = static_cast<CmdFollowTarget&>(generalCommand);
            break;
         }
         case CommandTypeEnum::SEARCH_TARGET:
         {
            auto command = static_cast<CmdSearchTarget&>(generalCommand);
            break;
         }
         default:
         {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Invalid command in queue: " + generalCommand.toString());
            break;
         }
         }
      }
   }
}

void StarTracker::pointToTarget(const CmdGoToTarget& cmd)
{
   myCommandQueue.push(cmd);
   myCondVar.notify_one();
}

void StarTracker::trackTarget(const CmdFollowTarget& cmd)
{
   myCommandQueue.push(cmd);
   myCondVar.notify_one();  
}

void StarTracker::queryTarget(const CmdSearchTarget& cmd)
{
   myCommandQueue.push(cmd);
   myCondVar.notify_one();
}