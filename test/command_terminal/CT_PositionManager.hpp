#include "MinPositionManager.hpp"

class CT_PositionManager : public MinPositionManager
{
public:
   CT_PositionManager() = default;
   ~CT_PositionManager() override = default;
   CT_PositionManager(const CT_PositionManager&) = delete;
   CT_PositionManager& operator=(const CT_PositionManager&) = delete;
   CT_PositionManager(CT_PositionManager&&) = delete;
   CT_PositionManager& operator=(CT_PositionManager&&) = delete;

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
