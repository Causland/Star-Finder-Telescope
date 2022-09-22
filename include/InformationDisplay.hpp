#ifndef INFORMATION_DISPLAY_HPP
#define INFORMATION_DISPLAY_HPP

#include "Subsystem.hpp"

class CommandTerminal;
class OpticsManager;
class PositionManager;
class StarTracker;

class InformationDisplay : public Subsystem
{
public:
    InformationDisplay(std::string subsystemName) : Subsystem(subsystemName) {}

    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureSubsystems(const std::vector<std::shared_ptr<Subsystem>>& subsystems) override;

    void displaySearchResults(const std::string& displayString);

    static const std::string NAME;
private:
    void threadLoop() override;
    
    std::weak_ptr<CommandTerminal> myCommandTerminal;
    std::weak_ptr<OpticsManager> myOpticsManager;
    std::weak_ptr<PositionManager> myPositionManager;
    std::weak_ptr<StarTracker> myStarTracker;
};

#endif