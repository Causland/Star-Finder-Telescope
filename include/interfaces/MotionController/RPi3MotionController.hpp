#ifndef RPI3_MOTION_CONTROLLER_HPP
#define RPI3_MOTION_CONTROLLER_HPP

#include "interfaces/MotionController/IMotionController.hpp"

#include <memory>
#include <ostream>

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
    * Creates an RPi3MotionController object with a servo control stream.
    * \param[in] stream an ostream shared pointer.
    */
   explicit RPi3MotionController(std::shared_ptr<std::ostream> stream) : myServoControlStream{std::move(stream)} {};

   /*!
    * Destroys an RPi3MotionController
    */
   ~RPi3MotionController() override = default;

   RPi3MotionController(const RPi3MotionController&) = delete;
   RPi3MotionController(RPi3MotionController&&) = delete;
   void operator=(const RPi3MotionController&) = delete;
   void operator=(RPi3MotionController&&) = delete;

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
   std::string getDisplayInfo() override;

private:
   /*!
    * Create a string in the servoblaster output format using the provided servo number
    * and the number of 10uS pulses. servoblaster format is <servo num>=<num 10us>
    * \param[in] servoNum a number of the servo assigned by servoblaster.
    * \param[in] numTenUs a number of ten uS pulses to set the GPIO output to.
    * \return the formatted string.
    */
   static std::string generateServoblasterFormat(const uint8_t& servoNum, const uint16_t& numTenUs);

   std::shared_ptr<std::ostream> myServoControlStream; //!< The device file output stream.
};

#endif
