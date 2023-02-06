#ifndef BN_180_GPS_MODULE_HPP
#define BN_180_GPS_MODULE_HPP

#include "interfaces/GpsModule/IGpsModule.hpp"
#include <atomic>
#include <mutex>
#include <sstream>
#include <thread>


class Bn180GpsModule : public IGpsModule
{
public:
   explicit Bn180GpsModule(const std::string& serialDevice);
   ~Bn180GpsModule();
   Bn180GpsModule(const Bn180GpsModule&) = delete;
   Bn180GpsModule(Bn180GpsModule&&) = delete;
   void operator=(const Bn180GpsModule&) = delete;
   void operator=(Bn180GpsModule&&) = delete;

   bool getGpsPosition(double* latitude, double* longitude, double* elevation) override;

private:
   void threadLoop();

   bool parseNmea(std::istringstream& messages);
   bool parseNmeaGGA(std::istringstream& message);
   bool parseNmeaRMC(std::istringstream& message);
   bool parseNmeaGLL(std::istringstream& message);
   
   double myLatitude{0.0};
   double myLongitude{0.0};
   double myElevation{0.0};
   bool myGpsLockFlag{false};
   std::atomic<bool> myExitFlag{false};
   std::thread myThread;
   std::mutex myMutex;
   std::string serialChars;
   int mySerialFd{-1};
};

#endif