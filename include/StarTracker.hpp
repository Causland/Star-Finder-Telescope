#ifndef STAR_TRACKER_HPP
#define STAR_TRACKER_HPP

#include "Subsystem.hpp"
#include "interfaces/IGpsModule.hpp"
#include "interfaces/IInformationDisplay.hpp"
#include "interfaces/IPositionManager.hpp"
#include "interfaces/IStarDatabase.hpp"
#include "interfaces/IStarTracker.hpp"
#include <memory>
#include <queue>

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
    double myGpsLong{0.0};
    double myGpsLat{0.0};
    double myGpsElev{0.0};
    std::queue<Command> myCommandQueue;
    std::weak_ptr<IStarDatabase> myStarDatabase;
    std::weak_ptr<IGpsModule> myGpsModule;
    std::weak_ptr<IInformationDisplay> myInformationDisplay;
    std::weak_ptr<IPositionManager> myPositionManager;
};

#endif