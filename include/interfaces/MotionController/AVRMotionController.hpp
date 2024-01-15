#ifndef AVR_MOTION_CONTROLLER_HPP
#define AVR_MOTION_CONTROLLER_HPP

#include "interfaces/MotionController/IMotionController.hpp"
#include "serial/Serial.hpp"

constexpr double DEGPERSEC_TO_ROTPERMIN = 60.0 / 360.0;

constexpr uint8_t BASE_VERT_SERVO_NUM = 0;
constexpr uint8_t BASE_VERT_SERVO_MIN_TEN_US = 86;
constexpr uint8_t BASE_VERT_SERVO_MAX_TEN_US = 212;
constexpr double BASE_VERT_SERVO_MOTION_RANGE_DEG = 180;
constexpr double BASE_VERT_SERVO_TEN_US_PER_DEG = (BASE_VERT_SERVO_MAX_TEN_US - BASE_VERT_SERVO_MIN_TEN_US) 
                                                         / BASE_VERT_SERVO_MOTION_RANGE_DEG;

constexpr uint8_t BASE_HORIZ_SERVO_NUM = 1;
constexpr uint8_t BASE_HORIZ_SERVO_MIN_TEN_US = 100;
constexpr uint8_t BASE_HORIZ_SERVO_MAX_TEN_US = 200;
constexpr double BASE_HORIZ_SERVO_CW_DEADZONE_TEN_US = 144; 
constexpr double BASE_HORIZ_SERVO_CCW_DEADZONE_TEN_US = 154;

constexpr uint8_t FOCUS_SERVO_NUM = 2;
constexpr uint8_t FOCUS_SERVO_MIN_TEN_US = 100;
constexpr uint8_t FOCUS_SERVO_MAX_TEN_US = 200;
constexpr double FOCUS_SERVO_CW_DEADZONE_TEN_US = 144; 
constexpr double FOCUS_SERVO_CCW_DEADZONE_TEN_US = 154;

/*!
 * The AVR Motion Controller is designed to drive the servos of the telescope
 * using PWM pins found on any AVR board (Arduino Uno/Nano etc). The controller uses 
 * a serial port to send position commands to the board.
 * The motion controller takes in the target position and target speed at a given time
 * and calculates the uSec required.
 */
class AVRMotionController : public IMotionController
{
public:
   /*!
    * Creates an AVRMotionController object by opening the provided
    * serial device for writing.
    * \param[in] serialDevice a path to a valid serial device to send commands.
    */
   explicit AVRMotionController(const std::string& serialDevice);

   /*!
    * Destroys an AVRMotionController.
    */
   virtual ~AVRMotionController() = default;

   AVRMotionController(const AVRMotionController&) = delete;
   AVRMotionController(AVRMotionController&&) = delete;
   void operator=(const AVRMotionController&) = delete;
   void operator=(AVRMotionController&&) = delete;

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

   /*!
    * Get information to display on the screen about the motion controller.
    * \return a formatted string of information to display.
    */
   std::string getDisplayInfo() override;

private:   
   Serial mySerial; //!< The serial device controller.
};

#endif
