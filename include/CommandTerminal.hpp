#ifndef COMMAND_TERMINAL_HPP
#define COMMAND_TERMINAL_HPP

#include "Subsystem.hpp"
#include "interfaces/ICommandTerminal.hpp"
#include "interfaces/IInformationDisplay.hpp"
#include "interfaces/IOpticsManager.hpp"
#include "interfaces/IPositionManager.hpp"
#include "interfaces/IStarTracker.hpp"
#include <queue>
#include <string>

class CommandTerminal : public ICommandTerminal, public Subsystem
{
public:
    CommandTerminal(std::string subsystemName,  std::shared_ptr<Logger> logger) : Subsystem(subsystemName, logger) {}

    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override;
    void threadLoop() override;

private:
    bool validateCommand(const std::string& command);
    bool processCommand(const std::string& command);

    std::queue<std::string> myCommandQueue;

    std::shared_ptr<IInformationDisplay> myInformationDisplay;
    std::shared_ptr<IOpticsManager> myOpticsManager;
    std::shared_ptr<IPositionManager> myPositionManager;
    std::shared_ptr<IStarTracker> myStarTracker;
};

#endif