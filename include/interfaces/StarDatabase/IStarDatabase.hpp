#ifndef I_STAR_DATABASE_HPP
#define I_STAR_DATABASE_HPP

#include "interfaces/GpsModule/IGpsModule.hpp"
#include "Common.hpp"

#include <chrono>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

/*!
 * Enum class of the various query results.
 */ 
enum class QueryResultEnum
{
   SUCCESS, //!< The requested query was successful.
   NO_MATCH, //!< There was no match for the query.
   INVALID_PARAM, //!< An invalid query parameter was used.
   FAILURE, //!< The query failed.
};

/*!
 * Holds the result of a query and other useful information.
 */ 
struct QueryResult
{
   using QueryRsltSearch = std::vector<std::string>;
   using QueryRsltTrajectory = std::vector<TrajectoryPoint>;

   /*!
    * Create a response with the specified result status.
    * \param[in] status the status of the query.
    */
   explicit QueryResult(const QueryResultEnum status) : myStatus{status} {}

   /*!
    * Create a response with a specified result status and variant.
    * \param[in] status the status of the query.
    * \param[in] result the content of the query.
    */
   QueryResult(const QueryResultEnum status, 
               std::variant<QueryRsltSearch, QueryRsltTrajectory, std::string>&& result) : 
                  myStatus{status}, myQueryResults{std::move(result)} {}

   QueryResultEnum myStatus{QueryResultEnum::FAILURE}; //!< The status of the response.
   std::variant<QueryRsltSearch, QueryRsltTrajectory, std::string> myQueryResults; //!< A variant containing either search results, trajectory or a message.
};

class IStarDatabase
{
public:
   /*!
    * Creates a Star Database.
    */
   IStarDatabase() = default;

   /*!
    * Destroys a Star Database.
    */ 
   virtual ~IStarDatabase() = default;

   IStarDatabase(const IStarDatabase&) = delete;
   IStarDatabase& operator=(const IStarDatabase&) = delete;
   IStarDatabase(IStarDatabase&&) = delete;
   IStarDatabase& operator=(IStarDatabase&&) = delete;

   /*!
    * Search for targets by name.
    * \param[in] searchName the name to search for.
    * \return the result of the search.
    */ 
   [[nodiscard]] virtual QueryResult searchTargetsByName(std::string_view searchName) = 0;

   /*!
    * Search for targets within a specified range.
    * \param[in] rangeInLightYears the range to search for.
    * \return the result of the search.
    */ 
   [[nodiscard]] virtual QueryResult searchTargetsByRange(const double& rangeInLightYears) = 0;

   /*!
    * Search for targets greater than the specified luminosity. 
    * \param[in] watts the minimum luminosity to search for.
    * \return the result of the search.
    */ 
   [[nodiscard]] virtual QueryResult searchTargetsByLuminosity(const double& watts) = 0;

   /*!
    * Get the target info for a specific time with the observer at the provided GPS position.
    * \param[in] targetName the name of the target.
    * \param[in] time a time point to get data for.
    * \param[in] gpsPosition the GPS position of the observer.
    * \return the result of the query.
    */ 
   [[nodiscard]] virtual QueryResult queryTargetPointing(const std::string& targetName, const std::chrono::system_clock::time_point& time, 
                                                         const GpsPosition& gpsPosition) = 0;

   /*!
    * Get the target info for a specific time range with the observer at the provided GPS position.
    * \param[in] targetName the name of the target.
    * \param[in] startTime the start time point.
    * \param[in] duration the duration for the trajectory.
    * \param[in] gpsPosition the GPS position of the observer.
    * \return the result of the query.
    */ 
   [[nodiscard]] virtual QueryResult queryTargetPointingTrajectory(const std::string& targetName, const std::chrono::system_clock::time_point& startTime, 
                                                                   const std::chrono::seconds& duration, const GpsPosition& gpsPosition) = 0;

   /*!
    * Get information about the database in a formatted string.
    */ 
   [[nodiscard]] virtual std::string getDisplayInfo() = 0;
};

#endif
