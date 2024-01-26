#include "interfaces/MotionController/AVRMotionController.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <string>

static constexpr double DEGPERSEC_TO_ROTPERMIN{60.0 / 360.0};

static constexpr uint8_t BASE_VERT_SERVO_NUM{0};
static constexpr uint8_t BASE_VERT_SERVO_MIN_TEN_US{86};
static constexpr uint8_t BASE_VERT_SERVO_MAX_TEN_US{212};
static constexpr double BASE_VERT_SERVO_MOTION_RANGE_DEG{180};
static constexpr double BASE_VERT_SERVO_TEN_US_PER_DEG{(BASE_VERT_SERVO_MAX_TEN_US - BASE_VERT_SERVO_MIN_TEN_US) 
                                                         / BASE_VERT_SERVO_MOTION_RANGE_DEG};

static constexpr uint8_t BASE_HORIZ_SERVO_NUM{1};
static constexpr uint8_t BASE_HORIZ_SERVO_MIN_TEN_US{100};
static constexpr uint8_t BASE_HORIZ_SERVO_MAX_TEN_US{200};
static constexpr double BASE_HORIZ_SERVO_AVG_TEN_US{(BASE_HORIZ_SERVO_MAX_TEN_US - BASE_HORIZ_SERVO_MIN_TEN_US) / 2.0 + BASE_HORIZ_SERVO_MIN_TEN_US};
static constexpr double BASE_HORIZ_SERVO_CW_DEADZONE_TEN_US{144}; 
static constexpr double BASE_HORIZ_SERVO_CCW_DEADZONE_TEN_US{154};

static constexpr uint8_t FOCUS_SERVO_NUM{2};
static constexpr uint8_t FOCUS_SERVO_MIN_TEN_US{100};
static constexpr uint8_t FOCUS_SERVO_MAX_TEN_US{200};
static constexpr double FOCUS_SERVO_AVG_TEN_US{(FOCUS_SERVO_MAX_TEN_US - FOCUS_SERVO_MIN_TEN_US) / 2.0 + FOCUS_SERVO_MIN_TEN_US};
static constexpr double FOCUS_SERVO_CW_DEADZONE_TEN_US{144}; 
static constexpr double FOCUS_SERVO_CCW_DEADZONE_TEN_US{154};

void AVRMotionController::moveFocusKnob(const Rotation& rot)
{
   // Convert theta_dot to rpm
   const auto rpm{rot.theta_dot * DEGPERSEC_TO_ROTPERMIN};
   double numTenUs{FOCUS_SERVO_AVG_TEN_US}; 
   
   if (rpm > 0.0)
   {
      // CW rotation
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers) : Precalculated coefficients for servo
      numTenUs = (-0.0082*rpm*rpm) - (0.4462*rpm) + 144.17;

      // Adjust to keep within min and max
      numTenUs = std::clamp(numTenUs, 
                            static_cast<double>(FOCUS_SERVO_MIN_TEN_US), 
                            static_cast<double>(FOCUS_SERVO_CW_DEADZONE_TEN_US - 1));
   }
   else if (rpm < 0.0)
   {
      // CCW rotation
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers) : Precalculated coefficients for servo
      numTenUs = (0.0084*rpm*rpm) - (0.4103*rpm) + 154.06;

      // Adjust to keep within min and max
      numTenUs = std::clamp(numTenUs, 
                            static_cast<double>(FOCUS_SERVO_CCW_DEADZONE_TEN_US + 1), 
                            static_cast<double>(FOCUS_SERVO_MAX_TEN_US));
   }
   
   // Write to serial device to move focus servo
   const std::array<uint8_t, 2> command{FOCUS_SERVO_NUM, static_cast<uint8_t>(numTenUs)};
   mySerial.writeToSerial(command.data(), command.size());
}

void AVRMotionController::moveHorizAngle(const Rotation& rot)
{
   // Convert theta_dot to rpm
   const auto rpm{rot.theta_dot * DEGPERSEC_TO_ROTPERMIN};
   double numTenUs{BASE_HORIZ_SERVO_AVG_TEN_US}; 
   
   if (rpm > 0.0)
   {
      // CW rotation
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers) : Precalculated coefficients for servo
      numTenUs = (-0.0082*rpm*rpm) - (0.4462*rpm) + 144.17;

      // Adjust to keep within min and max
      numTenUs = std::clamp(numTenUs, 
                            static_cast<double>(BASE_HORIZ_SERVO_MIN_TEN_US), 
                            static_cast<double>(BASE_HORIZ_SERVO_CW_DEADZONE_TEN_US - 1));
   }
   else if (rpm < 0.0)
   {
      // CCW rotation
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers) : Precalculated coefficients for servo
      numTenUs = (0.0084*rpm*rpm) - (0.4103*rpm) + 154.06; 

      // Adjust to keep within min and max
      numTenUs = std::clamp(numTenUs, 
                            static_cast<double>(BASE_HORIZ_SERVO_CCW_DEADZONE_TEN_US + 1), 
                            static_cast<double>(BASE_HORIZ_SERVO_MAX_TEN_US));
   }

   // Write to serial device to move base servo
   const std::array<uint8_t, 2> command{BASE_HORIZ_SERVO_NUM, static_cast<uint8_t>(numTenUs)};
   mySerial.writeToSerial(command.data(), command.size());
}

void AVRMotionController::moveVertAngle(const Rotation& rot)
{
   const auto numTenUs{std::clamp(rot.theta * BASE_VERT_SERVO_TEN_US_PER_DEG + BASE_VERT_SERVO_MIN_TEN_US,
                                    static_cast<double>(BASE_VERT_SERVO_MIN_TEN_US),
                                    static_cast<double>(BASE_VERT_SERVO_MAX_TEN_US))};

   // Write to serial device to move base servo
   const std::array<uint8_t, 2> command{BASE_VERT_SERVO_NUM, static_cast<uint8_t>(numTenUs)};
   mySerial.writeToSerial(command.data(), command.size());
}

std::string AVRMotionController::getDisplayInfo()
{
   return "";
}
