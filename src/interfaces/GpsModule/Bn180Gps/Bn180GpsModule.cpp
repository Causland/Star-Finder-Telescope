#include "interfaces/GpsModule/Bn180Gps/Bn180GpsModule.hpp"
#include "Logger.hpp"
#include <cstring>
#include <iostream>
#include <wiringPi.h>
#include <wiringSerial.h>

constexpr int SERIAL_BAUD_RATE{9600};
constexpr double MINUTES_PER_DEGREE{60.0};
constexpr uint32_t LATITUDE_FIELD_SIZE{10};
constexpr uint32_t LONGITUDE_FIELD_SIZE{11};
constexpr uint32_t GPS_LOOKUP_PERIOD{30};

Bn180GpsModule::Bn180GpsModule(const std::string& serialDevice)
{
   // Start up the serial connection
   mySerialFd = serialOpen(serialDevice.c_str(), SERIAL_BAUD_RATE);
   wiringPiSetup();

   // Only start the GPS thread if the serial port is opened
   if (mySerialFd != -1)
   {
      myThread = std::thread(&Bn180GpsModule::threadLoop, this);
   }
   else
   {
      throw std::runtime_error("Could not open serial port: " + serialDevice);
   }
}

Bn180GpsModule::~Bn180GpsModule()
{
   myExitFlag.store(true);
   if (myThread.joinable()) 
   {
      myThread.join();
   }
   serialClose(mySerialFd);
}

bool Bn180GpsModule::getGpsPosition(double* latitude, double* longitude, double* elevation)
{
   std::scoped_lock lk(myMutex);
   *latitude = myLatitude;
   *longitude = myLongitude;
   *elevation = myElevation;
   return myGpsLockFlag;
}

void Bn180GpsModule::threadLoop()
{
   while(!myExitFlag.load())
   {
      // Read the serial input for GPS data
      auto rc = serialDataAvail(mySerialFd);
      if (rc == -1)
      {
         Logger::log("GpsParser", LogCodeEnum::ERROR, 
                        "Check for serial data failed: errno=" + std::to_string(errno) + 
                        " " + std::string(std::strerror(errno)));
      }
      else
      {
         // Read until new line is found. Append to carryover to continue from
         // any missed characters on the last loop
         for (auto i=0; i<rc; ++i)
         {
            int cint = serialGetchar(mySerialFd);
            if (cint == -1)
            {
               break; // There's no more characters left
            }

            serialChars.push_back(static_cast<char>(cint));
            if (serialChars.back() == '\n')
            {
               std::istringstream iss{serialChars};
               parseNmea(iss);
            }
            serialChars.clear();
         }
      }

      std::this_thread::sleep_for(std::chrono::seconds(GPS_LOOKUP_PERIOD));
   }
}

bool Bn180GpsModule::parseNmea(std::istringstream& messages)
{
   std::string line;
   while (messages >> line)
   {
      if (line.find("GGA") != std::string::npos)
      {
         std::istringstream iss{line};
         return parseNmeaGGA(iss);
      }
      if (line.find("RMC") != std::string::npos)
      {
         std::istringstream iss{line};
         return parseNmeaRMC(iss);
      }
      if (line.find("GLL") != std::string::npos)
      {
         std::istringstream iss{line};
         return parseNmeaGLL(iss);
      }
   }
   return false; // None of the supported messages were received from the module
}

bool Bn180GpsModule::parseNmeaGGA(std::istringstream& message)
{
   std::string fieldVal;

   // GGA type (skipped)
   std::getline(message, fieldVal, ',');

   // Parse UTC time (skipped)
   std::getline(message, fieldVal, ',');

   // Parse latitude
   std::getline(message, fieldVal, ',');
   if (fieldVal.size() != LATITUDE_FIELD_SIZE) // ddmm.mmmmm
   {
      Logger::log("GpsParser", LogCodeEnum::ERROR, "Invalid size lat field: " + fieldVal);
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
      Logger::log("GpsParser", LogCodeEnum::ERROR, 
                     "Unable to convert deg/min into latitude. deg=" 
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
      Logger::log("GpsParser", LogCodeEnum::ERROR, "Invalid size long field: " + fieldVal);
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
      Logger::log("GpsParser", LogCodeEnum::ERROR, 
                     "Unable to convert deg/min into longitude. deg=" 
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
      Logger::log("GpsParser", LogCodeEnum::ERROR, 
                     "Unable to convert altitude into double. alt=" 
                     + fieldVal + " exception=" + e.what());
      return false;
   }

   myLatitude = latitude;
   myLongitude = longitude;
   myElevation = altitude;
   return true;
}

bool Bn180GpsModule::parseNmeaRMC(std::istringstream& message)
{
   std::string fieldVal;

   // RMC type (skipped)
   std::getline(message, fieldVal, ',');

   // UTC timestamp (skipped)
   std::getline(message, fieldVal, ',');

   // Status
   std::getline(message, fieldVal, ',');
   if (fieldVal != "A")
   {
      Logger::log("GpsParser", LogCodeEnum::WARNING, 
                     "RMC status not indicating valid");
      return false;
   }

   // Parse latitude
   std::getline(message, fieldVal, ',');
   if (fieldVal.size() != LATITUDE_FIELD_SIZE) // ddmm.mmmmm
   {
      Logger::log("GpsParser", LogCodeEnum::ERROR, "Invalid size lat field: " + fieldVal);
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
      Logger::log("GpsParser", LogCodeEnum::ERROR, 
                     "Unable to convert deg/min into latitude. deg=" 
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
      Logger::log("GpsParser", LogCodeEnum::ERROR, "Invalid size long field: " + fieldVal);
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
      Logger::log("GpsParser", LogCodeEnum::ERROR, 
                     "Unable to convert deg/min into longitude. deg=" 
                     + degStr + " min=" + minStr + " exception=" + e.what());
      return false;
   }

   // Parse EW indicator
   std::getline(message, fieldVal, ',');
   if (fieldVal != "E")
   {
      longitude *= -1;
   }

   myLatitude = latitude;
   myLongitude = longitude;
   return true;
}

bool Bn180GpsModule::parseNmeaGLL(std::istringstream& message)
{
   std::string fieldVal;

   // GLL type (skipped)
   std::getline(message, fieldVal, ',');

   // Parse latitude
   std::getline(message, fieldVal, ',');
   if (fieldVal.size() != LATITUDE_FIELD_SIZE) // ddmm.mmmmm
   {
      Logger::log("GpsParser", LogCodeEnum::ERROR, "Invalid size lat field: " + fieldVal);
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
      Logger::log("GpsParser", LogCodeEnum::ERROR, 
                     "Unable to convert deg/min into latitude. deg=" 
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
      Logger::log("GpsParser", LogCodeEnum::ERROR, "Invalid size long field: " + fieldVal);
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
      Logger::log("GpsParser", LogCodeEnum::ERROR, 
                     "Unable to convert deg/min into longitude. deg=" 
                     + degStr + " min=" + minStr + " exception=" + e.what());
      return false;
   }

   // Parse EW indicator
   std::getline(message, fieldVal, ',');
   if (fieldVal != "E")
   {
      longitude *= -1;
   }

   myLatitude = latitude;
   myLongitude = longitude;
   return true;
}