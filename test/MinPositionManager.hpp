#include "interfaces/IPositionManager.hpp"
#include "interfaces/ISubsystem.hpp"

class MinPositionManager : public IPositionManager, public ISubsystem
{
public:
   void updatePosition(const CmdUpdatePosition& cmd) override {}
   void trackTarget(const PositionTable& positions) override {}
   void calibrate(const CmdCalibrate& cmd) override {}
   void start() override {}
   void stop() override {}
   void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override {}
   bool checkHeartbeat() { return true; }
   std::string getName() { return NAME; }
};