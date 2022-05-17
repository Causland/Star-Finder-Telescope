#include "MinPositionManager.hpp"

class CT_PositionManager : public MinPositionManager
{
public:
   void userChangePosition(const CmdUserMove& cmd) override 
   {
      myUserMoveCmd = cmd;
      myCommandReceived = true;
   }
   void pointAtTarget(const CmdGoToTarget& cmd) override 
   {
      myGoToTargetCmd = cmd;
      myCommandReceived = true;
   }
   void calibrate(const CmdCalibrate& cmd) override 
   {
      myCalibrateCmd = cmd;
      myCommandReceived = true;
   }
   void reset()
   {
      myUserMoveCmd = CmdUserMove();
      myGoToTargetCmd = CmdGoToTarget();
      myCalibrateCmd = CmdCalibrate();
      myCommandReceived = false;
   }

   CmdUserMove myUserMoveCmd{};
   CmdGoToTarget myGoToTargetCmd{};
   CmdCalibrate myCalibrateCmd{};
   bool myCommandReceived{false};
};