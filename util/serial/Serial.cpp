#include "serial/Serial.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <utility>

Serial::Serial(const std::string& serialDevice, const int& fcntlMode, const uint32_t& baudRate, const uint8_t& timeoutds)
{
   // Open the port using the serial device and mode
   fd = open(serialDevice.c_str(), fcntlMode);
   if (fd == -1)
   {
      throw std::runtime_error("Unable to open serial device: " + serialDevice + "\n");
   }

   // Read in existing settings
   if (tcgetattr(fd, &tty) != 0)
   {
      throw std::runtime_error("Error " + std::to_string(errno) + " from tcgetattr: " + std::strerror(errno));
   }

   // Set interface to raw
   cfmakeraw(&tty);

   // Set the baud rate based on parameter
   cfsetspeed(&tty, baudRate);

   // Set control modes outside cfmakeraw
   tty.c_cflag &= ~CSTOPB; // Clear stop field
   tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control
   tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines

   // Set local modes outside cfmakeraw
   tty.c_lflag &= ~ECHOE; // Disable erasure
   
   // Set input modes outside cfmakeraw
   tty.c_iflag &= ~(IXOFF | IXANY); // Turn off s/w flow ctrl

   // Set output modes outside cfmakeraw
   tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

   // Set timeout based on parameter
   tty.c_cc[VTIME] = timeoutds;
   tty.c_cc[VMIN] = 0;

   // Flush port and save tty settings
   tcflush(fd, TCIFLUSH);
   if (tcsetattr(fd, TCSANOW, &tty) != 0)
   {
      throw std::runtime_error("Error " + std::to_string(errno) + " from tcsetattr: " + std::strerror(errno));
   }
}

Serial::~Serial()
{
   close(fd);
}

Serial::Serial(Serial&& dev) noexcept : fd{std::exchange(dev.fd, -1)}, tty{std::exchange(tty, {})} {}

ssize_t Serial::readFromSerial(uint8_t* data, const size_t& len)
{
   return read(fd, data, len);
}

ssize_t Serial::writeToSerial(uint8_t const* data, const size_t& len)
{
   return write(fd, data, len);
}
