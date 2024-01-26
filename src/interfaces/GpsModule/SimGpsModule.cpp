#include "interfaces/GpsModule/SimGpsModule.hpp"
#include <iomanip>
#include <sstream>

bool SimGpsModule::getGpsPosition(GpsPosition* position)
{
   *position = myGpsPosition;
   return true;
}

std::string SimGpsModule::getDisplayInfo()
{
   std::ostringstream oss;
   oss << std::fixed << std::internal
      << "GPS: " << std::setprecision(4) << myGpsPosition.myLatitude
      << "(deg), " << std::setprecision(4) << myGpsPosition.myLongitude
      << "(deg), " << std::setprecision(2) << myGpsPosition.myElevation
      << "(m)";
   return oss.str();
}
