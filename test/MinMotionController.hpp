#include "interfaces/IMotionController.hpp"
#include "interfaces/ISubsystem.hpp"

class MinMotionController : public IMotionController, public ISubsystem
{
public:
   void moveFocusKnob(double theta) override {}
   void moveHorizAngle(double theta) override {}
   void moveVertAngle(double phi) override {}
   void start() override {}
   void stop() override {}
   void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override {}
   bool checkHeartbeat() { return true; }
   std::string getName() { return NAME; }
};