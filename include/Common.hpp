#ifndef COMMON_HPP
#define COMMON_HPP

#include <chrono>
#include <memory>

struct Position
{
   Position() = default;
   Position(const double& az, const double& el) :
      myAzimuth(az), myElevation(el)
   {}

   double myAzimuth{0.0};
   double myElevation{0.0};
};

struct Velocity
{
   Velocity() = default;
   Velocity(const double& vAz, const double& vEl) :
      myVelAzimuth(vAz), myVelElevation(vEl)
   {}

   double myVelAzimuth{0.0};
   double myVelElevation{0.0};
};

#endif