#include "PositionManager.hpp"

class MinPositionManager : public PositionManager
{
public:
   MinPositionManager() : PositionManager(NAME, nullptr, nullptr) {}
   void updatePosition(const CmdUpdatePosition& cmd) override {}
   void trackTarget(std::vector<std::pair<Position, std::chrono::system_clock::time_point>>& positions) override {};
   void calibrate(const CmdCalibrate& cmd) override {}
   void start() override {}
   void stop() override {}
   void configureSubsystems(const std::vector<std::shared_ptr<Subsystem>>& subsystems) override {}
   bool checkHeartbeat() { return true; }
   std::string getName() { return NAME; }
};