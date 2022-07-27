#ifndef I_STAR_DATABASE_HPP
#define I_STAR_DATABASE_HPP

#include <chrono>
#include <string>

class IStarDatabase
{
public:
   // Search functions find and return target options from the database
   virtual void searchTargetsByName(const std::string& searchName) = 0;
   virtual void searchTargetsByRange(double rangeInLightYears) = 0;
   virtual void searchTargetsByLuminosity(double watts) = 0;

   // Query functions return the pointing information for a target from the database
   virtual bool queryTargetPointing(const std::string& targetName, const std::chrono::system_clock::time_point& time, 
                                       const double& gpsLong, const double& gpsLat, const double& gpsElev, double* azimuth, double* elevation) = 0;
   virtual void queryTargetPointingTrajectory(const std::string& targetName, const std::chrono::system_clock::time_point& startTime, 
                                                const std::chrono::system_clock::time_point& endTime, const double& gpsLong, const double& gpsLat, 
                                                const double& gpsElev, double* azimuth, double* elevation) = 0;

};

#endif