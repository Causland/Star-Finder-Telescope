#ifndef COMMON_HPP
#define COMMON_HPP

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

#endif
