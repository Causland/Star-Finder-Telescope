#ifndef I_MOTION_CONTROLLER_HPP
#define I_MOTION_CONTROLLER_HPP

#include "interfaces/ISubsystem.hpp"

class IMotionController : public ISubsystem
{
public:
    void moveFocusKnob(double theta);
    void moveHorizAngle(double theta);
    void moveVertAngle(double phi);
};

#endif