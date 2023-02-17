#ifndef RPI3_MOTION_CONTROLLER_HPP
#define RPI3_MOTION_CONTROLLER_HPP

#include "interfaces/MotionController/IMotionController.hpp"
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
constexpr double BASE_HORIZ_SERVO_CW_DEADZONE_TEN_US = 144; 
constexpr double BASE_HORIZ_SERVO_CCW_DEADZONE_TEN_US = 154;

constexpr uint8_t FOCUS_SERVO_NUM = 2; // GPIO 18, PIN 12
constexpr uint16_t FOCUS_SERVO_MIN_TEN_US = 100;
constexpr uint16_t FOCUS_SERVO_MAX_TEN_US = 200;
constexpr double FOCUS_SERVO_CW_DEADZONE_TEN_US = 144; 
constexpr double FOCUS_SERVO_CCW_DEADZONE_TEN_US = 154;

/*!
 * The Raspberry Pi 3 Motion Controller is designed to drive the servos of the telescope
 * using GPIO pins found on the board. The controller uses the servoblaster daemon, installed
 * via the build tools to control the speed and position of the servos.
 * The motion controller takes in the target position and target speed at a given time
 * and calculates the settings required to move the connected servos.
 */
class RPi3MotionController : public IMotionController
{
public:
   /*!
    * Creates an RPi3MotionController object by opening the /dev/servoblaster
    * device file for writing.
    */
   RPi3MotionController();

   /*!
    * Destroys an RPi3MotionController object by closing the device file.
    */
   ~RPi3MotionController();

   RPi3MotionController(const RPi3MotionController&) = delete;
   RPi3MotionController(RPi3MotionController&&) = delete;
   void operator=(const RPi3MotionController&) = delete;
   void operator=(RPi3MotionController&&) = delete;

   /*!
    * Move the focus knob servo to the target position or at the target velocity.
    * This servo is continuous and therefore uses the velocity target.
    * \param[in] theta a target position.
    * \param[in] theta_dot a target velocity.
    */
   void moveFocusKnob(const double& theta, const double& theta_dot) override;

   /*!
    * Move the azimuth (horizontal) servo to the target position or at the target velocity.
    * This servo is continuous and therefore uses the velocity target.
    * \param[in] theta a target position.
    * \param[in] theta_dot a target velocity.
    */
   void moveHorizAngle(const double& theta, const double& theta_dot) override;

   /*!
    * Move the elevation (vertical) servo to the target position or at the target velocity.
    * This servo is positional and therefore uses the position target.
    * \param[in] theta a target position.
    * \param[in] theta_dot a target velocity.
    */
   void moveVertAngle(const double& phi, const double& phi_dot) override;

private:
   /*!
    * Create a string in the servoblaster output format using the provided servo number
    * and the number of 10uS pulses. servoblaster format is <servo num>=<num 10us>
    * \param[in] servoNum a number of the servo assigned by servoblaster.
    * \param[in] numTenUs a number of ten uS pulses to set the GPIO output to.
    * \return the formatted string.
    */
   std::string generateServoblasterFormat(const uint8_t& servoNum, const uint16_t& numTenUs);

   std::ofstream myServoControlStream; //!< The servoblaster device file output stream.
   const std::string myServoControlFilePath{"/dev/servoblaster"}; //!< The path to the servoblaster device.
};

#endif