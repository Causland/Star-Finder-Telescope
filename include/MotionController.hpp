#ifndef MOTION_CONTROLLER_HPP
#define MOTION_CONTROLLER_HPP

#include "interfaces/IMotionController.hpp"
#include "Subsystem.hpp"

class MotionController : public IMotionController, public Subsystem
{
    // Things for the motion controller to do
    // Move servo by a certain degree on command
    // Differentiate between camera and telescope position servo
    // Keep track of movement progress/completion

public:
    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces() override;
    bool checkHeartbeat() override;
    void threadLoop() override;

    // Includes from IMotionController
    void moveFocusKnob(double theta) override;
    void moveHorizAngle(double theta) override;
    void moveVertAngle(double phi) override;
};

#endif