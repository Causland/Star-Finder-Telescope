#ifndef COMMAND_TERMINAL_HPP
#define COMMAND_TERMINAL_HPP

#include "CommandTypes.hpp"
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
    CommandTerminal(std::string subsystemName,  std::shared_ptr<Logger> logger, std::shared_ptr<std::atomic<bool>> exitingSignal) : 
        Subsystem(subsystemName, logger), myExitingSignal(exitingSignal)
    {
        myInputWaitingThread = std::thread(&CommandTerminal::cinWaitThreadLoop, this);
    }

    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override;

private:
    void threadLoop() override;
    void cinWaitThreadLoop();
    bool interpretCommand(const std::string& command);
    bool processCommand(const Command& command); 

    std::queue<std::string> myCommandQueue;
    std::thread myInputWaitingThread;
    std::shared_ptr<std::atomic<bool>> myExitingSignal;

    std::weak_ptr<IInformationDisplay> myInformationDisplay;
    std::weak_ptr<IOpticsManager> myOpticsManager;
    std::weak_ptr<IPositionManager> myPositionManager;
    std::weak_ptr<IStarTracker> myStarTracker;
};

#endif