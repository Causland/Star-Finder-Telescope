#ifndef SIM_STARDATABASE_HPP
#define SIM_STARDATABASE_HPP

#include "interfaces/StarDatabase/IStarDatabase.hpp"

/*!
 * A simulated database which contains canned data for searches and queries.
 */ 
class SimStarDatabase : public IStarDatabase
{
public:
   /*!
    * Creates a SimStarDatabase.
    */
   SimStarDatabase() = default;

   /*!
    * Destroys a SimStarDatabase.
    */ 
   ~SimStarDatabase() override = default;

   SimStarDatabase(const SimStarDatabase&) = delete;
   SimStarDatabase& operator=(const SimStarDatabase&) = delete;
   SimStarDatabase(SimStarDatabase&&) = delete;
   SimStarDatabase& operator=(SimStarDatabase&&) = delete;

   /*!
    * Search for targets by name.
    * \param[in] searchName the name to search for.
    * \return the result of the search.
    */ 
   QueryResult searchTargetsByName(std::string_view searchName) override;

   /*!
    * Search for targets within a specified range.
    * \param[in] rangeInLightYears the range to search for.
    * \return the result of the search.
    */ 
   QueryResult searchTargetsByRange(const double& rangeInLightYears) override;


   /*!
    * Search for targets greater than the specified luminosity. 
    * \param[in] watts the minimum luminosity to search for.
    * \return the result of the search.
    */ 
   QueryResult searchTargetsByLuminosity(const double& watts) override;

   /*!
    * Get the target info for a specific time with the observer at the provided GPS position.
    * \param[in] targetName the name of the target.
    * \param[in] time a time point to get data for.
    * \param[in] gpsPosition the GPS position of the observer.
    * \return the result of the query.
    */ 
   QueryResult queryTargetPointing(const std::string& targetName, const std::chrono::system_clock::time_point& time, 
                                   const GpsPosition& gpsPosition) override;

   /*!
    * Get the target info for a specific time range with the observer at the provided GPS position.
    * \param[in] targetName the name of the target.
    * \param[in] startTime the start time point.
    * \param[in] duration the duration of the trajectory.
    * \param[in] gpsPosition the GPS position of the observer.
    * \return the result of the query.
    */ 
   QueryResult queryTargetPointingTrajectory(const std::string& targetName, const std::chrono::system_clock::time_point& startTime, 
                                                      const std::chrono::seconds& duration, const GpsPosition& gpsPosition) override;

   /*!
    * Get information about the database in a formatted string.
    */ 
   std::string getDisplayInfo() override { return ""; };

private:
   QueryResult myResult{QueryResultEnum::SUCCESS}; //!< The query result returned by all functions.
};

#endif
