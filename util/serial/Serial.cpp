#include "serial/Serial.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>

constexpr uint32_t MAX_SERIAL_READ{1024};

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

   // Set control flags
   tty.c_cflag &= ~PARENB; // Clear parity bit
   tty.c_cflag &= ~CSTOPB; // Clear stop field
   tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
   tty.c_cflag |= CS8; // 8 bits per byte
   tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control
   tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines

   // Set local modes
   tty.c_lflag &= ~ICANON; // Disable canonical mode
   tty.c_lflag &= ~ECHO; // Disable echo
   tty.c_lflag &= ~ECHOE; // Disable erasure
   tty.c_lflag &= ~ECHONL; // Disable new-line echo
   tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
   tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
   tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

   // Set output modes
   tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
   tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

   // Set timeout based on parameter
   tty.c_cc[VTIME] = timeoutds;
   tty.c_cc[VMIN] = 0;

   // Set the baud rate based on parameter
   cfsetispeed(&tty, baudRate);
   cfsetospeed(&tty, baudRate);

   // Save tty settings
   if (tcsetattr(fd, TCSANOW, &tty) != 0)
   {
      throw std::runtime_error("Error " + std::to_string(errno) + " from tcsetattr: " + std::strerror(errno));
   }
}

Serial::~Serial()
{
   close(fd);
}

bool Serial::readFromSerial(std::string* strOut)
{
   char readBuf[MAX_SERIAL_READ];
   // Read the buffer less 1 byte for a null terminator
   auto bytes = read(fd, &readBuf, sizeof(readBuf) - 1);
   if (bytes != -1)
   {
      // Make sure the string terminates
      readBuf[bytes] = '\0';
      *strOut = std::string(readBuf);
      return true;
   }
   return false;
}

bool Serial::writeToSerial(const std::string& msg)
{
   // Write string to serial
   return write(fd, msg.c_str(), msg.size()) != -1;
}