#include "interfaces/GpsModule/SimGpsModule.hpp"

bool SimGpsModule::getGpsPosition(double* latitude, double* longitude, double* elevation)
{
   *latitude = myLatitude;
   *longitude = myLongitude;
   *elevation = myElevation;
   return true;
}