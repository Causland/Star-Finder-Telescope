#include "interfaces/GpsModule/Bn180Gps/Bn180GpsModule.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iomanip>
#include <sstream>

static constexpr double MINUTES_PER_DEGREE{60.0};
static constexpr uint32_t LATITUDE_FIELD_SIZE{10};
static constexpr uint32_t LONGITUDE_FIELD_SIZE{11};
static constexpr std::chrono::milliseconds STOP_CHECK_PERIOD{1000};

Bn180GpsModule::Bn180GpsModule(Serial&& serialDevice, const uint32_t& lookupPeriodInS) :
   mySerial{std::move(serialDevice)},
   myLookupPeriod{lookupPeriodInS},
   myLastLookupTime{std::chrono::system_clock::now() - myLookupPeriod}
{
   // Start the GPS thread. By now, the construction of
   // the serial port would have thrown an exception
   myThread = std::thread{&Bn180GpsModule::threadLoop, this};
}

Bn180GpsModule::~Bn180GpsModule()
{
   myExitFlag = true;
   if (myThread.joinable()) 
   {
      myThread.join();
   }
}

bool Bn180GpsModule::getGpsPosition(GpsPosition* position)
{
   std::scoped_lock lock(myMutex);
   *position = myGpsPosition;
   return myGpsLockFlag;
}

std::string Bn180GpsModule::getDisplayInfo()
{
   std::scoped_lock lock(myMutex);
   std::ostringstream oss;
   oss << std::fixed << std::internal
       << "GPS: " << std::setprecision(4) << myGpsPosition.myLatitude
       << "(deg), " << std::setprecision(4) << myGpsPosition.myLongitude
       << "(deg), " << std::setprecision(2) << myGpsPosition.myElevation
       << "(m)";
   return oss.str();
}

void Bn180GpsModule::threadLoop()
{
   while(!myExitFlag.load())
   {
      const auto now{std::chrono::system_clock::now()};
      if (now - myLastLookupTime >= myLookupPeriod)
      {
         myLastLookupTime = now;

         // Read the serial input for GPS data
         const auto readBytes{mySerial.readFromSerial(myRawSerialData.data(), myRawSerialData.size())};
         if (readBytes == -1)
         {
            LOG_ERROR("Check for serial data failed: errno=" + std::to_string(errno) + 
                        " " + std::string(std::strerror(errno)));
         }
         else if (readBytes > 0)
         {
            // We got some text from the serial port so start processing
            for (auto iter=myRawSerialData.cbegin(); iter != myRawSerialData.cend(); ++iter)
            {
               auto newline{std::find(iter, myRawSerialData.cend(), '\n')};
               if (newline != myRawSerialData.cend() && 
                   parseNmea(myLeftoverChars + std::string{iter, newline}))
               {
                  break;
               }
            }

            // Check for leftover characters and save off
            const auto lastNewline{std::next(std::find(myRawSerialData.crbegin(), myRawSerialData.crend(), '\n')).base()};
            myLeftoverChars = std::string{std::next(lastNewline), myRawSerialData.cend()};
         }
      }
      std::this_thread::sleep_for(STOP_CHECK_PERIOD);
   }
}

bool Bn180GpsModule::parseNmea(std::string_view message)
{
   if (message.find("GGA") != std::string_view::npos)
   {
      return parseNmeaGGA(message);
   }
   if (message.find("RMC") != std::string_view::npos)
   {
      return parseNmeaRMC(message);
   }
   if (message.find("GLL") != std::string_view::npos)
   {
      return parseNmeaGLL(message);
   }
   return false; // None of the supported messages were received from the module
}

bool Bn180GpsModule::parseNmeaGGA(std::string_view messageStr)
{
   std::string fieldVal;

   // GGA type (skipped)
   skipField(messageStr);

   // Parse UTC time (skipped)
   skipField(messageStr);

   // Parse latitude
   extractField(messageStr, &fieldVal);
   if (fieldVal.size() != LATITUDE_FIELD_SIZE) // ddmm.mmmmm
   {
      LOG_ERROR("Invalid size lat field: " + fieldVal);
      return false;
   }
   double latitude{0.0};
   try
   {
      latitude += std::stod(fieldVal.substr(0, 2));
      latitude += (std::stod(fieldVal.substr(2)) / MINUTES_PER_DEGREE);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert deg/min into latitude. exception=" + std::string{e.what()});
      return false;
   }

   // Parse NS Indicator
   extractField(messageStr, &fieldVal);
   if (fieldVal != "N")
   {
      latitude *= -1;
   }

   // Parse longitude
   extractField(messageStr, &fieldVal);
   if (fieldVal.size() != LONGITUDE_FIELD_SIZE) // dddmm.mmmmm
   {
      LOG_ERROR("Invalid size long field: " + fieldVal);
      return false;
   }
   double longitude{0.0};
   try
   {
      longitude += std::stod(fieldVal.substr(0, 3));
      longitude += (std::stod(fieldVal.substr(3)) / MINUTES_PER_DEGREE);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert deg/min into longitude. exception=" + std::string{e.what()});
      return false;
   }

   // Parse EW indicator
   extractField(messageStr, &fieldVal);
   if (fieldVal != "E")
   {
      longitude *= -1;
   }

   // Parse quality (skipped)
   skipField(messageStr);

   // Parse num satellites (skipped)
   skipField(messageStr);

   // Parse HDOP (skipped)
   skipField(messageStr);

   // Parse altitude
   extractField(messageStr, &fieldVal);
   double altitude{0.0};
   try
   {
      altitude = std::stod(fieldVal);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert altitude into double. exception=" + std::string{e.what()});
      return false;
   }

   {
      std::scoped_lock lock{myMutex};
      myGpsPosition.myLatitude = latitude;
      myGpsPosition.myLongitude = longitude;
      myGpsPosition.myElevation = altitude;
      myGpsLockFlag = true;
   }
   return true;
}

