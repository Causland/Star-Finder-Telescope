#ifndef RPI3_MOTION_CONTROLLER_HPP
#define RPI3_MOTION_CONTROLLER_HPP

#include "interfaces/IMotionController.hpp"

class RPi3MotionController : public IMotionController
{
public:
    RPi3MotionController() {}

    // Includes from IMotionController
    void moveFocusKnob(double theta) override;
    void moveHorizAngle(double theta) override;
    void moveVertAngle(double phi) override;
};

#endif