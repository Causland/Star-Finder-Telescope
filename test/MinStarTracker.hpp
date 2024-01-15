#include "StarTracker.hpp"

class MinStarTracker : public StarTracker
{
public:
   MinStarTracker() : StarTracker(NAME, nullptr, nullptr) {}
   virtual ~MinStarTracker() = default;
   void pointToTarget(const CmdGoToTarget& cmd) override {}
   void trackTarget(const CmdFollowTarget& cmd) override {}
   void searchForTargets(const CmdSearchTarget& cmd) override {}
   void start() override {}
   void stop() override {}
   void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                                      static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) override {}
   bool checkHeartbeat() override { return true; }
   std::string getName() override { return NAME; }
};
