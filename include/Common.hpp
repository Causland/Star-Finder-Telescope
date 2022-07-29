#ifndef COMMON_HPP
#define COMMON_HPP

#include <chrono>
#include <memory>

struct Position
{
   Position() = delete;
   Position(const double& az, const double& el, const std::chrono::system_clock::time_point& tp) :
      azimuth(az), elevation(el), time(tp)
   {}

   double azimuth{0.0};
   double elevation{0.0};
   std::chrono::system_clock::time_point time{};
};

#endif