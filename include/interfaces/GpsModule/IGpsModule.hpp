#ifndef I_GPS_MODULE_HPP
#define I_GPS_MODULE_HPP

#include <string>

class IGpsModule
{
public:
   virtual bool getGpsPosition(double* latitude, double* longitude, double* elevation) = 0;
   virtual std::string getDisplayInfo() = 0;
};

#endif