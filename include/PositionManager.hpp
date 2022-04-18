#ifndef POSITION_MANAGER_HPP
#define POSITION_MANAGER_HPP

#include "Subsystem.hpp"
#include "interfaces/IInformationDisplay.hpp"
#include "interfaces/IPositionManager.hpp"

class PositionManager : public IPositionManager, public Subsystem
{
public:
    PositionManager(std::string subsystemName,  std::shared_ptr<Logger> logger) : Subsystem(subsystemName, logger) {}

    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override;
    void threadLoop() override;

    // Includes from IPositionManager
    void userChangePosition(double theta, double phi) override;
    void pointAtTarget(StarPosition position) override;

private:
    std::weak_ptr<IInformationDisplay> myInformationDisplay;
};

#endif