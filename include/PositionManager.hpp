#ifndef POSITION_MANAGER_HPP
#define POSITION_MANAGER_HPP

#include "CommandTypes.hpp"
#include "Common.hpp"
#include "Subsystem.hpp"
#include "interfaces/MotionController/IMotionController.hpp"
#include <chrono>
#include <memory>
#include <queue>
#include <vector>

class InformationDisplay;

constexpr double MICROSECONDS_TO_SECONDS = 0.000001; //!< Conversion factor from us to s
constexpr double MILLISECONDS_TO_SECONDS = 0.001; //!< Conversion factor from ms to s

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
    * Creates a TrajectoryPoint at a specific azimuth, elevation, velocity, and time. Constructs a Position and Velocity
    * object using the provided information.
    * \param[in] az an azimuth of the telescope in degrees.
    * \param[in] el an elevation of the telescope in degrees.
    * \param[in] vAz an instantaneous change in azimuth over time of the telescope in RPM.
    * \param[in] vEl an instantaneous change in elevation over time of the telescope in RPM.
    * \param[in] tp a specific point in time.  
    */
   TrajectoryPoint(const double& az, const double& el, const double& vAz, const double& vEl, const std::chrono::system_clock::time_point& tp) :
                     myPosition(az, el), myVelocity(vAz, vEl), myTime(tp)
   {}

   /*!
    * Creates a TrajectoryPoint at a position, velocity, and time.
    * \param[in] pos a position of the telescope.
    * \param[in] vel an instantaneous velocity of the telescope.
    * \param[in] tp a specific point in time.
    */
   TrajectoryPoint(const Position& pos, const Velocity& vel, const std::chrono::system_clock::time_point& tp) :
                     myPosition(pos), myVelocity(vel), myTime(tp)
   {}

   Position myPosition{}; //!< The pointing position of the telescope.
   Velocity myVelocity{}; //!< The instantaneous velocity of the telescope.
   std::chrono::system_clock::time_point myTime{}; //!< The point in time.
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
    * Creates a PositionManager subsystem object with a logger and a pointer to the MotionController use to
    * move the telescope.
    * \param[in] subsystemName a string of the subsystem name moved into the class.
    * \param[in] motionController a shared pointer to a MotionController interface.
    */
   PositionManager(std::string subsystemName, std::shared_ptr<IMotionController> motionController) : 
                        Subsystem(subsystemName), myMotionController(motionController) {}

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
   void configureSubsystems(const std::vector<std::shared_ptr<Subsystem>>& subsystems) override;

   /*!
    * Move the telescope to a position over a fixed amount of time defined with the constant
    * MANUAL_MOVE_OFFSET. Creates a trajectory between the current position and current time and
    * the target position and offset time.
    * \param[in] cmd an update position command
    * \sa calculateTrajectory()
    */
   virtual void updatePosition(const CmdUpdatePosition& cmd);

   /*!
    * Move the telescope to follow a series of positions over time. Creates a trajectory between each
    * position in the series to smoothly follow the path.
    * \param[in] positions a vector of position and time pairs to create a trajectory from.
    * \sa calculateTrajectory()
    */
   virtual void trackTarget(std::vector<std::pair<Position, std::chrono::system_clock::time_point>>& positions);

   /*!
    * Inform the MotionController to calibrate and update its stopping limits
    * \param[in] cmd a calibration command.
    */
   virtual void calibrate(const CmdCalibrate& cmd);

   static const std::string NAME; //!< Name of the subsystem.
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
    * Calculate a trajectory over a series of position and time pairs. The trajectory is created
    * by interpolating a path using a 5th order polynomial function. The amount of points along
    * the trajectory are determines using the TRAJECTORY_SAMPLE_PERIOD_DURATION constant. The
    * acceleration through each position and time pair is calculated to be 0. The velocity through
    * each of these points is the average velocity from the previous point to the next.
    * \param[in] positions a vector of position and time pairs to create a trajectory from.
    * \sa calculatePolynomialCoef(), calculatePositionAndVelocity()   
    */
   void calculateTrajectory(const std::vector<std::pair<Position, std::chrono::system_clock::time_point>>& positions);

   /*!
    * Calculate the polynomial coefficients for a 5th order polynomial given the previous 
    * position and velocity and the target position and velocity. It is assumed that the
    * acceleration at the start point and end point is 0. A 5th order polynomial is defined
    * as s(t) = at^5 + bt^4 + ct^3 + dt^2 + et + f.
    * \param[in] prevVal the latest position
    * \param[in] currVal the target position
    * \param[in] prevVel the latest velocity
    * \param[in] currVel the target velocity
    * \param[out] a coefficient a
    * \param[out] b coefficient b
    * \param[out] c coefficient c
    * \param[out] d coefficient d
    * \param[out] e coefficient e
    * \param[out] f coefficient f  
    */
   void calculatePolynomialCoef(const double& prevVal, const double& currVal, const double& prevVel, const double& currVel, 
                                 double* a, double* b, double* c, double* d, double* e, double* f);

   /*!
    * Calculate the position and velocity for a 5th order polynomial given coefficients
    * and the time between 0 and 1. A 5th order polynomial for position is defined as
    * s(t) = at^5 + bt^4 + ct^3 + dt^2 + et + f
    * which velocity can be derived from as
    * v(t) = 5at^4 + 4bt^3 + 3ct^2 + 2dt + e
    * \param[in] t a time between 0 and 1
    * \param[in] a polynomial coefficient a
    * \param[in] b polynomial coefficient b
    * \param[in] c polynomial coefficient c
    * \param[in] d polynomial coefficient d
    * \param[in] e polynomial coefficient e
    * \param[in] f polynomial coefficient f  
    * \param[out] pos the calculated position
    * \param[out] vel the calculated instantaneous velocity
    */
   void calculatePositionAndVelocity(const double& t, const double& a, const double& b, const double& c, const double& d,
                                       const double& e, const double& f, double* pos, double* vel);

   static const std::chrono::milliseconds MANUAL_MOVE_OFFSET; //!< The constant used to offset the time for manually moving the telescope to a specific position.
   static const std::chrono::milliseconds TRAJECTORY_SAMPLE_PERIOD_DURATION; //!< The constant used to generate sub-points within a trajectory in milliseconds.
   static const double TRAJECTORY_SAMPLE_PERIOD_IN_SEC; //!< The constant used to generate sub-points within a trajectory in seconds.

   double myCurrentAzimuth{0.0}; //!< The current pointing azimuth of the telescope.
   double myCurrentElevation{0.0}; //!< The current pointing elevation of the telescope.
   bool myTargetUpdateFlag{false}; //!< The flag used to determine when the telescope needs to be moved.
   std::chrono::milliseconds myTrajectoryUpdateInterval{HEARTBEAT_UPDATE_INTERVAL_MS}; //!< The time interval to the next trajectory point.
   std::queue<TrajectoryPoint> myTrajectory{}; //!< The queue which holds the points along a trajectory.
   std::weak_ptr<IMotionController> myMotionController; //!< Weak pointer to the Motion Controller.
   std::weak_ptr<InformationDisplay> myInformationDisplay; //!< Weak pointer to the Information Display.
};

#endif