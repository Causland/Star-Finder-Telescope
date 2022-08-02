#ifndef RPI3_MOTION_CONTROLLER_HPP
#define RPI3_MOTION_CONTROLLER_HPP

#include "interfaces/IMotionController.hpp"
#include <fstream>
#include <iostream>

constexpr double DEGPERSEC_TO_ROTPERMIN = 60.0 / 360.0;

constexpr uint8_t BASE_VERT_SERVO_NUM = 0; // GPIO 4, Pin 7
constexpr uint16_t BASE_VERT_SERVO_MIN_TEN_US = 86;
constexpr uint16_t BASE_VERT_SERVO_MAX_TEN_US = 212;
constexpr double BASE_VERT_SERVO_MOTION_RANGE_DEG = 180;
constexpr double BASE_VERT_SERVO_TEN_US_PER_DEG = (BASE_VERT_SERVO_MAX_TEN_US - BASE_VERT_SERVO_MIN_TEN_US) 
                                                         / BASE_VERT_SERVO_MOTION_RANGE_DEG;

constexpr uint8_t BASE_HORIZ_SERVO_NUM = 1; // GPIO 17, PIN 11
constexpr uint16_t BASE_HORIZ_SERVO_MIN_TEN_US = 100;
constexpr uint16_t BASE_HORIZ_SERVO_MAX_TEN_US = 200;
constexpr double BASE_HORIZ_SERVO_RPM_PER_TEN_US = 1.0905;
constexpr double BASE_HORIZ_SERVO_RPM_INTERCEPT = -162.34;

constexpr uint8_t FOCUS_SERVO_NUM = 2; // GPIO 18, PIN 12
constexpr uint16_t FOCUS_SERVO_MIN_TEN_US = 100;
constexpr uint16_t FOCUS_SERVO_MAX_TEN_US = 200;
constexpr double FOCUS_SERVO_RPM_PER_TEN_US = 1.0905;
constexpr double FOCUS_SERVO_RPM_INTERCEPT = -162.34;
class RPi3MotionController : public IMotionController
{
public:
   RPi3MotionController();
   ~RPi3MotionController();
   RPi3MotionController(const RPi3MotionController&) = delete;
   RPi3MotionController(RPi3MotionController&&) = delete;
   void operator=(const RPi3MotionController&) = delete;
   void operator=(RPi3MotionController&&) = delete;

   // Includes from IMotionController
   void moveFocusKnob(const double& theta, const double& theta_dot) override;
   void moveHorizAngle(const double& theta, const double& theta_dot) override;
   void moveVertAngle(const double& phi, const double& phi_dot) override;

private:
   std::string generateServoblasterFormat(const uint8_t& servoNum, const uint16_t& numTenUs);

   std::ofstream myServoControlStream;
   const std::string myServoControlFilePath{"/dev/servoblaster"};
};

#endif