bool Bn180GpsModule::parseNmeaRMC(std::string_view messageStr)
{
   std::string fieldVal;

   // RMC type (skipped)
   skipField(messageStr);

   // UTC timestamp (skipped)
   skipField(messageStr);

   // Status
   extractField(messageStr, &fieldVal);
   if (fieldVal != "A")
   {
      LOG_WARN("RMC status not indicating valid");
      return false;
   }

   // Parse latitude
   extractField(messageStr, &fieldVal);
   if (fieldVal.size() != LATITUDE_FIELD_SIZE) // ddmm.mmmmm
   {
      LOG_ERROR("Invalid size lat field: " + fieldVal);
      return false;
   }
   double latitude{0.0};
   try
   {
      latitude += std::stod(fieldVal.substr(0, 2));
      latitude += (std::stod(fieldVal.substr(2)) / MINUTES_PER_DEGREE);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert deg/min into latitude. exception=" + std::string{e.what()});
      return false;
   }

   // Parse NS Indicator
   extractField(messageStr, &fieldVal);
   if (fieldVal != "N")
   {
      latitude *= -1;
   }

   // Parse longitude
   extractField(messageStr, &fieldVal);
   if (fieldVal.size() != LONGITUDE_FIELD_SIZE) // dddmm.mmmmm
   {
      LOG_ERROR("Invalid size long field: " + fieldVal);
      return false;
   }
   double longitude{0.0};
   try
   {
      longitude += std::stod(fieldVal.substr(0, 3));
      longitude += (std::stod(fieldVal.substr(3)) / MINUTES_PER_DEGREE);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert deg/min into longitude. exception=" + std::string{e.what()});
      return false;
   }

   // Parse EW indicator
   extractField(messageStr, &fieldVal);
   if (fieldVal != "E")
   {
      longitude *= -1;
   }

   {
      std::scoped_lock lock{myMutex};
      myGpsPosition.myLatitude = latitude;
      myGpsPosition.myLongitude = longitude;
      myGpsLockFlag = true;
   }
   return true;
}

bool Bn180GpsModule::parseNmeaGLL(std::string_view messageStr)
{
   std::string fieldVal;

   // GLL type
   skipField(messageStr);

   // Parse latitude
   extractField(messageStr, &fieldVal);
   if (fieldVal.size() != LATITUDE_FIELD_SIZE) // ddmm.mmmmm
   {
      LOG_ERROR("Invalid size lat field: " + fieldVal);
      return false;
   }
   double latitude{0.0};
   try
   {
      latitude += std::stod(fieldVal.substr(0, 2));
      latitude += (std::stod(fieldVal.substr(2)) / MINUTES_PER_DEGREE);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert deg/min into latitude. exception=" + std::string{e.what()});
      return false;
   }

   // Parse NS Indicator
   extractField(messageStr, &fieldVal);
   if (fieldVal != "N")
   {
      latitude *= -1;
   }

   // Parse longitude
   extractField(messageStr, &fieldVal);
   if (fieldVal.size() != LONGITUDE_FIELD_SIZE) // dddmm.mmmmm
   {
      LOG_ERROR("Invalid size long field: " + fieldVal);
      return false;
   }
   double longitude{0.0};
   try
   {
      longitude += std::stod(fieldVal.substr(0, 3));
      longitude += (std::stod(fieldVal.substr(3)) / MINUTES_PER_DEGREE);
   }
   catch(const std::exception& e)
   {
      LOG_ERROR("Unable to convert deg/min into longitude. exception=" + std::string{e.what()});
      return false;
   }

   // Parse EW indicator
   extractField(messageStr, &fieldVal);
   if (fieldVal != "E")
   {
      longitude *= -1;
   }

   {
      std::scoped_lock lock(myMutex);
      myGpsPosition.myLatitude = latitude;
      myGpsPosition.myLongitude = longitude;
      myGpsLockFlag = true;
   }
   return true;
}
