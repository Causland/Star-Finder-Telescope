#ifndef SIM_MOTION_CONTROLLER_HPP
#define SIM_MOTION_CONTROLLER_HPP

#include "interfaces/MotionController/IMotionController.hpp"

/*!
 * A simulated Motion Controller which only keeps track of the
 * position and speed it is set to.
 */ 
class SimMotionController : public IMotionController
{
public:
   /*!
    * Creates a SimMotionController.
    */
   SimMotionController() = default;

   /*!
    * Destroys a SimMotionController.
    */ 
   ~SimMotionController() override = default;

   SimMotionController(const SimMotionController&) = delete;
   SimMotionController& operator=(const SimMotionController&) = delete;
   SimMotionController(SimMotionController&&) = delete;
   SimMotionController& operator=(SimMotionController&&) = delete;

   /*!
    * Sets the stored values to the specific rotation. 
    * \param[in] rot the rotation information to perform.
    */ 
   void moveFocusKnob(const Rotation& rot) override;

   /*!
    * Sets the stored values to the specific rotation.
    * \param[in] rot the rotation information to perform.
    */ 
   void moveHorizAngle(const Rotation& rot) override;

   /*!
    * Sets the stored values to the specific rotation.
    * \param[in] rot the rotation information to perform.
    */ 
   void moveVertAngle(const Rotation& rot) override;


   /*!
    * Get a formatted string of the stored values
    */ 
   [[nodiscard]] std::string getDisplayInfo() override;

private:
   Rotation myHorizRot;
   Rotation myVertRot;
   Rotation myFocusRot;
};

#endif
