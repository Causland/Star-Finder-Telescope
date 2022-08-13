#ifndef I_GPS_MODULE_HPP
#define I_GPS_MODULE_HPP

#include <cstdint>

class IGpsModule
{
public:
   virtual bool getGpsPosition(double* latitude, double* longitude, double* elevation);
   virtual bool isGpsLock();
   virtual uint8_t getNumAcquiredSats();
   virtual bool acquireGpsLock();
};

#endif