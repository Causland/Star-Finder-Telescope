#include "MinPositionManager.hpp"

class CT_PositionManager : public MinPositionManager
{
public:
   void userChangePosition(const CmdUserMove& cmd) override 
   {

   }
   void pointAtTarget(const CmdGoToTarget& cmd) override 
   {

   }
   void calibrate(const CmdCalibrate& cmd) override 
   {
      
   }
};