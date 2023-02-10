#ifndef SERIAL_HPP
#define SERIAL_HPP

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
    * Read characters from the serial port up to MAX_SERIAL_READ or end of available.
    * \sa MAX_SERIAL_READ
    * \param[out] strOut a string to store the output.
    * \return true if successful read.
    */
   bool readFromSerial(std::string* strOut);

   /*!
    * Write a string to the serial port.
    * \param[in] msg a message string to write.
    * \return true if successful write.
    */
   bool writeToSerial(const std::string& msg);

private:
   int fd{-1}; //!< File descriptor of the port.
   struct termios tty; //!< Termios struct to hold configuration for the port.
};

#endif