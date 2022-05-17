#ifndef OPTICS_MANAGER_HPP
#define OPTICS_MANAGER_HPP

#include "Subsystem.hpp"
#include "interfaces/IInformationDisplay.hpp"
#include "interfaces/IOpticsManager.hpp"
#include <chrono>

class OpticsManager : public IOpticsManager, public Subsystem
{
public:
    OpticsManager(std::string subsystemName,  std::shared_ptr<Logger> logger) : Subsystem(subsystemName, logger) {}

    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override;
    void threadLoop() override;

    // Includes from IOpticsManager
    std::string takePhoto(const CmdTakePhoto& cmd) override;
    std::string takeVideo(const CmdTakeVideo& cmd) override;
    std::string takeTimelapse(const CmdTakeTimelapse& cmd) override;
    void userChangeFocus(const CmdUserFocus& cmd) override;

private:
    std::weak_ptr<IInformationDisplay> myInformationDisplay;
};

#endif