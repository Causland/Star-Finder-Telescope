#ifndef SIM_STARDATABASE_HPP
#define SIM_STARDATABASE_HPP

#include "interfaces/StarDatabase/IStarDatabase.hpp"

class SimStarDatabase : public IStarDatabase
{
public:
   virtual ~SimStarDatabase() = default;
   QueryResult searchTargetsByName(const std::string& searchName) override;
   QueryResult searchTargetsByRange(const double& rangeInLightYears) override;
   QueryResult searchTargetsByLuminosity(const double& watts) override;

   // Query functions return the pointing information for a target from the database
   QueryResult queryTargetPointing(const std::string& targetName, const std::chrono::system_clock::time_point& time, 
                                             const double& gpsLong, const double& gpsLat, const double& gpsElev) override;
   QueryResult queryTargetPointingTrajectory(const std::string& targetName, const std::chrono::system_clock::time_point& startTime, 
                                                      const std::chrono::system_clock::time_point& endTime, const double& gpsLong,
                                                      const double& gpsLat, const double& gpsElev) override;
   std::string getDisplayInfo() override { return ""; };
private:
   QueryResult myResult{};
};

#endif
