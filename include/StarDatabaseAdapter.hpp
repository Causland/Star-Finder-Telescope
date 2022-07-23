#ifndef STAR_DATABASE_HPP
#define STAR_DATABASE_HPP

#include "DatabaseSchema.hpp"
#include "StarApi.hpp"
#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlpp11.h"
#include <chrono>
#include <string>

class StarDatabaseAdapter
{
public:
    StarDatabaseAdapter();
    ~StarDatabaseAdapter();
    bool queryTargetPointing(const std::string& targetName, const std::chrono::time_point<std::chrono::system_clock>& time, 
                                const double& gpsLong, const double& gpsLat, const double& gpsElev, double* azimuth, double* elevation);
    void queryTargetsWithinRange(double rangeInLightMinutes);
    
private:
    std::string myDatabasePath;
    sqlpp::sqlite3::connection myDatabaseConnection;
    StarApi myStarApi;
};

#endif