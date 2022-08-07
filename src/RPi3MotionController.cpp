#include "RPi3MotionController.hpp"
#include <algorithm>
#include <exception>
#include <string>
#include <wiringPi.h>

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

void RPi3MotionController::moveFocusKnob(const double& theta, const double& theta_dot)
{
   // Convert theta_dot to rpm
   auto theta_dot_rpm = theta_dot * DEGPERSEC_TO_ROTPERMIN;
   auto numTenUs = (theta_dot_rpm - FOCUS_SERVO_RPM_INTERCEPT) / FOCUS_SERVO_RPM_PER_TEN_US;
   if (numTenUs > FOCUS_SERVO_MAX_TEN_US)
   {
      numTenUs = FOCUS_SERVO_MAX_TEN_US;
   }
   else if (numTenUs < FOCUS_SERVO_MIN_TEN_US)
   {
      numTenUs = FOCUS_SERVO_MIN_TEN_US;
   }

   // Write to servoblaster dev file to move focus servo
   myServoControlStream << generateServoblasterFormat(FOCUS_SERVO_NUM, static_cast<uint16_t>(numTenUs));
}

void RPi3MotionController::moveHorizAngle(const double& theta, const double& theta_dot)
{
   // Convert theta_dot to rpm
   auto theta_dot_rpm = theta_dot * DEGPERSEC_TO_ROTPERMIN;
   auto numTenUs = (theta_dot_rpm - BASE_HORIZ_SERVO_RPM_INTERCEPT) / BASE_HORIZ_SERVO_RPM_PER_TEN_US;
   if (numTenUs > BASE_HORIZ_SERVO_MAX_TEN_US)
   {
      numTenUs = BASE_HORIZ_SERVO_MAX_TEN_US;
   }
   else if (numTenUs < BASE_HORIZ_SERVO_MIN_TEN_US)
   {
      numTenUs = BASE_HORIZ_SERVO_MIN_TEN_US;
   }

   // Write to servoblaster dev file to move base servo
   myServoControlStream << generateServoblasterFormat(BASE_HORIZ_SERVO_NUM, static_cast<uint16_t>(numTenUs));
}

void RPi3MotionController::moveVertAngle(const double& phi, const double& phi_dot)
{
   uint16_t position = static_cast<uint16_t>(phi * BASE_VERT_SERVO_TEN_US_PER_DEG + BASE_VERT_SERVO_MIN_TEN_US);
   if (position > BASE_VERT_SERVO_MAX_TEN_US)
   {
      position = BASE_VERT_SERVO_MAX_TEN_US;
   }

   // Write to servoblaster dev file to move base servo
   myServoControlStream << generateServoblasterFormat(BASE_VERT_SERVO_NUM, position);
   myServoControlStream.flush();
}

std::string RPi3MotionController::generateServoblasterFormat(const uint8_t& servoNum, const uint16_t& numTenUs)
{
   return std::to_string(servoNum) + "=" + std::to_string(numTenUs) + "\n";
}