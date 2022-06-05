#include "StarApi.hpp"
#include <iomanip>

ApiResponseEnum StarApi::performApiQuery(std::string targetName, std::chrono::system_clock::time_point startTime, std::chrono::system_clock::time_point endTime, const std::chrono::seconds& timePeriod, double latitude, double longitude, double elevation)
{
   myProcessedQueryResults.clear();

   // Time calculations
   auto startMinute = std::chrono::floor<std::chrono::minutes>(startTime);
   auto endMinute = std::chrono::ceil<std::chrono::minutes>(endTime);
   auto numSteps = (endMinute - startMinute) / timePeriod;

   std::ostringstream stStream;
   std::time_t t1 = std::chrono::system_clock::to_time_t(startMinute);
   stStream << std::put_time(std::gmtime(&t1), "%F %R");
   std::string startTimeStr = stStream.str();
   encodeParameterValue(startTimeStr);

   std::ostringstream etStream;
   std::time_t t2 = std::chrono::system_clock::to_time_t(endMinute);
   etStream << std::put_time(std::gmtime(&t2), "%F %R");
   std::string endTimeStr = etStream.str();
   encodeParameterValue(endTimeStr);

   // Generate the URL
   std::string queryUrl = myBaseApiQuery;
   queryUrl += "&COMMAND='" + targetName + "'";
   queryUrl += "&SITE_COORD='" + std::to_string(longitude) + "," + std::to_string(latitude) + "," + std::to_string(elevation) + "'";
   queryUrl += "&START_TIME='" + startTimeStr + "'";
   queryUrl += "&STOP_TIME='" + endTimeStr + "'";
   queryUrl += "&STEP_SIZE='" + std::to_string(numSteps) + "'";
   myEasyCurlRequest.setOpt(curlpp::Options::Url(std::string(queryUrl)));

   // Create response stream
   std::stringstream response;
   myEasyCurlRequest.setOpt(curlpp::options::WriteStream(&response));
   
   // Perform request
   try
   {
      myEasyCurlRequest.perform();
   }
   catch(const std::exception& e)
   {
      // Add the error to the query result vector and return a failure
      myProcessedQueryResults.push_back(e.what());
      return ApiResponseEnum::FAILURE;
   }

   // Interpret the response and store the result for access later
   return interpretAndStoreResponse(response);
}

std::vector<std::string> StarApi::getQueryResults()
{
   return myProcessedQueryResults;
}

void StarApi::encodeParameterValue(std::string& param)
{
   auto encodedParam = curl_easy_escape(myEasyCurlRequest.getHandle(), param.c_str(), param.size());
   param = std::string(encodedParam);
   curl_free(encodedParam);
}

ApiResponseEnum StarApi::interpretAndStoreResponse(std::stringstream& response)
{  
   // 1) Verify that line #3 is "API VERSION: 1.1"
   // 2) Verify that line #4 is "API SOURCE: NASA/JPL Horizons API"
   //      These checks are specified in the Horizons API documentation
   // 3) Skip empty lines or lines with ***********
   //------------------------
   // SUCCESS output
   // 4) Find line that starts with $$SOE
   // 5) Skip lines that star with >..... or empty
   // 6) Find line that starts with $$EOE
   //------------------------
   // OPTIONS output
   // 4) Find line that starts with "Multiple major-bodies match string"
   // 5) Find line that starts with "Number of matches"
   //------------------------
   // NO_MATCH output
   // 4) Find line that starts with "No matches found"

   std::string line;

   // Check for NASA/JPL Horizons output lines
   std::getline(response, line);
   if (line != "API VERSION: 1.1")
   {
      return ApiResponseEnum::FAILURE; // FAILURE
   }
   std::getline(response, line);
   if (line != "API SOURCE: NASA/JPL Horizons API")
   {
      return ApiResponseEnum::FAILURE; // FAILURE
   }
      
   // Detect specific output case
   bool successCase = false;
   bool optionsCase = false;
   bool noMatchCase = false;
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
         successCase = true;
         break;
      }
      // Detect options case
      if (line.find("Multiple major-bodies match string") != std::string::npos)
      {
         optionsCase = true;
         break;
      }
      // Detect no match case
      if (line.find("No matches found") != std::string::npos)
      {
         noMatchCase = true;
         break;
      }
   }

   // Do output case specific processing
   ApiResponseEnum result = ApiResponseEnum::FAILURE;
   if (successCase)
   {
      while(std::getline(response, line))
      {
         if (line.empty() || line.find(">.....") != std::string::npos)
         {
            continue;
         }
         if (line.find("$$EOE") != std::string::npos)
         {
            break;
         }
         // At this point, the line is a datapoint. Push it to the result vector
         myProcessedQueryResults.push_back(line);
      }
      result = ApiResponseEnum::SUCCESS;
   }
   else if (optionsCase)
   {
      while(std::getline(response, line))
      {
         if (line.empty())
         {
            continue;
         }
         myProcessedQueryResults.push_back(line);
         if (line.find("Number of matches") != std::string::npos)
         {
            break;
         }
      }
      result = ApiResponseEnum::USER_OPTIONS;
   }
   else if (noMatchCase)
   {
      result = ApiResponseEnum::NO_MATCH;
   }

   return result;
}