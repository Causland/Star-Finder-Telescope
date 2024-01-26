#ifndef COMMON_HPP
#define COMMON_HPP

#include <chrono>

/*!
 * A Position object stores azimuth and elevation information.
 */ 
struct Position
{
   double myAzimuth{0.0};
   double myElevation{0.0};
};

/*!
 * A Velocity object stores change in azimuth and change in elevation information. 
 */ 
struct Velocity
{
   double myVelAzimuth{0.0};
   double myVelElevation{0.0};
};

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

#endif
