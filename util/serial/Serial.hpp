#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <cstdint>
#include <fcntl.h>
#include <string>
#include <termios.h>
#include <unistd.h>

/*!
 * This utility supports writing and reading to and from a designated serial port.
 * A majority of the code for the configuration of the serial port was adapted or
 * take directly from the following blog written by Geoffrey Hunter:
 * https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
 */
class Serial
{
public:
   /*!
    * Creates an opens a serial port with the provided path, control mode, baud rate, and timeout.
    * \param[in] serialDevice a path to a serial device file.
    * \param[in] fcntlMode a file control mode setting.
    * \param[in] baudRate a baud rate constant defined by the termios structure.
    * \param[in] timeoutds a timeout period for reading the serial port in deciseconds.
    */
   Serial(const std::string& serialDevice, const int& fcntlMode, const uint32_t& baudRate, const uint8_t& timeoutds);

   /*!
    * Destroy a serial object by closing serial port file descriptor.
    */
   ~Serial();

   Serial(const Serial&) = delete;
   Serial(Serial&&) = delete;
   void operator=(const Serial&) = delete;
   void operator=(Serial&&) = delete;

   /*!
    * Read data from the serial port up to len or end of available.
    * Note that this function assumes data can fit all the data requested.
    * \param[out] data a byte array to store the data.
    * \param[in] len a number of bytes to read.
    * \return number of bytes read.
    */
   ssize_t readFromSerial(uint8_t* data, const size_t& len);

   /*!
    * Write data to the serial port. Note that this function assumes
    * data contains all data to send up to len.
    * \param[in] data a byte array to write.
    * \param[in] len a number of bytes to write.
    * \return number of bytes written.
    */
   ssize_t writeToSerial(uint8_t const* data, const size_t& len);

private:
   int fd{-1}; //!< File descriptor of the port.
   struct termios tty; //!< Termios struct to hold configuration for the port.
};

#endif
