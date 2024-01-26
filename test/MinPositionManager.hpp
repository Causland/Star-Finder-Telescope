#include "PositionManager.hpp"

class MinPositionManager : public PositionManager
{
public:
   MinPositionManager() : PositionManager{nullptr} {}
   ~MinPositionManager() override = default;

   MinPositionManager(const MinPositionManager&) = delete;
   MinPositionManager& operator=(const MinPositionManager&) = delete;
   MinPositionManager(MinPositionManager&&) = delete;
   MinPositionManager& operator=(MinPositionManager&&) = delete;

   void updatePosition(const CmdUpdatePosition& cmd) override {}
   void trackTarget(std::vector<TrajectoryPoint>& path) override {};
   void calibrate(const CmdCalibrate& cmd) override {}
   void start() override {}
   void stop() override {}
   void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                                      static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) override {}
};
