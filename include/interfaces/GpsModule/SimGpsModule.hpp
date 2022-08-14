#ifndef SIM_GPSMODULE_HPP
#define SIM_GPSMODULE_HPP

#include "interfaces/GpsModule/IGpsModule.hpp"

class SimGpsModule : public IGpsModule
{
public:
   bool getGpsPosition(double* latitude, double* longitude, double* elevation) override;
   bool isGpsLock() override;
   uint8_t getNumAcquiredSats() override;
   bool acquireGpsLock() override;

private:
   double myLatitude{0.0};
   double myLongitude{0.0};
   double myElevation{0.0};
   bool myGpsLockedFlag{false};
   uint8_t myNumAcquiredSats{0};
};

#endif