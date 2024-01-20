#ifndef POSITION_MANAGER_HPP
#define POSITION_MANAGER_HPP

#include "CommandTypes.hpp"
#include "Common.hpp"
#include "InformationDisplay.hpp"
#include "Subsystem.hpp"
#include "interfaces/MotionController/IMotionController.hpp"

#include <chrono>
#include <memory>
#include <queue>
#include <vector>

static constexpr double MILLISECONDS_TO_SECONDS{0.001}; //!< Conversion factor from ms to s
static constexpr uint32_t DEFAULT_MANUAL_MOVE_TIME_OFFSET{300};
static constexpr uint32_t DEFAULT_TRAJECTORY_SAMPLE_PERIOD{10};

/*!
 * Structure to hold information about a specific point along a trajectory path. Each point is has a
 * time at which the telescope should be pointing at the position or moving at the velocity.
 */
struct TrajectoryPoint
{
   /*!
    * Creates a default TrajectoryPoint positioned at the origin, no velocity, and at the epoch.
    */
   TrajectoryPoint() = default;

   /*!
    * Creates a TrajectoryPoint at a position and time.
    * \param[in] pos a position of the telescope.
    * \param[in] tp a specific point in time.
    */
   TrajectoryPoint(const Position& pos, const std::chrono::system_clock::time_point& time) :
                     myPosition{pos}, myTime{time} {}

   /*!
    * Creates a TrajectoryPoint at a position, velocity, and time.
    * \param[in] pos a position of the telescope.
    * \param[in] vel an instantaneous velocity of the telescope.
    * \param[in] tp a specific point in time.
    */
   TrajectoryPoint(const Position& pos, const Velocity& vel, const std::chrono::system_clock::time_point& time) :
                     myPosition{pos}, myVelocity{vel}, myTime{time} {}

   Position myPosition; //!< The pointing position of the telescope.
   Velocity myVelocity; //!< The instantaneous velocity of the telescope.
   std::chrono::system_clock::time_point myTime; //!< The point in time.
};

/*!
 * The PositionManager subsystem is responsible for changing the pointing position of the
 * telescope over a target trajectory. The PositionManager thread waits for a trajectory
 * to be created via interface calls from either the StarTracker subsystem or the 
 * CommandTerminal subsystem. The PositionManager waits for the specific time point listed
 * in the trajectory and uses the MotionController interface to move to a position or
 * change the velocity.
 */
class PositionManager : public Subsystem
{
public:
   /*!
    * Creates a PositionManager subsystem object with a pointer to the MotionController use to
    * move the telescope.
    * \param[in] motionController a shared pointer to a MotionController interface.
    */
   explicit PositionManager(std::shared_ptr<IMotionController> motionController) : 
                              Subsystem{"PositionManager"}, myMotionController{std::move(motionController)} {}

   /*!
    * Destroys a PositionManager.
    */
   ~PositionManager() override = default;

   PositionManager(const PositionManager&) = delete;
   PositionManager& operator=(const PositionManager&) = delete;
   PositionManager(PositionManager&&) = delete;
   PositionManager& operator=(PositionManager&&) = delete;

   /*!
    * Initialize the subsystem and start the thread.
    */
   void start() override;

   /*!
    * Cleanup the subsystem, signal to end the forever loop, and wait for the thread to join.
    */
   void stop() override;

   /*!
    * Set interface pointers for use throughout the subsystem.
    * \param[in] subsystems a list of subsystem interface pointers.
    */
   void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                             static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) override;

   /*!
    * Move the telescope to a position over a fixed amount of time defined with the constant
    * MANUAL_MOVE_OFFSET. Creates a trajectory between the current position and current time and
    * the target position and offset time.
    * \param[in] cmd an update position command
    * \sa calculateTrajectory()
    */
   void updatePosition(const CmdUpdatePosition& cmd);

   /*!
    * Move the telescope to follow a set path over time. Interpolate smooth trajectory from the provided points.
    * \param[in] path a vector of TrajectoryPoints containing position and time data.
    * \sa calculateTrajectory()
    */
   void trackTarget(std::vector<TrajectoryPoint>& path);

   /*!
    * Inform the MotionController to calibrate and update its stopping limits
    * \param[in] cmd a calibration command.
    */
   void calibrate(const CmdCalibrate& cmd);

