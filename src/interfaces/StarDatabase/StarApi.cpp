#include "Logger.hpp"
#include "curlpp/Easy.hpp"
#include "curlpp/Options.hpp"
#include "interfaces/StarDatabase/StarApi.hpp"

#include <iomanip>
#include <string_view>

static constexpr std::string_view API_URL_BASE{"https://ssd.jpl.nasa.gov/api/horizons.api?format=text&OBJ_DATA='NO'&MAKE_EPHEM='YES'&EPHEM_TYPE='OBSERVER'"
                                        "&CENTER='coord'&COORD_TYPE='GEODETIC'&QUANTITIES='4,20'&ANG_FORMAT='DEG'&APPARENT='REFRACTED'"
                                        "&TIME_DIGITS='SECONDS'&RANGE_UNITS='KM'&SKIP_DAYLT='YES'&ELEV_CUT='0'&CSV_FORMAT='YES'"};

ApiResponse StarApi::performApiQuery(const std::string& targetName, 
                                     const std::chrono::system_clock::time_point& startTime, 
                                     const std::chrono::system_clock::time_point& endTime, 
                                     const std::chrono::milliseconds& timePeriod, 
                                     const GpsPosition& position)
{
   curlpp::Easy curlRequest;

   auto encodeParamVal{[curlRequest](const std::string& val) -> std::string {
                        const auto encodedParam{curl_easy_escape(curlRequest.getHandle(), val->c_str(), param->size())};
                        const std::string ret{encodedParam};
                        curl_free(encodedParam);
                        return ret;
                      }};

   // Time calculations
   const auto startMinute{std::chrono::floor<std::chrono::minutes>(startTime)};
   const auto endMinute{std::chrono::ceil<std::chrono::minutes>(endTime)};
   const auto numSteps{(endMinute - startMinute) / timePeriod};

   std::ostringstream stStream;
   const std::time_t tStart{std::chrono::system_clock::to_time_t(startMinute)};
   stStream << std::put_time(std::gmtime(&tStart), "%F %R");
   const std::string startTimeStr{encodeParamVal(stStream.str())};

   std::ostringstream etStream;
   const std::time_t tEnd{std::chrono::system_clock::to_time_t(endMinute)};
   etStream << std::put_time(std::gmtime(&tEnd), "%F %R");
   const std::string endTimeStr{encodeParamVal(etStream.str())};

   // Generate the URL
   std::string queryUrl{};
   queryUrl += "&COMMAND='" + targetName + "'";
   queryUrl += "&SITE_COORD='" + std::to_string(position.myLongitude) + "," + 
                                 std::to_string(position.myLatitude) + "," + 
                                 std::to_string(position.myElevation) + "'";
   queryUrl += "&START_TIME='" + startTimeStr + "'";
   queryUrl += "&STOP_TIME='" + endTimeStr + "'";
   queryUrl += "&STEP_SIZE='" + std::to_string(numSteps) + "'";
   curlRequest.setOpt(curlpp::Options::Url(queryUrl));

   // Create response stream
   std::stringstream response;
   curlRequest.setOpt(curlpp::options::WriteStream(&response));
   
   // Perform request
   LOG_INFO("Performing curl request: " + queryUrl);
   try
   {
      curlRequest.perform();
   }
   catch(const std::exception& e)
   {
      // Add the error to the query result vector and return a failure
      LOG_ERROR("Failed to perform curl request: " + std::string{e.what()});
      return ApiResponse{ApiResponseEnum::FAILURE, "Curl request failed"};
   }

   // Interpret the response
   return processResponse(response);
}

ApiResponse StarApi::processResponse(std::istream& response)
{  
   switch (detectResponseType(response))
   {
      case ApiResponseEnum::SUCCESS:
      {
         return processSuccessResponse(response); 
      }
      case ApiResponseEnum::USER_OPTIONS:
      {
         return processOptionsResponse(response);
      }
      case ApiResponseEnum::NO_MATCH:
      {
         return ApiResponse{ApiResponseEnum::NO_MATCH, "No match found for target"};
      }
   }

   return ApiResponse{ApiResponseEnum::FAILURE, "Invalid response"};
}

ApiResponseEnum StarApi::detectResponseType(std::istream& response)
{
   std::string line;
   while(std::getline(response, line))
   {
      // Skip lines to ignore
      if (line.empty() || line.find("************") != std::string::npos)
      {
         continue;
      }
      // Detect success case
      if (line.find("$$SOE") != std::string::npos)
      {
         return ApiResponseEnum::SUCCESS;
      }
      // Detect options case
      if (line.find("Multiple major-bodies match string") != std::string::npos)
      {
         return ApiResponseEnum::USER_OPTIONS;
      }
      // Detect no match case
      if (line.find("No matches found") != std::string::npos)
      {
         return ApiResponseEnum::NO_MATCH;
      }
   }
   return ApiResponseEnum::FAILURE;
}

ApiResponse StarApi::processSuccessResponse(std::istream& response)
{
   ApiRspSuccess points;
   std::string line;
   while(std::getline(response, line))
   {
      if (line.empty() || 
          line.find(">.....") != std::string::npos || 
          line.find("$$EOE") != std::string::npos)
      {
         continue;
      }

      // At this point, the line is a datapoint. Parse out the data and insert
      std::istringstream data{line};

      // Parse the date and time
      std::string item;
      std::tm datetime;
      date >> std::get_time(&datetime, "%Y-%b-%d %H:%M:%S");
      const auto time{};

      std::getline(data, item, ','); // Sub seconds
      std::getline(data, item, ','); // Skip
      std::getline(data, item, ','); // Skip

      // Parse position data
      Position pos;
      std::getline(data, item, ','); // Az
      try 
      {
         pos.myAzimuth = std::stod(item);
      }
      catch (const std::exception& e) 
      {
         LOG_ERROR("Could not parse azimuth data: " + std::string{e.what()});
         return ApiResponse{ApiResponseEnum::FAILURE, "Parse azimuth failed"};
      }  
      std::getline(dataLine, item, ','); // Az
      try 
      {
         pos.myElevation = std::stod(item);
      }
      catch (const std::exception& e) 
      {
         LOG_ERROR("Could not parse elevation data: " + std::string{e.what()});
         return ApiResponse{ApiResponseEnum::FAILURE, "Parse elevation failed"};
      }  

      points.emplace_back(pos, std::chrono::system_clock::from_time_t(std::mktime(&datetime)));
   }

   return ApiResponse{ApiResponseEnum::SUCCESS, points};
}

ApiResponse StarApi::processOptionsResponse(std::istream& response)
{
   ApiRspOptions options;
   while(std::getline(response, line))
   {
      if (line.empty() || 
          line.find("ID#") != std::string::npos || 
          line.find("----") != std::string::npos ||
          line.find("Number of matches") != std::string::npos ||
          line.find("****") != std::string::npos)
      {
         continue;
      }

      options.push_back(line);
   }

   return ApiResponse{ApiResponseEnum::USER_OPTIONS, options};
}
