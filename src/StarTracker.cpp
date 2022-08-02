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
   myCondVar.notify_one();
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
         auto generalCommand = myCommandQueue.front();
         myCommandQueue.pop();

         switch (generalCommand.myCommandType)
         {
         case CommandTypeEnum::GOTO_TARGET:
         {
            auto command = static_cast<CmdGoToTarget&>(generalCommand);
            auto gpsModule = myGpsModule.lock();
            gpsModule->getGpsPosition(&myGpsLat, &myGpsLong, &myGpsElev);
            auto starDatabase = myStarDatabase.lock();
            auto result = starDatabase->queryTargetPointing(command.myTargetName, std::chrono::system_clock::now(),
                                                               myGpsLong, myGpsLat, myGpsElev);
            if (result.myStatusCode == QueryResult::SUCCESS)
            {
               auto position = result.myPositionResults.front();
               auto positionManager = myPositionManager.lock();
               myLogger->log(mySubsystemName, LogCodeEnum::INFO, "Issuing goto command for target=" + command.myTargetName + " (az,el)=(" + 
                                                                  std::to_string(position.first.myAzimuth) + "," + std::to_string(position.first.myElevation) + ")");
               positionManager->updatePosition(CmdUpdatePosition(position.first.myAzimuth, position.first.myElevation));
            }
            else if (result.myStatusCode == QueryResult::INVALID_PARAM || result.myStatusCode == QueryResult::FAILURE)
            {
               myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Goto Target Result: " + result.myLogStatement);
            }
            break;
         }
         case CommandTypeEnum::FOLLOW_TARGET:
         {
            auto command = static_cast<CmdFollowTarget&>(generalCommand);
            auto gpsModule = myGpsModule.lock();
            gpsModule->getGpsPosition(&myGpsLat, &myGpsLong, &myGpsElev);
            auto starDatabase = myStarDatabase.lock();
            auto result = starDatabase->queryTargetPointingTrajectory(command.myTargetName, command.myStartTime, command.myStartTime + command.myDuration,
                                                                        myGpsLong, myGpsLat, myGpsElev);
            if (result.myStatusCode == QueryResult::SUCCESS)
            {
               auto positionManager = myPositionManager.lock();
               positionManager->trackTarget(result.myPositionResults);
            }
            else if (result.myStatusCode == QueryResult::INVALID_PARAM || result.myStatusCode == QueryResult::FAILURE)
            {
               myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Follow Target Result: " + result.myLogStatement);
            }
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