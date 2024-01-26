#include "InformationDisplay.hpp"
#include "Logger.hpp"
#include "PositionManager.hpp"
#include "PropertyManager.hpp"

#include <algorithm>

void PositionManager::start()
{
   // Get all properties from the properties manager
   int64_t timeOffset{0};
   int64_t trajectorySamplePeriod{0};
   if (PropertyManager::getProperty("manual_move_time_offset_ms", &timeOffset))
   {
      myManualMoveTimeOffset = std::chrono::milliseconds(timeOffset);
   }
   else
   {
      LOG_ERROR("Unable to set property: manual_move_time_offset_ms. Using default " + std::to_string(myManualMoveTimeOffset.count()));
   }
   if (PropertyManager::getProperty("trajectory_sample_period_ms", &trajectorySamplePeriod))
   {
      myTrajectorySamplePeriod = std::chrono::milliseconds(trajectorySamplePeriod);
   }
   else
   {
      LOG_ERROR("Unable to set property: trajectory_sample_period_ms. Using default " + std::to_string(myTrajectorySamplePeriod.count()));
   }

   // Start the thread loop to begin subsystem behavior
   myExitingFlag = false;
   myThread = std::thread(&PositionManager::threadLoop, this);
}

void PositionManager::stop()
{
   myExitingFlag = true;
   myCondVar.notify_one();
   myThread.join();
}

void PositionManager::configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                                           static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems)
{
   // InformationDisplay
   myInformationDisplay = std::dynamic_pointer_cast<InformationDisplay>(
                                    subsystems[static_cast<int>(SubsystemEnum::INFORMATION_DISPLAY)]);
   if (myInformationDisplay.expired())
   {
      LOG_ERROR("Could not cast to Information Display");
   }
}

void PositionManager::updatePosition(const CmdUpdatePosition& cmd)
{
   std::scoped_lock<std::mutex> lock(myMutex);

   // Create a path with the current position and time and the target position with a small time offset
   std::vector<TrajectoryPoint> path{
      {Position{myCurrentPosition.myAzimuth, myCurrentPosition.myElevation}, std::chrono::system_clock::now()},
      {Position{cmd.myPosition}, std::chrono::system_clock::now() + myManualMoveTimeOffset}
   };

   calculateTrajectory(path);

   myTargetUpdateFlag = true;
   myCondVar.notify_one();
   LOG_INFO("Received move request: (az,el) | (" + std::to_string(myCurrentPosition.myAzimuth) + "," + std::to_string(myCurrentPosition.myElevation) + 
            ")->(" + std::to_string(cmd.myPosition.myAzimuth) + "," + std::to_string(cmd.myPosition.myElevation) + ")");
}

void PositionManager::trackTarget(std::vector<TrajectoryPoint>& path)
{
   std::scoped_lock<std::mutex> lock(myMutex);

   // Insert the current position and time at the start of the path
   path.insert(std::begin(path), {Position{myCurrentPosition.myAzimuth, myCurrentPosition.myElevation}, std::chrono::system_clock::now()});

   calculateTrajectory(path);

   myTargetUpdateFlag = true;
   myCondVar.notify_one();
   LOG_INFO("Received target track request: points=" + std::to_string(path.size()) + " start=(" + std::to_string(myCurrentPosition.myAzimuth) +
            "," + std::to_string(myCurrentPosition.myElevation) + ") end=(" + std::to_string(path.rbegin()->myPosition.myAzimuth) + 
            "," + std::to_string(path.rbegin()->myPosition.myElevation) + ")");
}

void PositionManager::calibrate(const CmdCalibrate& cmd)
{
}

