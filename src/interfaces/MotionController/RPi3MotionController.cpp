#include "interfaces/MotionController/RPi3MotionController.hpp"
#include <algorithm>
#include <exception>
#include <string>

RPi3MotionController::RPi3MotionController()
{
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
   auto rpm{theta_dot * DEGPERSEC_TO_ROTPERMIN};
   double numTenUs{(FOCUS_SERVO_MAX_TEN_US - FOCUS_SERVO_MIN_TEN_US) / 2.0 + FOCUS_SERVO_MIN_TEN_US}; 
   
   if (rpm > 0.0)
   {
      // CW rotation
      numTenUs = (-0.0082*rpm*rpm) - (0.4462*rpm) + 144.17;

      // Adjust to keep within min and max
      if (numTenUs >= FOCUS_SERVO_CW_DEADZONE_TEN_US)
      {
         numTenUs = FOCUS_SERVO_CW_DEADZONE_TEN_US - 1;
      }
      else if (numTenUs < FOCUS_SERVO_MIN_TEN_US)
      {
         numTenUs = FOCUS_SERVO_MIN_TEN_US;
      }
   }
   else if (rpm < 0.0)
   {
      // CCW rotation
      numTenUs = (0.0084*rpm*rpm) - (0.4103*rpm) + 154.06;

      // Adjust to keep within min and max
      if (numTenUs <= FOCUS_SERVO_CCW_DEADZONE_TEN_US)
      {
         numTenUs = FOCUS_SERVO_CCW_DEADZONE_TEN_US + 1;
      }
      else if (numTenUs > FOCUS_SERVO_MAX_TEN_US)
      {
         numTenUs = FOCUS_SERVO_MAX_TEN_US;
      }
   }

   // Write to servoblaster dev file to move focus servo
   myServoControlStream << generateServoblasterFormat(FOCUS_SERVO_NUM, static_cast<uint16_t>(numTenUs));
}

void RPi3MotionController::moveHorizAngle(const double& theta, const double& theta_dot)
{
   // Convert theta_dot to rpm
   auto rpm{theta_dot * DEGPERSEC_TO_ROTPERMIN};
   double numTenUs{(BASE_HORIZ_SERVO_MAX_TEN_US - BASE_HORIZ_SERVO_MIN_TEN_US) / 2.0 + BASE_HORIZ_SERVO_MIN_TEN_US}; 
   
   if (rpm > 0.0)
   {
      // CW rotation
      numTenUs = (-0.0082*rpm*rpm) - (0.4462*rpm) + 144.17;

      // Adjust to keep within min and max
      if (numTenUs >= BASE_HORIZ_SERVO_CW_DEADZONE_TEN_US)
      {
         numTenUs = BASE_HORIZ_SERVO_CW_DEADZONE_TEN_US - 1;
      }
      else if (numTenUs < BASE_HORIZ_SERVO_MIN_TEN_US)
      {
         numTenUs = BASE_HORIZ_SERVO_MIN_TEN_US;
      }
   }
   else if (rpm < 0.0)
   {
      // CCW rotation
      numTenUs = (0.0084*rpm*rpm) - (0.4103*rpm) + 154.06;

      // Adjust to keep within min and max
      if (numTenUs <= BASE_HORIZ_SERVO_CCW_DEADZONE_TEN_US)
      {
         numTenUs = BASE_HORIZ_SERVO_CCW_DEADZONE_TEN_US + 1;
      }
      else if (numTenUs > BASE_HORIZ_SERVO_MAX_TEN_US)
      {
         numTenUs = BASE_HORIZ_SERVO_MAX_TEN_US;
      }
   }

   // Write to servoblaster dev file to move base servo
   myServoControlStream << generateServoblasterFormat(BASE_HORIZ_SERVO_NUM, static_cast<uint16_t>(numTenUs));
}

void RPi3MotionController::moveVertAngle(const double& phi, const double& phi_dot)
{
   double numTenUs{phi * BASE_VERT_SERVO_TEN_US_PER_DEG + BASE_VERT_SERVO_MIN_TEN_US};
   if (numTenUs > BASE_VERT_SERVO_MAX_TEN_US)
   {
      numTenUs = BASE_VERT_SERVO_MAX_TEN_US;
   }
   else if (numTenUs < BASE_VERT_SERVO_MIN_TEN_US)
   {
      numTenUs = BASE_VERT_SERVO_MIN_TEN_US;
   }

   // Write to servoblaster dev file to move base servo
   myServoControlStream << generateServoblasterFormat(BASE_VERT_SERVO_NUM, static_cast<uint16_t>(numTenUs));
   myServoControlStream.flush();
}

std::string RPi3MotionController::getDisplayInfo()
{
   return "";
}

std::string RPi3MotionController::generateServoblasterFormat(const uint8_t& servoNum, const uint16_t& numTenUs)
{
   return std::to_string(servoNum) + "=" + std::to_string(numTenUs) + "\n";
}