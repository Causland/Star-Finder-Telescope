#include "InformationDisplay.hpp"
#include "Logger.hpp"
#include "PositionManager.hpp"
#include "PropertyManager.hpp"
#include <algorithm>

const std::string PositionManager::NAME{"PositionManager"};

std::chrono::milliseconds PositionManager::MANUAL_MOVE_TIME_OFFSET{300};
std::chrono::milliseconds PositionManager::TRAJECTORY_SAMPLE_PERIOD_DURATION{10};
double PositionManager::TRAJECTORY_SAMPLE_PERIOD_IN_SEC{static_cast<double>(TRAJECTORY_SAMPLE_PERIOD_DURATION.count()) * MILLISECONDS_TO_SECONDS};

void PositionManager::start()
{
   // Get all properties from the properties manager
   int64_t timeOffset = 0;
   int64_t trajectorySamplePeriod = 0;
   if (PropertyManager::getProperty("manual_move_time_offset_ms", &timeOffset))
   {
      MANUAL_MOVE_TIME_OFFSET = std::chrono::milliseconds(timeOffset);
   }
   else
   {
      Logger::log(mySubsystemName, LogCodeEnum::ERROR, 
                     "Unable to set property: manual_move_time_offset_ms. Using default " + std::to_string(MANUAL_MOVE_TIME_OFFSET.count()));
   }
   if (PropertyManager::getProperty("trajectory_sample_period_ms", &trajectorySamplePeriod))
   {
      TRAJECTORY_SAMPLE_PERIOD_DURATION = std::chrono::milliseconds(trajectorySamplePeriod);
   }
   else
   {
      Logger::log(mySubsystemName, LogCodeEnum::ERROR, 
                     "Unable to set property: trajectory_sample_period_ms. Using default " + std::to_string(TRAJECTORY_SAMPLE_PERIOD_DURATION.count()));
   }

   // Set any calculated properties
   TRAJECTORY_SAMPLE_PERIOD_IN_SEC = static_cast<double>(TRAJECTORY_SAMPLE_PERIOD_DURATION.count()) * MILLISECONDS_TO_SECONDS;

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

void PositionManager::configureSubsystems(const std::vector<std::shared_ptr<Subsystem>>& subsystems)
{
   // Find each subsystem from the vector and store in the respective pointer
   // InformationDisplay
   auto it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == InformationDisplay::NAME; });
   if (it == subsystems.end())
   {
      Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Information Display pointer");
   }
   else
   {
      myInformationDisplay = std::dynamic_pointer_cast<InformationDisplay>(*it);
      if (myInformationDisplay.expired())
      {
         Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Information Display");
      }
   }
}

void PositionManager::updatePosition(const CmdUpdatePosition& cmd)
{
   std::scoped_lock<std::mutex> lk(myMutex);
   // Create a position series vector with the current position and time and the target position with a small time offset
   std::vector<std::pair<Position, std::chrono::system_clock::time_point>> positions;
   positions.emplace_back(Position(myCurrentAzimuth, myCurrentElevation), std::chrono::system_clock::now());
   positions.emplace_back(Position(cmd.myThetaInDeg, cmd.myPhiInDeg), std::chrono::system_clock::now() + MANUAL_MOVE_TIME_OFFSET);
   calculateTrajectory(positions);
   myTargetUpdateFlag = true;
   myCondVar.notify_one();
   Logger::log(mySubsystemName, LogCodeEnum::INFO, "Received move request: (az,el) | (" + std::to_string(myCurrentAzimuth) + "," + std::to_string(myCurrentElevation) + 
                                                         ")->(" + std::to_string(cmd.myThetaInDeg) + "," + std::to_string(cmd.myPhiInDeg) + ")");
}

