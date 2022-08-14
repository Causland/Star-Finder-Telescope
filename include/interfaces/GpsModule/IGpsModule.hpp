#ifndef I_GPS_MODULE_HPP
#define I_GPS_MODULE_HPP

#include <cstdint>

class IGpsModule
{
public:
   virtual bool getGpsPosition(double* latitude, double* longitude, double* elevation) = 0;
   virtual bool isGpsLock() = 0;
   virtual uint8_t getNumAcquiredSats() = 0;
   virtual bool acquireGpsLock() = 0;
};

#endif