#include "StarTracker.hpp"

class MinStarTracker : public StarTracker
{
public:
   MinStarTracker() : StarTracker(NAME, nullptr, nullptr) {}
   void pointToTarget(const CmdGoToTarget& cmd) override {}
   void trackTarget(const CmdFollowTarget& cmd) override {}
   void searchForTargets(const CmdSearchTarget& cmd) override {}
   void start() override {}
   void stop() override {}
   void configureSubsystems(const std::vector<std::shared_ptr<Subsystem>>& subsystems) override {}
   bool checkHeartbeat() override { return true; }
   std::string getName() override { return NAME; }
};