void PositionManager::trackTarget(std::vector<std::pair<Position, std::chrono::system_clock::time_point>>& positions)
{
   std::scoped_lock<std::mutex> lk(myMutex);
   // Insert the current position and time at the front of the position series vector.
   const auto currentTime = std::chrono::system_clock::now();
   const auto currentPosition = std::pair<Position, std::chrono::system_clock::time_point>(Position(myCurrentAzimuth, myCurrentElevation), currentTime);
   positions.insert(positions.begin(), currentPosition);
   calculateTrajectory(positions);
   myTargetUpdateFlag = true;
   myCondVar.notify_one();
   Logger::log(mySubsystemName, LogCodeEnum::INFO, "Received target track request: points=" + std::to_string(positions.size()) + " start=(" + std::to_string(myCurrentAzimuth) +
                                                          "," + std::to_string(myCurrentElevation) + ") end=(" + std::to_string(positions.rbegin()->first.myAzimuth) + 
                                                          "," + std::to_string(positions.rbegin()->first.myElevation) + ")");
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
         auto& tp = myTrajectory.front();
         auto currentTime = std::chrono::system_clock::now();
         // If the current time is past the next trajectory point time, it is time to move the telescope
         if (currentTime >= tp.myTime)
         {
            auto motionController = myMotionController.lock();
            motionController->moveHorizAngle(tp.myPosition.myAzimuth, tp.myVelocity.myVelAzimuth);
            motionController->moveVertAngle(tp.myPosition.myElevation, tp.myVelocity.myVelElevation);
            myCurrentAzimuth = tp.myPosition.myAzimuth;
            myCurrentElevation = tp.myPosition.myElevation;
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
   // Clear any existing trajectory from the system
   while (!myTrajectory.empty())
   {
      myTrajectory.pop();
   }

   if (positions.size() > 1)
   {
      auto prevAzVel = 0.0;
      auto prevElVel = 0.0;
      double aAz = 0.0; double aEl = 0.0;
      double bAz = 0.0; double bEl = 0.0;
      double cAz = 0.0; double cEl = 0.0;
      double dAz = 0.0; double dEl = 0.0;
      double eAz = 0.0; double eEl = 0.0;
      double fAz = 0.0; double fEl = 0.0;

      auto prevIt = positions.begin();
      auto currIt = positions.begin()+1;
      for (; currIt!=positions.end(); ++currIt)
      {
         auto timeDiff = std::chrono::duration_cast<std::chrono::microseconds>(currIt->second - prevIt->second);
         auto diffInSec = static_cast<double>(timeDiff.count()) * MICROSECONDS_TO_SECONDS;
         auto avgVelAz = 0.0;
         auto avgVelEl = 0.0;
         // Set the final velocity target in the trajectory series to 0
         if (currIt + 1 == positions.end())
         {
            avgVelAz = 0;
            avgVelEl = 0;
         }
         else
         {
            avgVelAz = (currIt->first.myAzimuth - prevIt->first.myAzimuth) / diffInSec;
            avgVelEl = (currIt->first.myElevation - prevIt->first.myElevation) / diffInSec;
         }
         calculatePolynomialCoef(prevIt->first.myAzimuth, currIt->first.myAzimuth, prevAzVel, avgVelAz, &aAz, &bAz, &cAz, &dAz, &eAz, &fAz);
         calculatePolynomialCoef(prevIt->first.myElevation, currIt->first.myElevation, prevElVel, avgVelEl, &aEl, &bEl, &cEl, &dEl, &eEl, &fEl);

         double posAz = 0.0;
         double velAz = 0.0;
         double posEl = 0.0;
         double velEl = 0.0;
         auto mainTimePoint = prevIt->second;
         auto t = 0.0;
         // Calculate the position and velocity of telescope at sub intervals between the position and time pairs
         while (mainTimePoint < currIt->second)
         {
            calculatePositionAndVelocity(t/diffInSec, aAz, bAz, cAz, dAz, eAz, fAz, &posAz, &velAz);
            calculatePositionAndVelocity(t/diffInSec, aEl, bEl, cEl, dEl, eEl, fEl, &posEl, &velEl);
            TrajectoryPoint tp(posAz, posEl, velAz, velEl, mainTimePoint);
            myTrajectory.push(tp);
            mainTimePoint += TRAJECTORY_SAMPLE_PERIOD_DURATION;
            t += TRAJECTORY_SAMPLE_PERIOD_IN_SEC;
         }
         mainTimePoint = currIt->second;
         t = static_cast<double>(timeDiff.count()) * MICROSECONDS_TO_SECONDS;
         calculatePositionAndVelocity(t/diffInSec, aAz, bAz, cAz, dAz, eAz, fAz, &posAz, &velAz);
         calculatePositionAndVelocity(t/diffInSec, aEl, bEl, cEl, dEl, eEl, fEl, &posEl, &velEl);
         TrajectoryPoint tp(posAz, posEl, velAz, velEl, mainTimePoint);
         myTrajectory.push(tp);
         prevAzVel = velAz;
         prevElVel = velEl;
         prevIt = currIt;
      }
   }
   else
   {
      Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Need to provide at least two positions in call to calculateTrajectory()");
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
   const auto t2 = t  * t;
   const auto t3 = t2 * t;
   const auto t4 = t3 * t;
   const auto t5 = t4 * t;
   *pos = a*t5 + b*t4 + c*t3 + d*t2 + e*t + f;
   *vel = 5*a*t4 + 4*b*t3 + 3*c*t2 + 2*d*t + e;
}