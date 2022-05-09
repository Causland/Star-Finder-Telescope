#include "interfaces/IStarTracker.hpp"

const std::string IStarTracker::NAME{"SimStarTracker"};

class MinStarTracker : public IStarTracker
{
public:
   void pointToTarget(const CmdGoToTarget& cmd) override {}
   void trackTarget(const CmdFollowTarget& cmd) override {}
   void queryTarget(const CmdSearchTarget& cmd) override {}
};