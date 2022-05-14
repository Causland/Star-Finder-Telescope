#include "interfaces/IStarTracker.hpp"
#include "interfaces/ISubsystem.hpp"

const std::string IStarTracker::NAME{"StarTracker"};

class MinStarTracker : public IStarTracker, public ISubsystem
{
public:
   void pointToTarget(const CmdGoToTarget& cmd) override {}
   void trackTarget(const CmdFollowTarget& cmd) override {}
   void queryTarget(const CmdSearchTarget& cmd) override {}
   void start() override {}
   void stop() override {}
   void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override {}
   bool checkHeartbeat() { return true; }
   std::string getName() { return NAME; }
};