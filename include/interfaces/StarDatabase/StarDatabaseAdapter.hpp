#ifndef STAR_DATABASE_HPP
#define STAR_DATABASE_HPP

#include "interfaces/StarDatabase/DatabaseSchema.hpp"
#include "StarApi.hpp"
#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlpp11.h"

#include <chrono>
#include <string>

/*!
 * StarDatabaseAdapter interacts with the underlying database to make queries, ask for data from the StarApi,
 * and store data for later use.
 */ 
class StarDatabaseAdapter
{
public:
   /*!
    * Creates a StarDatabaseAdapter using the provided path to a database file.
    * \param[in] databasePath the path to a database file.
    */ 
   explicit StarDatabaseAdapter(const std::string& databasePath) : myDatabaseConnection{sqlpp::sqlite3::connection_config(databasePath)} {}


   /*!
    * Query a target's position at a given time and observer position.
    * \param[in] targetName the name of the target.
    * \param[in] time the time to query for.
    * \param[in] gpsPosition the position of the observer.
    * \param[out] position the azimuth and elevation data to point to.
    * \return true if the query was successful and data in position is valid.
    */ 
   bool queryTargetPointing(const std::string& targetName, const std::chrono::time_point<std::chrono::system_clock>& time, 
                            const GpsPosition& gpsPosition, Position* position);


   /*!
    * Query a target's position within a specific distance.
    * \param[in] rangeInLightMinutes the maximum search distance.
    */ 
   void queryTargetsWithinRange(double rangeInLightMinutes);
    
private:
   sqlpp::sqlite3::connection myDatabaseConnection; //!< A cursor to the active database.
   StarApi myStarApi; //!< The API to use for collecting missing data.
};

#endif
