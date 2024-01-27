#ifndef AVR_MOTION_CONTROLLER_HPP
#define AVR_MOTION_CONTROLLER_HPP

#include "interfaces/MotionController/IMotionController.hpp"
#include "serial/Serial.hpp"

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
    * \param[in] serialDevice a serial device moved into controller.
    */
   explicit AVRMotionController(Serial&& serialDevice) : mySerial{std::move(serialDevice)} {}

   /*!
    * Destroys an AVRMotionController.
    */
   ~AVRMotionController() override = default;

   AVRMotionController(const AVRMotionController&) = delete;
   AVRMotionController(AVRMotionController&&) = delete;
   void operator=(const AVRMotionController&) = delete;
   void operator=(AVRMotionController&&) = delete;

   /*!
    * Move the focus knob servo to the target position or at the target velocity.
    * This servo is continuous and therefore uses the velocity target.
    * \param[in] rot the rotation information to perform.
    */
   void moveFocusKnob(const Rotation& rot) override;

   /*!
    * Move the azimuth (horizontal) servo to the target position or at the target velocity.
    * This servo is continuous and therefore uses the velocity target.
    * \param[in] rot the rotation information to perform.
    */
   void moveHorizAngle(const Rotation& rot) override;

   /*!
    * Move the elevation (vertical) servo to the target position or at the target velocity.
    * This servo is positional and therefore uses the position target.
    * \param[in] rot the rotation information to perform.
    */
   void moveVertAngle(const Rotation& rot) override;

   /*!
    * Get information to display on the screen about the motion controller.
    * \return a formatted string of information to display.
    */
   [[nodiscard]] std::string getDisplayInfo() override;

private:   
   Serial mySerial; //!< The serial device controller.
};

#endif
