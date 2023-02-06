#ifndef SIM_GPSMODULE_HPP
#define SIM_GPSMODULE_HPP

#include "interfaces/GpsModule/IGpsModule.hpp"

class SimGpsModule : public IGpsModule
{
public:
   bool getGpsPosition(double* latitude, double* longitude, double* elevation) override;

private:
   double myLatitude{28.4186};
   double myLongitude{81.5812};
   double myElevation{15};
};

#endif