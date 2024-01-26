#include "interfaces/StarDatabase/SimStarDatabase.hpp"

QueryResult SimStarDatabase::searchTargetsByName(std::string_view searchName)
{
   return myResult;
}

QueryResult SimStarDatabase::searchTargetsByRange(const double& rangeInLightYears)
{
   return myResult;
}

QueryResult SimStarDatabase::searchTargetsByLuminosity(const double& watts)
{
   return myResult;
}

QueryResult SimStarDatabase::queryTargetPointing(const std::string& targetName, const std::chrono::system_clock::time_point& time, 
                                                 const GpsPosition& gpsPosition)
{
   return myResult;
}

QueryResult SimStarDatabase::queryTargetPointingTrajectory(const std::string& targetName, const std::chrono::system_clock::time_point& startTime, 
                                                   const std::chrono::seconds& duration, const GpsPosition& gpsPosition)
{
   return myResult;
}
