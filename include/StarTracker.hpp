#ifndef STAR_TRACKER_HPP
#define STAR_TRACKER_HPP

#include "CommandTypes.hpp"
#include "Subsystem.hpp"
#include "interfaces/GpsModule/IGpsModule.hpp"
#include "interfaces/StarDatabase/IStarDatabase.hpp"
#include <memory>
#include <queue>

class InformationDisplay;
class PositionManager;

class StarTracker : public Subsystem
{
public:
    StarTracker(std::string subsystemName,  std::shared_ptr<Logger> logger) : Subsystem(subsystemName, logger) {}

    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureSubsystems(const std::vector<std::shared_ptr<Subsystem>>& subsystems) override;

    virtual void pointToTarget(const CmdGoToTarget& cmd);
    virtual void trackTarget(const CmdFollowTarget& cmd);
    virtual void queryTarget(const CmdSearchTarget& cmd);
    
    static const std::string NAME;
private:
    void threadLoop() override;
    
    double myGpsLong{0.0};
    double myGpsLat{0.0};
    double myGpsElev{0.0};
    std::queue<Command> myCommandQueue{};
    std::weak_ptr<IStarDatabase> myStarDatabase;
    std::weak_ptr<IGpsModule> myGpsModule;
    std::weak_ptr<InformationDisplay> myInformationDisplay;
    std::weak_ptr<PositionManager> myPositionManager;
};

#endif