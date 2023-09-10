#include "InformationDisplay.hpp"
#include "Logger.hpp"
#include "PositionManager.hpp"
#include "PropertyManager.hpp"
#include <algorithm>

const std::string PositionManager::NAME{"PositionManager"};

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
      LOG_ERROR(
                     "Unable to set property: manual_move_time_offset_ms. Using default " + std::to_string(myManualMoveTimeOffset.count()));
   }
   if (PropertyManager::getProperty("trajectory_sample_period_ms", &trajectorySamplePeriod))
   {
      myTrajectorySamplePeriod = std::chrono::milliseconds(trajectorySamplePeriod);
   }
   else
   {
      LOG_ERROR(
                     "Unable to set property: trajectory_sample_period_ms. Using default " + std::to_string(myTrajectorySamplePeriod.count()));
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
   // Find each subsystem from the vector and store in the respective pointer
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
   std::scoped_lock<std::mutex> lk(myMutex);
   // Create a position series vector with the current position and time and the target position with a small time offset
   std::vector<std::pair<Position, std::chrono::system_clock::time_point>> positions;
   positions.emplace_back(Position(myCurrentAzimuth, myCurrentElevation), std::chrono::system_clock::now());
   positions.emplace_back(Position(cmd.myThetaInDeg, cmd.myPhiInDeg), std::chrono::system_clock::now() + myManualMoveTimeOffset);
   calculateTrajectory(positions);
   myTargetUpdateFlag = true;
   myCondVar.notify_one();
   LOG_INFO("Received move request: (az,el) | (" + std::to_string(myCurrentAzimuth) + "," + std::to_string(myCurrentElevation) + 
                                                         ")->(" + std::to_string(cmd.myThetaInDeg) + "," + std::to_string(cmd.myPhiInDeg) + ")");
}

void PositionManager::trackTarget(std::vector<std::pair<Position, std::chrono::system_clock::time_point>>* positions)
{
   std::scoped_lock<std::mutex> lk(myMutex);
   // Insert the current position and time at the front of the position series vector.
   const auto currentPosition{std::pair<Position, std::chrono::system_clock::time_point>(
                                 Position(myCurrentAzimuth, myCurrentElevation), 
                                 std::chrono::system_clock::now())};
   positions->insert(positions->begin(), currentPosition);
   calculateTrajectory(*positions);
   myTargetUpdateFlag = true;
   myCondVar.notify_one();
   LOG_INFO("Received target track request: points=" + std::to_string(positions->size()) + " start=(" + std::to_string(myCurrentAzimuth) +
                                                          "," + std::to_string(myCurrentElevation) + ") end=(" + std::to_string(positions->rbegin()->first.myAzimuth) + 
                                                          "," + std::to_string(positions->rbegin()->first.myElevation) + ")");
}

void PositionManager::calibrate(const CmdCalibrate& cmd)
{
}

