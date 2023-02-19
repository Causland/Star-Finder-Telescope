#ifndef BN_180_GPS_MODULE_HPP
#define BN_180_GPS_MODULE_HPP

#include "interfaces/GpsModule/IGpsModule.hpp"
#include "serial/Serial.hpp"
#include <atomic>
#include <chrono>
#include <mutex>
#include <sstream>
#include <thread>

/*!
 * The BN180 GPS module gathers data from GPS satellites and provides a series of NMEA messages 
 * over serial to the connected device. This class is responsible for reading
 * the messages from the serial port and parsing the NMEA messages into usable position data.
 */
class Bn180GpsModule : public IGpsModule
{
public:
   /*!
    * Creates a Bn180GpsModule object with the provided serial device, the serial timeout period,
    * and the GPS lookup period for updated position. The module uses the serial port to receive
    * NMEA data from the BN180 GPS module and parses out the position data.
    * \param[in] serialDevice a path to a valid serial device which provides NMEA GPS messages.
    * \param[in] serialTimeout a timeout for reading the serial device in deciseconds.
    * \param[in] lookupPeriodInS a period to update the GPS position data.
    */
   explicit Bn180GpsModule(const std::string& serialDevice, const uint8_t& serialTimeout, const uint32_t& lookupPeriodInS);
   
   /*!
    * Destroys a Bn180GpsModule by stopping the threadloop and waiting for it to exit.
    */
   ~Bn180GpsModule();

   Bn180GpsModule(const Bn180GpsModule&) = delete;
   Bn180GpsModule(Bn180GpsModule&&) = delete;
   void operator=(const Bn180GpsModule&) = delete;
   void operator=(Bn180GpsModule&&) = delete;

   /*!
    * Get the latitude, longitude, and elevation of the current position
    * \param[out] latitude a latitude in degrees. Positive if North.
    * \param[out] longitude a longitude in degrees. Positive if East.
    * \param[out] elevation an elevation in meters.
    * \return true if the GPS module has received valid data.
    */
   bool getGpsPosition(double* latitude, double* longitude, double* elevation) override;

   /*!
    * Get information to display on the screen about the GPS module.
    * \return a formatted string of information to display.
    */
   std::string getDisplayInfo() override;

private:
   /*!
    * At the provided GPS lookup period, read all available data from the serial port and
    * process it as NMEA data. Use suporting NMEA processing functions to parse out
    * GPS positioning data.
    * \sa parseNmea()
    */
   void threadLoop();

   /*!
    * Check the beginning of a message string to determine the NMEA message type, then
    * pass the message to the appropriate message parser.
    * \sa parseNmeaGGA(), parseNmeaRMC(), parseNmeaGLL()
    * \param message an NMEA message string.
    * \return true if a matching NMEA parser was found and the parsing is successful.
    */
   bool parseNmea(const std::string& message);

   /*!
    * Parse an NMEA GGA message for latitude, longitude, and altitude data.
    * \param messageStr an NMEA GGA message string.
    * \return true if the data is parsed succesfully.
    */
   bool parseNmeaGGA(const std::string& messageStr);

   /*!
    * Parse an NMEA RMC message for latitude and longitude data.
    * \param messageStr an NMEA RMC message string.
    * \return true if the data is parsed successfully.
    */
   bool parseNmeaRMC(const std::string& messageStr);

   /*!
    * Parse an NMEA GLL message for latitude and longitude data.
    * \param messageStr an NMEA GLL message string.
    * \return true if the data is parsed successfully.
    */
   bool parseNmeaGLL(const std::string& messageStr);
   
   double myLatitude{0.0}; //!< The current latitude. Positive for North.
   double myLongitude{0.0}; //!< The current longitude. Positive for East.
   double myElevation{0.0}; //!< The current elevation.
   bool myGpsLockFlag{false}; //!< The flag used to determine if the GPS module has received position data.
   std::atomic<bool> myExitFlag{false}; //!< The flag used to control the lifetime of the thread loop.
   std::thread myThread; //!< The GPS processing thread.
   std::mutex myMutex; //!< The mutex for accessing position data external to module.
   std::string myLeftoverChars; //!< The characters from the serial device left over from last processing loop.
   Serial mySerial; //!< The serial device controller.
   const std::chrono::seconds myLookupPeriod{60}; //!< The period to look for new GPS data.
   std::chrono::system_clock::time_point myLastLookupTime; //!< The timestamp of the last GPS data update.
};

#endif