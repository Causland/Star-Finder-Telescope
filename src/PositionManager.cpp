#include "PositionManager.hpp"
#include <algorithm>

const std::string IPositionManager::NAME{"PositionManager"};

const std::chrono::milliseconds PositionManager::MANUAL_MOVE_OFFSET{300};
const std::chrono::milliseconds PositionManager::TRAJECTORY_SAMPLE_PERIOD_DURATION{10};
const double PositionManager::TRAJECTORY_SAMPLE_PERIOD_IN_SEC{TRAJECTORY_SAMPLE_PERIOD_DURATION.count() * MILLISECONDS_TO_SECONDS};

void PositionManager::start()
{
   myThread = std::thread(&PositionManager::threadLoop, this);
}

void PositionManager::stop()
{
   myExitingFlag = true;
   myCondVar.notify_one();
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
         if (currentTime >= tp.myTime)
         {
            myLogger->log(mySubsystemName, LogCodeEnum::INFO, "Changing position to (" + std::to_string(tp.myPosition.myAzimuth) + "," + std::to_string(tp.myPosition.myElevation) + ")");
            auto motionController = myMotionController.lock();
            motionController->moveHorizAngle(tp.myPosition.myAzimuth, tp.myVelocity.myVelAzimuth);
            motionController->moveVertAngle(tp.myPosition.myElevation, tp.myVelocity.myVelElevation);
            myCurrentAzimuth = tp.myPosition.myAzimuth;
            myCurrentElevation = tp.myPosition.myElevation;
            myTrajectory.pop();
         }
         else
         {
            myTrajectoryUpdateInterval = std::chrono::duration_cast<std::chrono::milliseconds>(tp.myTime - currentTime);
         }
      }
      else
      {
         myTargetUpdateFlag = false;
         myTrajectoryUpdateInterval = HEARTBEAT_UPDATE_INTERVAL_MS;
      }
   }
}

void PositionManager::updatePosition(const CmdUpdatePosition& cmd)
{
   std::scoped_lock<std::mutex> lk(myMutex);
   std::vector<std::pair<Position, std::chrono::system_clock::time_point>> positions;
   positions.emplace_back(Position(myCurrentAzimuth, myCurrentElevation), std::chrono::system_clock::now());
   positions.emplace_back(Position(cmd.myThetaInDeg, cmd.myPhiInDeg), std::chrono::system_clock::now() + MANUAL_MOVE_OFFSET);
   calculateTrajectory(positions);
   myTargetUpdateFlag = true;
   myCondVar.notify_one();
   myLogger->log(mySubsystemName, LogCodeEnum::INFO, "Received move request: (az,el) | (" + std::to_string(myCurrentAzimuth) + "," + std::to_string(myCurrentElevation) + 
                                                         ")->(" + std::to_string(cmd.myThetaInDeg) + "," + std::to_string(cmd.myPhiInDeg) + ")");
}

void PositionManager::trackTarget(std::vector<std::pair<Position, std::chrono::system_clock::time_point>>& positions)
{
   std::scoped_lock<std::mutex> lk(myMutex);
   const auto currentTime = std::chrono::system_clock::now();
   const auto currentPosition = std::pair<Position, std::chrono::system_clock::time_point>(Position(myCurrentAzimuth, myCurrentElevation), currentTime);
   positions.insert(positions.begin(), currentPosition);
   calculateTrajectory(positions);
   myTargetUpdateFlag = true;
   myCondVar.notify_one();
}

void PositionManager::calibrate(const CmdCalibrate& cmd)
{
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
         auto diffInSec = timeDiff.count() * MICROSECONDS_TO_SECONDS;
         auto avgVelAz = 0.0;
         auto avgVelEl = 0.0;
         if (currIt + 1 == positions.end())
         {
            // Set the final velocity target in the trajectory series to 0
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
         t = timeDiff.count() * MICROSECONDS_TO_SECONDS;
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
      myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Need to provide at least two positions in call to calculateTrajectory()");
      return;
   }
}

void PositionManager::calculatePolynomialCoef(const double& prevPos, const double& currPos, const double& prevVel, const double& currVel,  
                                                double* a, double* b, double* c, double* d, double* e, double* f)
{
   // Assuming accel is 0 at beginning and end of points
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