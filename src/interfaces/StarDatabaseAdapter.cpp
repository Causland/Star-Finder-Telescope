#include "StarDatabaseAdapter.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>

StarDatabaseAdapter::StarDatabaseAdapter() : myDatabaseConnection(sqlpp::sqlite3::connection_config(myDatabasePath))
{
}

StarDatabaseAdapter::~StarDatabaseAdapter()
{
}

bool StarDatabaseAdapter::queryTargetPointing(const std::string& targetName, const std::chrono::time_point<std::chrono::system_clock>& time,
                                                const double& gpsLong, const double& gpsLat, const double& gpsElev, double* azimuth, double* elevation)
{
   // Preprocess query information
   // Target Name
   std::string name = "None";
   auto id = UINT32_MAX; // No results from API should have this id
   if (std::find_if(targetName.begin(), targetName.end(), [](auto& c){ return !std::isdigit(c); }) == targetName.end())
   {
      id = std::stoi(targetName);
   }
   else
   {
      name = targetName;
   }

   // Time
   std::ostringstream timeStream;
   std::time_t t = std::chrono::system_clock::to_time_t(time);
   timeStream << std::put_time(std::gmtime(&t), "%F %R");
   std::string timeStr = timeStream.str();

   // GPS Position
   auto roundedLong = std::round(10 * gpsLong) / 10;
   auto roundedLat = std::round(10 * gpsLat) / 10;
   auto roundedElev = std::round(10 * gpsElev) / 10;

   // Query the database
   StarDatabaseSchema::TargetBody tb;
   StarDatabaseSchema::Ephemeris eph;
   auto jt = tb.join(eph).on(tb.bodyId == eph.bodyId);
   /*auto results = myDatabaseConnection(select(tb.bodyId, tb.bodyName, eph.azimuth, eph.elevation)
                                     .from(jt)
                                     .where(
                                          (tb.bodyName == targetName || tb.bodyId == targetName) && 
                                          eph.time == timeStr &&
                                          eph.gpsLong == roundedLong &&
                                          eph.gpsLat == roundedLat &&
                                          eph.gpsElev == roundedElev
                                       ));
   
   // Interpret results and determine if API call is necessary
   if (results.empty())
   {
      // API call is needed
      auto endTime = time + std::chrono::hours(24 * 7);
      auto responseCode = myStarApi.performApiQuery(targetName, time, endTime, std::chrono::milliseconds(125), roundedLat, roundedLong, roundedElev);
      auto responseResult = myStarApi.getQueryResults();
      if (responseCode == ApiResponseEnum::SUCCESS)
      {
         // Insert the result into the database
         //auto multiInsert = insert_into();
         for (const auto& row : responseResult)
         {
            std::stringstream ss(row);
            std::string unused;
            std::string timestamp;
            std::string azStr;
            std::string elStr;
            getline(ss, timestamp, ',');
            getline(ss, unused, ',');
            getline(ss, unused, ',');
            getline(ss, azStr, ',');
            getline(ss, elStr, ',');

         }
         // Return the desired az/el
      }
      else if (responseCode == ApiResponseEnum::NO_MATCH)
      {
         // Prompt the user for more specific search
         // Remember the name of the last search for insert into table later
         return false;
      }
      else
      {
         // The API failed to return a result
         return false;
      }
   }
   else
   {
      // Return results of database query
      for (const auto& result : results)
      {
         *azimuth = result.azimuth;
         *elevation = result.elevation;
         break; // Only look at the first entry (Should be only entry. Currently unable to check)
      }
   }*/
   return true;   
}

void StarDatabaseAdapter::queryTargetsWithinRange(double rangeInLightMinutes)
{
   
}