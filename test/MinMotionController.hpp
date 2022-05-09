#include "interfaces/IMotionController.hpp"

const std::string IMotionController::NAME{"SimMotionController"};

class MinMotionController : public IMotionController
{
public:
   void moveFocusKnob(double theta) override {}
   void moveHorizAngle(double theta) override {}
   void moveVertAngle(double phi) override {}
};