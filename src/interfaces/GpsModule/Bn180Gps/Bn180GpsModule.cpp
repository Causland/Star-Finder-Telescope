#include "interfaces/GpsModule/Bn180Gps/Bn180GpsModule.hpp"
#include "Logger.hpp"
#include <cerrno>
#include <cstring>
#include <iomanip>
#include <sstream>

constexpr double MINUTES_PER_DEGREE{60.0};
constexpr uint32_t LATITUDE_FIELD_SIZE{10};
constexpr uint32_t LONGITUDE_FIELD_SIZE{11};
constexpr std::chrono::milliseconds STOP_CHECK_PERIOD{1000};

Bn180GpsModule::Bn180GpsModule(const std::string& serialDevice, const uint8_t& serialTimeout, const uint32_t& lookupPeriodInS) :
   mySerial(serialDevice, O_RDWR, B9600, serialTimeout),
   myLookupPeriod(lookupPeriodInS),
   myLastLookupTime(std::chrono::system_clock::now() - myLookupPeriod)
{
   // Start the GPS thread. By now, the construction of
   // the serial port would have thrown an exception
   myThread = std::thread(&Bn180GpsModule::threadLoop, this);
}

Bn180GpsModule::~Bn180GpsModule()
{
   myExitFlag.store(true);
   if (myThread.joinable()) 
   {
      myThread.join();
   }
}

bool Bn180GpsModule::getGpsPosition(double* latitude, double* longitude, double* elevation)
{
   std::scoped_lock lk(myMutex);
   *latitude = myLatitude;
   *longitude = myLongitude;
   *elevation = myElevation;
   return myGpsLockFlag;
}

std::string Bn180GpsModule::getDisplayInfo()
{
   std::scoped_lock lk(myMutex);
   std::stringstream ss;
   ss << std::fixed << std::internal
      << "GPS: " << std::setprecision(4) << myLatitude
      << "(deg), " << std::setprecision(4) << myLongitude
      << "(deg), " << std::setprecision(2) << myElevation
      << "(m)";
   return ss.str();
}

void Bn180GpsModule::threadLoop()
{
   while(!myExitFlag.load())
   {
      auto now = std::chrono::system_clock::now();
      if (now - myLastLookupTime >= myLookupPeriod)
      {
         myLastLookupTime = now;
         // Read the serial input for GPS data
         ssize_t readBytes = mySerial.readFromSerial(myRawSerialData.data(), myRawSerialData.size());
         if (readBytes == -1)
         {
            LOG_ERROR("Check for serial data failed: errno=" + std::to_string(errno) + 
                        " " + std::string(std::strerror(errno)));
         }
         else
         {
            if (readBytes > 0)
            {
               // We got some text from the serial port so start processing
               std::string serialOut(myRawSerialData.begin(), myRawSerialData.begin() + readBytes);
               bool endsWithNewline = (serialOut.back() == '\n');
               if (endsWithNewline)
               {
                  serialOut.pop_back();
               }

               // Push all substrings into a vector for processing.
               // If a message does not end with a newline, add to the leftoverChars string.
               std::vector<std::string> nmeaMsgs;
               std::stringstream msgs{serialOut};
               std::string msg;
               msgs >> msg;
               msg += myLeftoverChars + msg;
               do
               {
                  nmeaMsgs.push_back(msg);
               } 
               while (msgs >> msg);

               if (!endsWithNewline)
               {
                  myLeftoverChars = nmeaMsgs.back();
                  nmeaMsgs.pop_back();
               }
               else
               {
                  myLeftoverChars.clear();
               }

               // Process from most recent to oldest for data
               for (auto rit = nmeaMsgs.rbegin(); rit != nmeaMsgs.rend(); ++rit)
               {
                  if (parseNmea(*rit))
                  {
                     break;
                  }
               }
            }
         }
      }
      std::this_thread::sleep_for(STOP_CHECK_PERIOD);
   }
}

bool Bn180GpsModule::parseNmea(const std::string& message)
{
   if (message.find("GGA") != std::string::npos)
   {
      return parseNmeaGGA(message);
   }
   if (message.find("RMC") != std::string::npos)
   {
      return parseNmeaRMC(message);
   }
   if (message.find("GLL") != std::string::npos)
   {
      return parseNmeaGLL(message);
   }
   return false; // None of the supported messages were received from the module
}

