#include "RPi3MotionController.hpp"
#include <algorithm>
#include <exception>
#include <string>
#include <wiringPi.h>

constexpr uint8_t FOCUS_SERVO_PIN = 0;
constexpr uint16_t FOCUS_SERVO_TEN_US_PER_DEG = 500;

constexpr uint8_t BASE_HORIZ_SERVO_PIN = 1; 
constexpr uint16_t BASE_HORIZ_SERVO_TEN_US_PER_DEG = 500;

constexpr uint8_t BASE_VERT_SERVO_PIN = 0; 
constexpr uint16_t BASE_VERT_SERVO_TEN_US_PER_DEG = 500;

const std::string IMotionController::NAME{"RPi3MotionController"};

RPi3MotionController::RPi3MotionController()
{
   wiringPiSetup();
   myServoControlStream.open(myServoControlFilePath);
   if (!myServoControlStream.is_open())
   {
      throw std::runtime_error("Could not open servo controller file: " + myServoControlFilePath);
   }
}

RPi3MotionController::~RPi3MotionController()
{
   myServoControlStream.close();
}

void RPi3MotionController::moveFocusKnob(double theta)
{
   // Write to servoblaster dev file to move focus servo
   myServoControlStream << generateServoblasterFormat(FOCUS_SERVO_PIN, static_cast<uint16_t>(theta * FOCUS_SERVO_TEN_US_PER_DEG));
}

void RPi3MotionController::moveHorizAngle(double theta)
{
   // Write to servoblaster dev file to move base servo
   myServoControlStream << generateServoblasterFormat(BASE_HORIZ_SERVO_PIN, static_cast<uint16_t>(theta * BASE_HORIZ_SERVO_TEN_US_PER_DEG));
}

void RPi3MotionController::moveVertAngle(double phi)
{
   // Write to servoblaster dev file to move base servo
   myServoControlStream << generateServoblasterFormat(FOCUS_SERVO_PIN, static_cast<uint16_t>(phi * BASE_VERT_SERVO_TEN_US_PER_DEG));
}

std::string RPi3MotionController::generateServoblasterFormat(const uint8_t& pinNum, const uint16_t& posInUs)
{
   return std::to_string(pinNum) + "=" + std::to_string(posInUs);
}