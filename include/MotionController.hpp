#ifndef MOTION_CONTROLLER_HPP
#define MOTION_CONTROLLER_HPP

#include "interfaces/IMotionController.hpp"
#include "Subsystem.hpp"

class MotionController : public IMotionController, public Subsystem
{
public:
    MotionController(std::string subsystemName,  std::shared_ptr<Logger> logger) : Subsystem(subsystemName, logger) {}

    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override;
    void threadLoop() override;

    // Includes from IMotionController
    void moveFocusKnob(double theta) override;
    void moveHorizAngle(double theta) override;
    void moveVertAngle(double phi) override;
};

#endif