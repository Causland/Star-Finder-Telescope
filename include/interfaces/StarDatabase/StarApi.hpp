#ifndef STAR_API_HPP
#define STAR_API_HPP

#include "curlpp/cURLpp.hpp"
#include "curlpp/Easy.hpp"
#include "curlpp/Options.hpp"
#include <chrono>
#include <sstream>
#include <string>
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
   explicit ApiResponse(ApiResponseEnum type) : myType(type) {}

   ApiResponseEnum myType{SUCCESS}; //!< The type of response from the API.
   std::vector<std::string> myResponseResults; //!< A vector of strings containing lines of parsed response content.
};

/*!
 * The StarApi class is responsible for querying the NASA/JPL Horizons API and returning
 * the response in a parsed vector of strings where each entry is either a datapoint or a
 * line of text.
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
    * \sa encodeParameterValue(), interpretAndStoreResponse()
    * \param[in] targetName a string of the target's name.
    * \param[in] startTime a system clock time point to begin the query.
    * \param[in] endTime a system clock time point to end the query.
    * \param[in] timePeriod a time interval to sub divide the beginning and end time.
    * \param[in] latitude a latitude position relative to the WGS-84 GPS reference frame.
    * \param[in] longitude a longitude position relative to the WGS-84 GPS reference frame.
    * \param[in] elevation an elevation position relative to the WGS-84 GPS reference frame.
    * \return an ApiResponse with the result of the API call.
    */
   ApiResponse performApiQuery(const std::string& targetName, 
                               const std::chrono::system_clock::time_point& startTime, 
                               const std::chrono::system_clock::time_point& endTime, 
                               const std::chrono::milliseconds& timePeriod, 
                               const double& latitude, const double& longitude, const double& elevation);

private:
   /*!
    * Calls the cURL function curl_easy_escape() to conver the given input string to a URL encoded string.
    * The cURL function returns a newly allocated C string which needs to be deallocated using the curl_free() function.
    * \param[out] param a parameter value to covert into a URL encoded string. This string is edited directly.
    */
   void encodeParameterValue(std::string* param);

   /*!
    * Processes the query result stream line by line to determine whether the result is a success, has user options,
    * returned no match, or failed. The function first verifies that the API version and source are correct. It then
    * checks for specific indicators to determine the response type and returns the data.
    * \param[in] response a stringstream containing the response of the API query.
    * \return an ApiResponse with the result of the interpreted query.
    */
   ApiResponse interpretAndStoreResponse(std::stringstream& response);

   curlpp::Easy myEasyCurlRequest; //!< The curlpp easy request object used to make an API query.
   const std::string myBaseApiQuery{"https://ssd.jpl.nasa.gov/api/horizons.api?format=text&OBJ_DATA='NO'&MAKE_EPHEM='YES'&EPHEM_TYPE='OBSERVER'"
                                        "&CENTER='coord'&COORD_TYPE='GEODETIC'&QUANTITIES='4,20'&ANG_FORMAT='DEG'&APPARENT='REFRACTED'"
                                        "&TIME_DIGITS='SECONDS'&RANGE_UNITS='KM'&SKIP_DAYLT='YES'&ELEV_CUT='0'&CSV_FORMAT='YES'"}; //!< The base NASA/JPL Horizons API query used. Variable queries get added in the performApiQuery() function.
};

#endif