void PositionManager::threadLoop()
{
   while (!myExitingFlag)
   {
      myHeartbeatFlag = true;
      std::unique_lock<std::mutex> lock{myMutex};
      if (!myCondVar.wait_for(lock, std::min(myTrajectoryUpdateInterval, HEARTBEAT_UPDATE_INTERVAL_MS), 
                              [this](){ return myTargetUpdateFlag || myExitingFlag; }))
      {
         continue; // Heartbeat update interval timeout
      }
      if (myExitingFlag)
      {
         break;
      }

      if (!myTrajectory.empty())
      {
         auto& trajPnt{myTrajectory.front()};
         auto currentTime{std::chrono::system_clock::now()};
         // If the current time is past the next trajectory point time, it is time to move the telescope
         if (currentTime >= trajPnt.myTime)
         {
            myMotionController->moveHorizAngle(Rotation{trajPnt.myPosition.myAzimuth, trajPnt.myVelocity.myVelAzimuth});
            myMotionController->moveVertAngle(Rotation{trajPnt.myPosition.myElevation, trajPnt.myVelocity.myVelElevation});
            myCurrentPosition.myAzimuth = trajPnt.myPosition.myAzimuth;
            myCurrentPosition.myElevation = trajPnt.myPosition.myElevation;
            
            auto infoDisp = myInformationDisplay.lock();
            if (infoDisp != nullptr)
            {
               infoDisp->updateMotion(trajPnt.myPosition, trajPnt.myVelocity);
            }

            myTrajectory.pop();
         }
         else
         {
            // Set the time interval to the next trajectory point to wake up the thread
            myTrajectoryUpdateInterval = std::chrono::duration_cast<std::chrono::milliseconds>(trajPnt.myTime - currentTime);
         }
      }
      else
      {
         // There are no more points along the trajectory. Resume waiting for another trajectory.
         myTargetUpdateFlag = false;
         myTrajectoryUpdateInterval = HEARTBEAT_UPDATE_INTERVAL_MS;
      }
   }
}

void PositionManager::calculateTrajectory(std::vector<TrajectoryPoint>& path)
{
   using std::chrono::milliseconds;
   using std::chrono::duration_cast;

   // Clear any existing trajectory from the system by swapping in an empty queue
   std::queue<TrajectoryPoint>().swap(myTrajectory);

   if (path.size() > 1)
   {
      PolyCoefs azCoefs;
      PolyCoefs elCoefs;

      auto prevIt{path.begin()};
      auto currIt{std::next(path.begin())};
      for (; currIt!=path.end(); ++currIt, ++prevIt)
      {
         const auto timeDiff{duration_cast<milliseconds>(currIt->myTime - prevIt->myTime)};
         const auto diffInSec{static_cast<double>(timeDiff.count()) * MILLISECONDS_TO_SECONDS};

         // Only set the target velocity through the path if not the end 
         // We want the final velocity to be 0
         if (std::next(currIt) != path.end())
         {
            currIt->myVelocity.myVelAzimuth = (currIt->myPosition.myAzimuth - prevIt->myPosition.myAzimuth) / diffInSec;
            currIt->myVelocity.myVelElevation = (currIt->myPosition.myElevation - prevIt->myPosition.myElevation) / diffInSec;
         }
         calculatePolynomialCoef(prevIt->myPosition, currIt->myPosition, prevIt->myVelocity, currIt->myVelocity, &azCoefs, &elCoefs);

         // Interpolate position and velocity at sub time intervals
         Position pos;
         Velocity vel;
         auto timePoint{prevIt->myTime};
         while (timePoint < currIt->myTime)
         {
            // Shift to a range between 0 and 1
            const auto scaledTime{(static_cast<double>(duration_cast<milliseconds>(timePoint - prevIt->myTime).count()) * MILLISECONDS_TO_SECONDS) / diffInSec};
            calculatePositionAndVelocity(scaledTime, azCoefs, elCoefs, &pos, &vel);
            myTrajectory.emplace(pos, vel, timePoint);
            timePoint += myTrajectorySamplePeriod;
         }

         timePoint = currIt->myTime;
         if (std::next(currIt) == path.end())
         {
            pos = currIt->myPosition;
            vel = Velocity{0.0, 0.0};
         }
         else
         {
            calculatePositionAndVelocity(1, azCoefs, elCoefs, &pos, &vel);
         }
         myTrajectory.emplace(pos, vel, timePoint);
      }
   }
   else
   {
      LOG_ERROR("Need to provide at least two positions in call to calculateTrajectory()");
      return;
   }
}

