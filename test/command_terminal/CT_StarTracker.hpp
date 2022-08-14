#include "MinStarTracker.hpp"

class CT_StarTracker : public MinStarTracker
{
public:
   void pointToTarget(const CmdGoToTarget& cmd) override 
   {
      myGoToTargetCmd = cmd;
      myCommandReceived = true;
   }
   void trackTarget(const CmdFollowTarget& cmd) override 
   {
      myFollowTargetCmd = cmd;
      myCommandReceived = true;
   }
   void searchForTargets(const CmdSearchTarget& cmd) override 
   {
      mySearchTargetCmd = cmd;
      myCommandReceived = true;
   }
   void reset()
   {
      myGoToTargetCmd = CmdGoToTarget();
      myFollowTargetCmd = CmdFollowTarget();
      mySearchTargetCmd = CmdSearchTarget();
      myCommandReceived = false;
   }

   CmdGoToTarget myGoToTargetCmd{};
   CmdFollowTarget myFollowTargetCmd{};
   CmdSearchTarget mySearchTargetCmd{};
   bool myCommandReceived{false};
};