bool Bn180GpsModule::parseNmeaGGA(const std::string& messageStr)
{
   std::istringstream message{messageStr};
   std::string fieldVal;

   // GGA type (skipped)
   std::getline(message, fieldVal, ',');

   // Parse UTC time (skipped)
   std::getline(message, fieldVal, ',');

   // Parse latitude
   std::getline(message, fieldVal, ',');
   if (fieldVal.size() != LATITUDE_FIELD_SIZE) // ddmm.mmmmm
   {
      LOG_ERROR("Invalid size lat field: " + fieldVal);
      return false;
   }
   double latitude = 0.0;
   std::string degStr = fieldVal.substr(0, 2);
   std::string minStr = fieldVal.substr(2);
   try
   {
      latitude += std::stod(degStr);
      latitude += (std::stod(minStr) / MINUTES_PER_DEGREE);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert deg/min into latitude. deg=" 
                  + degStr + " min=" + minStr + " exception=" + e.what());
      return false;
   }

   // Parse NS Indicator
   std::getline(message, fieldVal, ',');
   if (fieldVal != "N")
   {
      latitude *= -1;
   }

   // Parse longitude
   std::getline(message, fieldVal, ',');
   if (fieldVal.size() != LONGITUDE_FIELD_SIZE) // dddmm.mmmmm
   {
      LOG_ERROR("Invalid size long field: " + fieldVal);
      return false;
   }
   double longitude = 0.0;
   degStr = fieldVal.substr(0, 3);
   minStr = fieldVal.substr(3);
   try
   {
      longitude += std::stod(degStr);
      longitude += (std::stod(minStr) / MINUTES_PER_DEGREE);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert deg/min into longitude. deg=" 
                  + degStr + " min=" + minStr + " exception=" + e.what());
      return false;
   }

   // Parse EW indicator
   std::getline(message, fieldVal, ',');
   if (fieldVal != "E")
   {
      longitude *= -1;
   }

   // Parse quality (skipped)
   std::getline(message, fieldVal, ',');

   // Parse num satellites (skipped)
   std::getline(message, fieldVal, ',');

   // Parse HDOP (skipped)
   std::getline(message, fieldVal, ',');

   // Parse altitude
   std::getline(message, fieldVal, ',');
   double altitude = 0.0;
   try
   {
      altitude += std::stod(fieldVal);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert altitude into double. alt=" 
                  + fieldVal + " exception=" + e.what());
      return false;
   }

   {
      std::scoped_lock lk(myMutex);
      myLatitude = latitude;
      myLongitude = longitude;
      myElevation = altitude;
      myGpsLockFlag = true;
   }
   return true;
}

bool Bn180GpsModule::parseNmeaRMC(const std::string& messageStr)
{
   std::istringstream message{messageStr};
   std::string fieldVal;

   // RMC type (skipped)
   std::getline(message, fieldVal, ',');

   // UTC timestamp (skipped)
   std::getline(message, fieldVal, ',');

   // Status
   std::getline(message, fieldVal, ',');
   if (fieldVal != "A")
   {
      LOG_WARN("RMC status not indicating valid");
      return false;
   }

   // Parse latitude
   std::getline(message, fieldVal, ',');
   if (fieldVal.size() != LATITUDE_FIELD_SIZE) // ddmm.mmmmm
   {
      LOG_ERROR("Invalid size lat field: " + fieldVal);
      return false;
   }
   double latitude = 0.0;
   std::string degStr = fieldVal.substr(0, 2);
   std::string minStr = fieldVal.substr(2);
   try
   {
      latitude += std::stod(degStr);
      latitude += (std::stod(minStr) / MINUTES_PER_DEGREE);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert deg/min into latitude. deg=" 
                  + degStr + " min=" + minStr + " exception=" + e.what());
      return false;
   }

   // Parse NS Indicator
   std::getline(message, fieldVal, ',');
   if (fieldVal != "N")
   {
      latitude *= -1;
   }

   // Parse longitude
   std::getline(message, fieldVal, ',');
   if (fieldVal.size() != LONGITUDE_FIELD_SIZE) // dddmm.mmmmm
   {
      LOG_ERROR("Invalid size long field: " + fieldVal);
      return false;
   }
   double longitude = 0.0;
   degStr = fieldVal.substr(0, 3);
   minStr = fieldVal.substr(3);
   try
   {
      longitude += std::stod(degStr);
      longitude += (std::stod(minStr) / MINUTES_PER_DEGREE);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert deg/min into longitude. deg=" 
                  + degStr + " min=" + minStr + " exception=" + e.what());
      return false;
   }

   // Parse EW indicator
   std::getline(message, fieldVal, ',');
   if (fieldVal != "E")
   {
      longitude *= -1;
   }

   {
      std::scoped_lock lk(myMutex);
      myLatitude = latitude;
      myLongitude = longitude;
      myGpsLockFlag = true;
   }
   return true;
}

bool Bn180GpsModule::parseNmeaGLL(const std::string& messageStr)
{
   std::istringstream message{messageStr};
   std::string fieldVal;

   // GLL type (skipped)
   std::getline(message, fieldVal, ',');

   // Parse latitude
   std::getline(message, fieldVal, ',');
   if (fieldVal.size() != LATITUDE_FIELD_SIZE) // ddmm.mmmmm
   {
      LOG_ERROR("Invalid size lat field: " + fieldVal);
      return false;
   }
   double latitude = 0.0;
   std::string degStr = fieldVal.substr(0, 2);
   std::string minStr = fieldVal.substr(2);
   try
   {
      latitude += std::stod(degStr);
      latitude += (std::stod(minStr) / MINUTES_PER_DEGREE);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert deg/min into latitude. deg=" 
                  + degStr + " min=" + minStr + " exception=" + e.what());
      return false;
   }

   // Parse NS Indicator
   std::getline(message, fieldVal, ',');
   if (fieldVal != "N")
   {
      latitude *= -1;
   }

   // Parse longitude
   std::getline(message, fieldVal, ',');
   if (fieldVal.size() != LONGITUDE_FIELD_SIZE) // dddmm.mmmmm
   {
      LOG_ERROR("Invalid size long field: " + fieldVal);
      return false;
   }
   double longitude = 0.0;
   degStr = fieldVal.substr(0, 3);
   minStr = fieldVal.substr(3);
   try
   {
      longitude += std::stod(degStr);
      longitude += (std::stod(minStr) / MINUTES_PER_DEGREE);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert deg/min into longitude. deg=" 
                  + degStr + " min=" + minStr + " exception=" + e.what());
      return false;
   }

   // Parse EW indicator
   std::getline(message, fieldVal, ',');
   if (fieldVal != "E")
   {
      longitude *= -1;
   }

   {
      std::scoped_lock lk(myMutex);
      myLatitude = latitude;
      myLongitude = longitude;
      myGpsLockFlag = true;
   }
   return true;
}