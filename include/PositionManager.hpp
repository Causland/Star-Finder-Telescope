#ifndef POSITION_MANAGER_HPP
#define POSITION_MANAGER_HPP

#include "Subsystem.hpp"
#include "interfaces/IInformationDisplay.hpp"
#include "interfaces/IPositionManager.hpp"

class PositionManager : public IPositionManager, public Subsystem
{
#ifdef UNIT_TEST
    friend class TestFixturePositionManager;
#endif
public:
    PositionManager(std::string subsystemName,  std::shared_ptr<Logger> logger) : Subsystem(subsystemName, logger) {}

    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override;
    void threadLoop() override;

    // Includes from IPositionManager
    void userChangePosition(const CmdUserMove& cmd) override;
    void pointAtTarget(const CmdGoToTarget& cmd) override;
    void calibrate(const CmdCalibrate& cmd) override;

private:
    std::weak_ptr<IInformationDisplay> myInformationDisplay;
};

#endif