void PositionManager::threadLoop()
{
   while (!myExitingFlag)
   {
      myHeartbeatFlag = true;
      std::unique_lock<std::mutex> lk(myMutex);
      if (!myCondVar.wait_for(lk, std::min(myTrajectoryUpdateInterval, HEARTBEAT_UPDATE_INTERVAL_MS), [this](){ return myTargetUpdateFlag || myExitingFlag; }))
      {
         continue; // Heartbeat update interval timeout
      }
      if (myExitingFlag)
      {
         break;
      }

      if (!myTrajectory.empty())
      {
         auto& tp{myTrajectory.front()};
         auto currentTime{std::chrono::system_clock::now()};
         // If the current time is past the next trajectory point time, it is time to move the telescope
         if (currentTime >= tp.myTime)
         {
            myMotionController->moveHorizAngle(tp.myPosition.myAzimuth, tp.myVelocity.myVelAzimuth);
            myMotionController->moveVertAngle(tp.myPosition.myElevation, tp.myVelocity.myVelElevation);
            myCurrentAzimuth = tp.myPosition.myAzimuth;
            myCurrentElevation = tp.myPosition.myElevation;
            
            auto infoDisp = myInformationDisplay.lock();
            if (infoDisp != nullptr)
            {
               infoDisp->updateMotion(tp.myPosition, tp.myVelocity);
            }

            myTrajectory.pop();
         }
         else
         {
            // Set the time interval to the next trajectory point to wake up the thread
            myTrajectoryUpdateInterval = std::chrono::duration_cast<std::chrono::milliseconds>(tp.myTime - currentTime);
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

void PositionManager::calculateTrajectory(const std::vector<std::pair<Position, std::chrono::system_clock::time_point>>& positions)
{
   // Clear any existing trajectory from the system by swapping in an empty queue
   std::queue<TrajectoryPoint>().swap(myTrajectory);

   if (positions.size() > 1)
   {
      auto prevAzVel{0.0};
      auto prevElVel{0.0};
      double aAz{0.0}; double aEl{0.0};
      double bAz{0.0}; double bEl{0.0};
      double cAz{0.0}; double cEl{0.0};
      double dAz{0.0}; double dEl{0.0};
      double eAz{0.0}; double eEl{0.0};
      double fAz{0.0}; double fEl{0.0};

      auto prevIt{positions.begin()};
      auto currIt{positions.begin()+1};
      for (; currIt!=positions.end(); ++currIt)
      {
         const auto timeDiff{std::chrono::duration_cast<std::chrono::microseconds>(currIt->second - prevIt->second)};
         const auto diffInSec{static_cast<double>(timeDiff.count()) * MICROSECONDS_TO_SECONDS};
         auto avgVelAz{0.0};
         auto avgVelEl{0.0};

         // Only set the target velocity through the motion if not the end. 
         // We want the final velocity to be 0
         if (currIt + 1 != positions.end())
         {
            avgVelAz = (currIt->first.myAzimuth - prevIt->first.myAzimuth) / diffInSec;
            avgVelEl = (currIt->first.myElevation - prevIt->first.myElevation) / diffInSec;
         }
         calculatePolynomialCoef(prevIt->first.myAzimuth, currIt->first.myAzimuth, prevAzVel, avgVelAz, &aAz, &bAz, &cAz, &dAz, &eAz, &fAz);
         calculatePolynomialCoef(prevIt->first.myElevation, currIt->first.myElevation, prevElVel, avgVelEl, &aEl, &bEl, &cEl, &dEl, &eEl, &fEl);

         auto posAz{0.0};
         auto velAz{0.0};
         auto posEl{0.0};
         auto velEl{0.0};
         auto mainTimePoint{prevIt->second};
         auto t{0.0};
         // Calculate the position and velocity of telescope at sub intervals between the position and time pairs
         while (mainTimePoint < currIt->second)
         {
            calculatePositionAndVelocity(t/diffInSec, aAz, bAz, cAz, dAz, eAz, fAz, &posAz, &velAz);
            calculatePositionAndVelocity(t/diffInSec, aEl, bEl, cEl, dEl, eEl, fEl, &posEl, &velEl);
            TrajectoryPoint tp(posAz, posEl, velAz, velEl, mainTimePoint);
            myTrajectory.push(tp);
            mainTimePoint += myTrajectorySamplePeriod;
            t += static_cast<double>(myTrajectorySamplePeriod.count()) * MILLISECONDS_TO_SECONDS;
         }
         if (currIt + 1 == positions.end())
         {
            posAz = currIt->first.myAzimuth;
            posEl = currIt->first.myElevation;
            velAz = 0;
            velEl = 0;
         }
         else
         {
            mainTimePoint = currIt->second;
            calculatePositionAndVelocity(1, aAz, bAz, cAz, dAz, eAz, fAz, &posAz, &velAz);
            calculatePositionAndVelocity(1, aEl, bEl, cEl, dEl, eEl, fEl, &posEl, &velEl);
         }
         TrajectoryPoint tp(posAz, posEl, velAz, velEl, mainTimePoint);
         myTrajectory.push(tp);
         prevAzVel = velAz;
         prevElVel = velEl;
         prevIt = currIt;
      }
   }
   else
   {
      LOG_ERROR("Need to provide at least two positions in call to calculateTrajectory()");
      return;
   }
}

void PositionManager::calculatePolynomialCoef(const double& prevPos, const double& currPos, const double& prevVel, const double& currVel,  
                                                double* a, double* b, double* c, double* d, double* e, double* f)
{
   // Assuming accel is 0 at beginning and end points
   *a = (-6*prevPos + 6*currPos + -3*prevVel + -3*currVel);
   *b = (15*prevPos + -15*currPos + 8*prevVel + 7*currVel);
   *c = (-10*prevPos + 10*currPos + -6*prevVel + -4*currVel);
   *d = 0;
   *e = prevVel;
   *f = prevPos;
}

void PositionManager::calculatePositionAndVelocity(const double& t, const double& a, const double& b, const double& c, const double& d,
                                                      const double& e, const double& f, double* pos, double* vel)
{
   const auto t2{t  * t};
   const auto t3{t2 * t};
   const auto t4{t3 * t};
   const auto t5{t4 * t};
   *pos = a*t5 + b*t4 + c*t3 + d*t2 + e*t + f;
   *vel = 5*a*t4 + 4*b*t3 + 3*c*t2 + 2*d*t + e;
}