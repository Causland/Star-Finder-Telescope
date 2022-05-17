#ifndef INFORMATION_DISPLAY_HPP
#define INFORMATION_DISPLAY_HPP

#include "Subsystem.hpp"
#include "interfaces/ICommandTerminal.hpp"
#include "interfaces/IInformationDisplay.hpp"
#include "interfaces/IOpticsManager.hpp"
#include "interfaces/IPositionManager.hpp"
#include "interfaces/IStarTracker.hpp"

class InformationDisplay : public IInformationDisplay, public Subsystem
{
public:
    InformationDisplay(std::string subsystemName,  std::shared_ptr<Logger> logger) : Subsystem(subsystemName, logger) {}

    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override;
    void threadLoop() override;

private:
    std::weak_ptr<ICommandTerminal> myCommandTerminal;
    std::weak_ptr<IOpticsManager> myOpticsManager;
    std::weak_ptr<IPositionManager> myPositionManager;
    std::weak_ptr<IStarTracker> myStarTracker;
};

#endif