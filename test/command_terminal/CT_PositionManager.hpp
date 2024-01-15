#include "MinPositionManager.hpp"

class CT_PositionManager : public MinPositionManager
{
public:
   virtual ~CT_PositionManager() = default;

   void updatePosition(const CmdUpdatePosition& cmd) override 
   {
      myUpdatePositionCmd = cmd;
      myCommandReceived = true;
   }
   void calibrate(const CmdCalibrate& cmd) override 
   {
      myCalibrateCmd = cmd;
      myCommandReceived = true;
   }
   void reset()
   {
      myUpdatePositionCmd = CmdUpdatePosition();
      myCalibrateCmd = CmdCalibrate();
      myCommandReceived = false;
   }

   CmdUpdatePosition myUpdatePositionCmd{};
   CmdGoToTarget myGoToTargetCmd{};
   CmdCalibrate myCalibrateCmd{};
   bool myCommandReceived{false};
};
