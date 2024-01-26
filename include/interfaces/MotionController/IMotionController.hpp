#ifndef I_MOTION_CONTROLLER_HPP
#define I_MOTION_CONTROLLER_HPP

#include <string>

/*!
 * Stores information about a specific rotation's position
 * and velocity.
 */
struct Rotation
{
   double theta{0.0};
   double theta_dot{0.0};
};

/*!
 * A Motion Controller is responsible for moving connected motors to
 * a set position or at a set speed. The controller can choose to use either
 * piece of info. For example, absolute servos would use position but 
 * continuous servos would use the speed.
 */
class IMotionController
{
public:
   /*!
    * Creates an IMotionController.
    */
   IMotionController() = default;

   /*!
    * Destroys an IMotionController.
    */ 
   virtual ~IMotionController() = default;
   
   IMotionController(const IMotionController&) = delete;
   IMotionController& operator=(const IMotionController&) = delete;
   IMotionController(IMotionController&&) = delete;
   IMotionController& operator=(IMotionController&&) = delete;

   /*!
    * Move the focus motor to the specific theta or at the theta_dot speed.
    * \param[in] rot the rotation information to perform.
    */ 
   virtual void moveFocusKnob(const Rotation& rot) = 0;

   /*!
    * Move the horizontal angle motor to the specific theta or at the theta_dot speed.
    * \param[in] rot the rotation information to perform.
    */ 
   virtual void moveHorizAngle(const Rotation& rot) = 0;

   /*!
    * Move the vertical angle motor to the specific theta or at the theta_dot speed.
    * \param[in] rot the rotation information to perform.
    */ 
   virtual void moveVertAngle(const Rotation& rot) = 0;

   /*!
    * Get information about the Motion Controller state in string format.
    * \return a formatted string.
    */ 
   virtual std::string getDisplayInfo() = 0;
};

#endif
