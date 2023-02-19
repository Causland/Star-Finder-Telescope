#include "interfaces/GpsModule/SimGpsModule.hpp"
#include <iomanip>
#include <sstream>

bool SimGpsModule::getGpsPosition(double* latitude, double* longitude, double* elevation)
{
   *latitude = myLatitude;
   *longitude = myLongitude;
   *elevation = myElevation;
   return true;
}

std::string SimGpsModule::getDisplayInfo()
{
   std::stringstream ss;
   ss << std::fixed << std::internal
      << "GPS: " << std::setprecision(4) << myLatitude
      << "(deg), " << std::setprecision(4) << myLongitude
      << "(deg), " << std::setprecision(2) << myElevation
      << "(m)";
   return ss.str();
}