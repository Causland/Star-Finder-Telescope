#include "interfaces/MotionController/AVRMotionController.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <string>

constexpr uint8_t SERIAL_TIMEOUT{50};

AVRMotionController::AVRMotionController(const std::string& serialDevice) :
   mySerial(serialDevice, O_WRONLY, B9600, SERIAL_TIMEOUT)
{
}

void AVRMotionController::moveFocusKnob(const double& theta, const double& theta_dot)
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

   // Write to serial device to move focus servo
   std::array<uint8_t, 2> command{FOCUS_SERVO_NUM, static_cast<uint8_t>(numTenUs)};
   mySerial.writeToSerial(command.data(), command.size());
}

void AVRMotionController::moveHorizAngle(const double& theta, const double& theta_dot)
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

   // Write to serial device to move base servo
   std::array<uint8_t, 2> command{BASE_HORIZ_SERVO_NUM, static_cast<uint8_t>(numTenUs)};
   mySerial.writeToSerial(command.data(), command.size());
}

void AVRMotionController::moveVertAngle(const double& phi, const double& phi_dot)
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

   // Write to serial device to move base servo
   std::array<uint8_t, 2> command{BASE_VERT_SERVO_NUM, static_cast<uint8_t>(numTenUs)};
   mySerial.writeToSerial(command.data(), command.size());
}

std::string AVRMotionController::getDisplayInfo()
{
   return "";
}
