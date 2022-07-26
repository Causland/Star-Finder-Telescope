#ifndef POSITION_MANAGER_HPP
#define POSITION_MANAGER_HPP

#include "Subsystem.hpp"
#include "interfaces/IInformationDisplay.hpp"
#include "interfaces/IMotionController.hpp"
#include "interfaces/IPositionManager.hpp"

class PositionManager : public IPositionManager, public Subsystem
{
public:
    PositionManager(std::string subsystemName,  std::shared_ptr<Logger> logger, std::shared_ptr<IMotionController> motionController) : 
                        Subsystem(subsystemName, logger), myMotionController(motionController) {}

    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override;
    void threadLoop() override;

    // Includes from IPositionManager
    void updatePosition(const CmdUpdatePosition& cmd) override;
    void trackTarget(const PositionTable& positions) override;
    void calibrate(const CmdCalibrate& cmd) override;

private:
    double myTargetAzimuth{0.0};
    double myTargetElevation{0.0};
    PositionTable myPositionTable;
    bool myTargetUpdateFlag{false};
    bool myInTrackingMode{false};
    std::weak_ptr<IMotionController> myMotionController;
    std::weak_ptr<IInformationDisplay> myInformationDisplay;

};

#endif