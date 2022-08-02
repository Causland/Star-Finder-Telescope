#ifndef POSITION_MANAGER_HPP
#define POSITION_MANAGER_HPP

#include "Subsystem.hpp"
#include "interfaces/IInformationDisplay.hpp"
#include "interfaces/IMotionController.hpp"
#include "interfaces/IPositionManager.hpp"
#include <chrono>
#include <memory>
#include <queue>
#include <vector>

constexpr double MICROSECONDS_TO_SECONDS = 0.000001;
constexpr double MILLISECONDS_TO_SECONDS = 0.001;

struct TrajectoryPoint
{
   TrajectoryPoint() = default;
   TrajectoryPoint(const double& az, const double& el, const double& vAz, const double& vEl, const std::chrono::system_clock::time_point& tp) :
                     myPosition(az, el), myVelocity(vAz, vEl), myTime(tp)
   {}
   TrajectoryPoint(const Position& pos, const Velocity& vel, const std::chrono::system_clock::time_point& tp) :
                     myPosition(pos), myVelocity(vel), myTime(tp)
   {}

   std::string toString()
   {
      auto tse = std::chrono::duration_cast<std::chrono::milliseconds>(myTime.time_since_epoch()).count();
      return std::to_string(tse) + ": (" + std::to_string(myPosition.myAzimuth) + "," + std::to_string(myPosition.myElevation) + ") - (" +
                                    std::to_string(myVelocity.myVelAzimuth) + "," + std::to_string(myVelocity.myVelElevation) + ")";
   }

   Position myPosition{};
   Velocity myVelocity{};
   std::chrono::system_clock::time_point myTime{};
};

class PositionManager : public IPositionManager, public Subsystem
{
public:
   PositionManager(std::string subsystemName,  std::shared_ptr<Logger> logger, std::shared_ptr<IMotionController> motionController) : 
                        Subsystem(subsystemName, logger), myMotionController(motionController) {}

   // Includes from ISubsystem
   void start() override;
   void stop() override;
   void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override;
   void threadLoop() override;

   // Includes from IPositionManager
   void updatePosition(const CmdUpdatePosition& cmd) override;
   void trackTarget(std::vector<std::pair<Position, std::chrono::system_clock::time_point>>& positions) override;
   void calibrate(const CmdCalibrate& cmd) override;

private:
   void calculateTrajectory(const std::vector<std::pair<Position, std::chrono::system_clock::time_point>>& positions);
   void calculatePolynomialCoef(const double& prevVal, const double& currVal, const double& prevVel, const double& currVel, 
                                 double* a, double* b, double* c, double* d, double* e, double* f);
   void calculatePositionAndVelocity(const double& t, const double& a, const double& b, const double& c, const double& d,
                                       const double& e, const double& f, double* pos, double* vel);

   static const std::chrono::milliseconds MANUAL_MOVE_OFFSET;
   static const std::chrono::milliseconds TRAJECTORY_SAMPLE_PERIOD_DURATION;
   static const double TRAJECTORY_SAMPLE_PERIOD_IN_SEC;

   double myCurrentAzimuth{0.0};
   double myCurrentElevation{0.0};
   bool myTargetUpdateFlag{false};
   std::chrono::milliseconds myTrajectoryUpdateInterval{HEARTBEAT_UPDATE_INTERVAL_MS};
   std::queue<TrajectoryPoint> myTrajectory{};
   std::weak_ptr<IMotionController> myMotionController;
   std::weak_ptr<IInformationDisplay> myInformationDisplay;
};

#endif