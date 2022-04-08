#ifndef I_MOTION_CONTROLLER_HPP
#define I_MOTION_CONTROLLER_HPP

class IMotionController
{
public:
    virtual void moveFocusKnob(double theta) = 0;
    virtual void moveHorizAngle(double theta) = 0;
    virtual void moveVertAngle(double phi) = 0;
};

#endif