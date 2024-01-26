#include "interfaces/MotionController/SimMotionController.hpp"

void SimMotionController::moveFocusKnob(const Rotation& rot)
{
   myFocusRot = rot;
}

void SimMotionController::moveHorizAngle(const Rotation& rot)
{
   myHorizRot = rot;
}

void SimMotionController::moveVertAngle(const Rotation& rot)
{
   myVertRot = rot;
}

std::string SimMotionController::getDisplayInfo()
{
   return "                 (theta) (theta_dot)\n"
          "Horiz Rotation: " + std::to_string(myHorizRot.theta) + " " + std::to_string(myHorizRot.theta_dot) + "\n"
          "Vert  Rotation: " + std::to_string(myVertRot.theta) + " " + std::to_string(myVertRot.theta_dot) + "\n"
          "Focus Rotation: " + std::to_string(myFocusRot.theta) + " " + std::to_string(myFocusRot.theta_dot) + "\n";
}
