#include "interfaces/IPositionManager.hpp"

const std::string IPositionManager::NAME{"SimPositionManager"};

class MinPositionManager : public IPositionManager
{
public:
   void userChangePosition(const CmdUserMove& cmd) override {}
   void pointAtTarget(const CmdGoToTarget& cmd) override {}
   void calibrate(const CmdCalibrate& cmd) override {}
};