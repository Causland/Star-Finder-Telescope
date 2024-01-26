#ifndef BN_180_GPS_MODULE_HPP
#define BN_180_GPS_MODULE_HPP

#include "interfaces/GpsModule/IGpsModule.hpp"
#include "serial/Serial.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

static constexpr uint16_t MAX_GPS_MSG_LEN{1024};

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
    * \param[in] serialDevice a serial device provided to the GPS module.
    * \param[in] lookupPeriodInS a period to update the GPS position data.
    */
   Bn180GpsModule(Serial&& serialDevice, const uint32_t& lookupPeriodInS);
   
   /*!
    * Destroys a Bn180GpsModule by stopping the threadloop and waiting for it to exit.
    */
   ~Bn180GpsModule() override;

   Bn180GpsModule(const Bn180GpsModule&) = delete;
   Bn180GpsModule(Bn180GpsModule&&) = delete;
   void operator=(const Bn180GpsModule&) = delete;
   void operator=(Bn180GpsModule&&) = delete;

   /*!
    * Get the GPS position reported by the module.
    * \param[out] position the GPS position data.
    * \return true if the position is known.
    */ 
   bool getGpsPosition(GpsPosition* position) override;

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
   bool parseNmea(std::string_view message);

   /*!
    * Parse an NMEA GGA message for latitude, longitude, and altitude data.
    * \param messageStr an NMEA GGA message string.
    * \return true if the data is parsed succesfully.
    */
   bool parseNmeaGGA(std::string_view messageStr);

   /*!
    * Parse an NMEA RMC message for latitude and longitude data.
    * \param messageStr an NMEA RMC message string.
    * \return true if the data is parsed successfully.
    */
   bool parseNmeaRMC(std::string_view messageStr);

   /*!
    * Parse an NMEA GLL message for latitude and longitude data.
    * \param messageStr an NMEA GLL message string.
    * \return true if the data is parsed successfully.
    */
   bool parseNmeaGLL(std::string_view messageStr);

   /*!
    * Message parsing helper function. Skip to the next field in the string view.
    * \param[in,out] messageStr a string view with a field to skip.
    */ 
   static constexpr void skipField(std::string_view& messageStr)
   {
      const auto posComma{messageStr.find(',')};

      if (posComma == std::string_view::npos)
      {
         messageStr.remove_prefix(messageStr.size());
      }
      else 
      {
         messageStr.remove_prefix(posComma + 1); 
      }
   }

   /*!
    * Message parsing helper function. Save off the current field of the string view and
    * move to the beginning of the next field.
    * \param[in,out] messageStr a string view beginning with the field to processes.
    * \param[out] fieldVal the value of the extracted field.
    */ 
   static constexpr void extractField(std::string_view& messageStr, std::string* fieldVal)
   {
      const auto posComma{messageStr.find(',')};
      *fieldVal = messageStr.substr(0, posComma);

      if (posComma == std::string_view::npos)
      {
         messageStr.remove_prefix(messageStr.size());
      }
      else 
      {
         messageStr.remove_prefix(posComma + 1); 
      }
   }
   
   GpsPosition myGpsPosition; //!< The current GPS position.
   bool myGpsLockFlag{false}; //!< The flag used to determine if the GPS module has received position data.
   std::atomic<bool> myExitFlag{false}; //!< The flag used to control the lifetime of the thread loop.
   std::thread myThread; //!< The GPS processing thread.
   std::mutex myMutex; //!< The mutex for accessing position data external to module.
   std::array<uint8_t, MAX_GPS_MSG_LEN> myRawSerialData{}; //! The array to hold raw data from the serial device.
   std::string myLeftoverChars; //!< The characters from the serial device left over from last processing loop.
   Serial mySerial; //!< The serial device controller.
   const std::chrono::seconds myLookupPeriod{60}; //!< The period to look for new GPS data.
   std::chrono::system_clock::time_point myLastLookupTime; //!< The timestamp of the last GPS data update.
};

#endif
