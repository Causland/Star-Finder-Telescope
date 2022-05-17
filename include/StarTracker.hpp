#ifndef STAR_TRACKER_HPP
#define STAR_TRACKER_HPP

#include "Subsystem.hpp"
#include "StarDatabase.hpp"
#include "interfaces/IInformationDisplay.hpp"
#include "interfaces/IPositionManager.hpp"
#include "interfaces/IStarTracker.hpp"
#include <memory>

class StarTracker : public IStarTracker, public Subsystem
{
public:
    StarTracker(std::string subsystemName,  std::shared_ptr<Logger> logger) : Subsystem(subsystemName, logger) {}

    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override;
    void threadLoop() override;

    // Includes from IStarTracker
    void pointToTarget(const CmdGoToTarget& cmd) override;
    void trackTarget(const CmdFollowTarget& cmd) override;
    void queryTarget(const CmdSearchTarget& cmd) override;
    
private:
    StarDatabase database;
    std::weak_ptr<IInformationDisplay> myInformationDisplay;
    std::weak_ptr<IPositionManager> myPositionManager;
};

#endif