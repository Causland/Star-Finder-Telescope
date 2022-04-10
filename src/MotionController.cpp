#include "MotionController.hpp"
#include <algorithm>

const std::string IMotionController::NAME{"MotionController"};

void MotionController::start()
{
   myThread = std::thread(&MotionController::threadLoop, this);
}

void MotionController::stop()
{
   myExitingFlag = true;
   myThread.join();
}

void MotionController::configureInterfaces([[maybe_unused]] const std::vector<std::shared_ptr<ISubsystem>>& subsystems)
{
   // Motion Controller does not use any subsystem interfaces
}

void MotionController::threadLoop()
{
   while (!myExitingFlag)
   {
      // Things for the motion controller to do
      // Move servo by a certain degree on command
      // Differentiate between camera and telescope position servo
      // Keep track of movement progress/completion
   }
}

void MotionController::moveFocusKnob(double theta)
{

}

void MotionController::moveHorizAngle(double theta)
{

}

void MotionController::moveVertAngle(double phi)
{
   
}