#ifndef I_STAR_DATABASE_HPP
#define I_STAR_DATABASE_HPP

#include "Common.hpp"
#include <chrono>
#include <string>
#include <utility>
#include <vector>

struct QueryResult
{
   enum Status{
      SUCCESS,
      NO_MATCH,
      INVALID_PARAM,
      FAILURE,
      UNINITIALIZED,
   };

   Status myStatusCode{UNINITIALIZED};
   std::string myLogStatement{"None"};
   std::vector<std::string> mySearchResults{};
   std::vector<std::pair<Position, std::chrono::system_clock::time_point>> myPositionResults{};
};

class IStarDatabase
{
public:
   // Search functions find and return target options from the database
   virtual QueryResult searchTargetsByName(const std::string& searchName) = 0;
   virtual QueryResult searchTargetsByRange(const double& rangeInLightYears) = 0;
   virtual QueryResult searchTargetsByLuminosity(const double& watts) = 0;

   // Query functions return the pointing information for a target from the database
   virtual QueryResult queryTargetPointing(const std::string& targetName, const std::chrono::system_clock::time_point& time, 
                                             const double& gpsLong, const double& gpsLat, const double& gpsElev) = 0;
   virtual QueryResult queryTargetPointingTrajectory(const std::string& targetName, const std::chrono::system_clock::time_point& startTime, 
                                                      const std::chrono::system_clock::time_point& endTime, const double& gpsLong,
                                                      const double& gpsLat, const double& gpsElev) = 0;
};

#endif