private:
   /*!
    * The PositionManager threadloop handles changing the position of the telescope to follow
    * a specific trajectory updating a MotionController via interface calls to control position
    * and velocity. Once a trajectory is created, the threadloop waits for the time point at the
    * top of the trajectory queue. It then pops the TrajectoryPoint off the queue
    * and informs the MotionController of the new position and velocity.
    */
   void threadLoop() override;

   /*!
    * Calculate a trajectory to follow a set path over time. The trajectory is created
    * by interpolating points along the path using a 5th order polynomial function. The number of points in the 
    * trajectory are determined using TRAJECTORY_SAMPLE_PERIOD_DURATION. The acceleration through each point is 
    * calculated to be 0. The velocity through each point is the average velocity from the previous point to the next.
    * \param[in] path a vector of TrajectoryPoints containing position and time data.
    * \sa calculatePolynomialCoef(), calculatePositionAndVelocity()   
    */
   void calculateTrajectory(std::vector<TrajectoryPoint>& path);

   /*!
    * Stores the coefficients for a 5th order polynomial.
    */
   struct PolyCoefs
   {
      double a{0.0};
      double b{0.0};
      double c{0.0};
      double d{0.0};
      double e{0.0};
      double f{0.0};
   };

   /*!
    * Calculate the 5th order polynomial coefficients for a trajectory which tracks the
    * position and velocity of the azimuth and elevation.
    * It is assumed that the acceleration at the start point and end point is 0. 
    * A 5th order polynomial is defined as s(t) = at^5 + bt^4 + ct^3 + dt^2 + et + f.
    * \param[in] prevPos the latest position.
    * \param[in] currPos the target position.
    * \param[in] prevVel the latest velocity.
    * \param[in] currVel the target velocity.
    * \param[out] azCoefs the coefficients for the azimuth polynomial.
    * \param[out] elCoefs the coefficients for the elevation polynomial.
    */
   static constexpr void calculatePolynomialCoef(const Position& prevPos, const Position& currPos, 
                                                 const Velocity& prevVel, const Velocity& currVel, 
                                                 PolyCoefs* azCoefs, PolyCoefs* elCoefs)
   {
      // Assuming accel is 0 at beginning and end points. Equations for coefficients derived from polynomial 
      // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
      azCoefs->a = -6*prevPos.myAzimuth + 6*currPos.myAzimuth + -3*prevVel.myVelAzimuth + -3*currVel.myVelAzimuth;
      azCoefs->b = 15*prevPos.myAzimuth + -15*currPos.myAzimuth + 8*prevVel.myVelAzimuth + 7*currVel.myVelAzimuth;
      azCoefs->c = -10*prevPos.myAzimuth + 10*currPos.myAzimuth + -6*prevVel.myVelAzimuth + -4*currVel.myVelAzimuth;
      azCoefs->d = 0;
      azCoefs->e = prevVel.myVelAzimuth;
      azCoefs->f = prevPos.myAzimuth;

      elCoefs->a = -6*prevPos.myElevation + 6*currPos.myElevation + -3*prevVel.myVelElevation + -3*currVel.myVelElevation;
      elCoefs->b = 15*prevPos.myElevation + -15*currPos.myElevation + 8*prevVel.myVelElevation + 7*currVel.myVelElevation;
      elCoefs->c = -10*prevPos.myElevation + 10*currPos.myElevation + -6*prevVel.myVelElevation + -4*currVel.myVelElevation;
      elCoefs->d = 0;
      elCoefs->e = prevVel.myVelElevation;
      elCoefs->f = prevPos.myElevation;
      // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
   }

   /*!
    * Calculate the position and velocity of azimuth and elevation at a point of time 
    * between 0 and 1 using the provided polynomial coefficients.
    * A 5th order polynomial for position is defined as
    * s(t) = at^5 + bt^4 + ct^3 + dt^2 + et + f
    * which velocity can be derived from as
    * v(t) = 5at^4 + 4bt^3 + 3ct^2 + 2dt + e
    * \param[in] t a time between 0 and 1.
    * \param[in] azCoefs the coefficients for the azimuth polynomial.
    * \param[in] elCoefs the coefficients for the elevation polynomial.
    * \param[out] pos the calculated position.
    * \param[out] vel the calculated instantaneous velocity.
    */
   // NOLINTBEGIN(bugprone-easily-swappable-parameters, readability-identifier-length, cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
   static constexpr void calculatePositionAndVelocity(const double& t, const PolyCoefs& azCoefs, const PolyCoefs& elCoefs,
                                                      Position* pos, Velocity* vel)
   {
      const auto t2{t * t};
      const auto t3{t2 * t};
      const auto t4{t3 * t};
      const auto t5{t4 * t};

      pos->myAzimuth = azCoefs.a*t5 + azCoefs.b*t4 + azCoefs.c*t3 + azCoefs.d*t2 + azCoefs.e*t + azCoefs.f;
      vel->myVelAzimuth = 5*azCoefs.a*t4 + 4*azCoefs.b*t3 + 3*azCoefs.c*t2 + 2*azCoefs.d*t + azCoefs.e;

      pos->myElevation = elCoefs.a*t5 + elCoefs.b*t4 + elCoefs.c*t3 + elCoefs.d*t2 + elCoefs.e*t + elCoefs.f;
      vel->myVelElevation = 5*elCoefs.a*t4 + 4*elCoefs.b*t3 + 3*elCoefs.c*t2 + 2*elCoefs.d*t + elCoefs.e;
   }
   // NOLINTEND(bugprone-easily-swappable-parameters, readability-identifier-length, cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

   Position myCurrentPosition; //!< The current pointing position of the telescope.
   bool myTargetUpdateFlag{false}; //!< The flag used to determine when the telescope needs to be moved.
   std::chrono::milliseconds myManualMoveTimeOffset{DEFAULT_MANUAL_MOVE_TIME_OFFSET}; //!< The time used to offset the time for manually moving the telescope to a specific position.
   std::chrono::milliseconds myTrajectorySamplePeriod{DEFAULT_TRAJECTORY_SAMPLE_PERIOD}; //!< The time used to generate sub-points within a trajectory in milliseconds.
   std::chrono::milliseconds myTrajectoryUpdateInterval{HEARTBEAT_UPDATE_INTERVAL_MS}; //!< The time interval to the next trajectory point.
   std::queue<TrajectoryPoint> myTrajectory; //!< The queue which holds the points along a trajectory.
   std::shared_ptr<IMotionController> myMotionController; //!< Shared pointer to the Motion Controller.
   std::weak_ptr<InformationDisplay> myInformationDisplay; //!< Weak pointer to the Information Display.
};

#endif
