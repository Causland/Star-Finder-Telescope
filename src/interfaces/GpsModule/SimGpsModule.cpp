#include "interfaces/GpsModule/SimGpsModule.hpp"

bool SimGpsModule::getGpsPosition(double* latitude, double* longitude, double* elevation)
{
   *latitude = myLatitude;
   *longitude = myLongitude;
   *elevation = myElevation;
   return isGpsLock();
}

bool SimGpsModule::isGpsLock()
{
   return myGpsLockedFlag;
}

uint8_t SimGpsModule::getNumAcquiredSats()
{
   return myNumAcquiredSats;
}

bool SimGpsModule::acquireGpsLock()
{
   myGpsLockedFlag = true;
   myNumAcquiredSats = 3;
   myLatitude = 28.4186;
   myLongitude = 81.5812;
   myElevation = 15;
   return true;
}