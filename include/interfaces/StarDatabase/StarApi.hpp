#ifndef STAR_API_HPP
#define STAR_API_HPP

#include "interfaces/GpsModule/IGpsModule.hpp"
#include "Common.hpp"
#include "curlpp/cURLpp.hpp"

#include <chrono>
#include <istream>
#include <string>
#include <variant>
#include <vector>

/*!
 * Enum class of different NASA/JPL Horizons API response types that can be returned.
 */
enum class ApiResponseEnum
{
   SUCCESS, //!< The API returned datapoints regarding the selected target.
   USER_OPTIONS, //!< The API returned multiple target options that need to be viewed by the user.
   NO_MATCH, //!< The API returned no match for the selected target.
   FAILURE, //!< The query could not be processed by the server or the server could not be reached.
};

/*!
 * Holds the response type and content of a NASA/JPL Horizons API call.
 */
struct ApiResponse
{
   using ApiRspSuccess = std::vector<TrajectoryPoint>;
   using ApiRspOptions = std::vector<std::string>;
   
   /*!
    * Create a response with the specified response type.
    * \param[in] type the result of the response.
    */
   explicit ApiResponse(const ApiResponseEnum type) : myType{type} {}

   /*!
    * Create a response with a specified type and variant.
    * \param[in] type the type of the response.
    * \param[in] result the content of the response.
    */
   ApiResponse(const ApiResponseEnum type, 
               std::variant<ApiRspSuccess, ApiRspOptions, std::string>&& result) : 
                  myType{type}, myResponseResults{std::move(result)} {}

   ApiResponseEnum myType{ApiResponseEnum::FAILURE}; //!< The type of response from the API.
   std::variant<ApiRspSuccess, ApiRspOptions, std::string> myResponseResults; //!< A variant containing either a trajectory, options, or comment.
};

/*!
 * The StarApi class is responsible for querying the NASA/JPL Horizons API and returning
 * the response. The contents of the response may vary depending on the result.
 */
class StarApi
{
public:
   /*!
    * Creates a StarApi object. Calls the curlpp initialize function required before any
    * other cURL calls can be made.
    */
   StarApi()
   {
      curlpp::initialize();
   }

   /*!
    * Destroys a StarApi object. Calls the curlpp terminate function required to cleanup
    * cURL resources that have been initialized.
    */
   ~StarApi()
   {
      curlpp::terminate();
   }

   StarApi(const StarApi&) = delete;
   StarApi(StarApi&&) = delete;
   void operator=(const StarApi&) = delete;
   void operator=(StarApi&&) = delete;

   /*!
    * Perform an API query of the NASA/JPL Horizons API with a specified target, start time, 
    * end time, time interval, latitude, longitude, and elevation. The time points provided are
    * converted into strings of format YYYY-MM-DD HH:MM. All value strings are preprocessed into
    * URL format to escape special characters. The beginning and end time range is sub divided
    * into a number of counts based on the provided time interval.
    * \param[in] targetName a string of the target's name.
    * \param[in] startTime a system clock time point to begin the query.
    * \param[in] endTime a system clock time point to end the query.
    * \param[in] timePeriod a time interval to sub divide the beginning and end time.
    * \param[in] gpsPosition a position relative to the WGS-84 GPS reference frame.
    * \return an ApiResponse with the result of the API call.
    */
   static ApiResponse performApiQuery(const std::string& targetName, 
                                      const std::chrono::system_clock::time_point& startTime, 
                                      const std::chrono::system_clock::time_point& endTime, 
                                      const std::chrono::milliseconds& timePeriod, 
                                      const GpsPosition& position);

private:
   /*!
    * Processes the query result stream line by line to determine whether the result is a success, has user options,
    * returned no match, or failed. The function first verifies that the API version and source are correct. It then
    * checks for specific indicators to determine the response type and returns the data.
    * \param[in] response a stream containing the response of the API query.
    * \return an ApiResponse with the result of the interpreted query.
    */
   static ApiResponse processResponse(std::istream& response);

   /*!
    * Detect which type of response was received from the API. The types are defined in the 
    * ApiResponseEnum.
    * \param[in] response a stream containing the response of the API query.
    * \return the type of response contained in the remaining stream.
    */ 
   static ApiResponseEnum detectResponseType(std::istream& response);

   /*!
    * Process a successful response by parsing out data points.
    * \param[in] response a stream containing a successful response from the API.
    * \return an ApiResponse with the successful result and data.
    */ 
   static ApiResponse processSuccessResponse(std::istream& response);

   /*!
    * Process an options response by parsing out the available options.
    * \param[in] response a stream containing an options response from the API.
    * \return an ApiResponse with the options result and the parsed options.
    */ 
   static ApiResponse processOptionsResponse(std::istream& response);
};

#endif
