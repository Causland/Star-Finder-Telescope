#include "interfaces/IPositionManager.hpp"
#include "interfaces/ISubsystem.hpp"

const std::string IPositionManager::NAME{"PositionManager"};

class MinPositionManager : public IPositionManager, public ISubsystem
{
public:
   void userChangePosition(const CmdUserMove& cmd) override {}
   void pointAtTarget(const CmdGoToTarget& cmd) override {}
   void calibrate(const CmdCalibrate& cmd) override {}
   void start() override {}
   void stop() override {}
   void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override {}
   bool checkHeartbeat() { return true; }
   std::string getName() { return NAME; }
};