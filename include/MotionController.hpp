#ifndef MOTION_CONTROLLER_HPP
#define MOTION_CONTROLLER_HPP

#include "interfaces/IMotionController.hpp"

class MotionController : public IMotionController
{
    // Things for the motion controller to do
    // Move servo by a certain degree on command
    // Differentiate between camera and telescope position servo
    // Keep track of movement progress/completion
    void moveFocusKnob(double theta);
    void moveHorizAngle(double theta);
    void moveVertAngle(double phi);
};

#endif