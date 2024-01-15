#ifndef I_MOTION_CONTROLLER_HPP
#define I_MOTION_CONTROLLER_HPP

#include <string>

class IMotionController
{
public:
   virtual ~IMotionController() = default;
   virtual void moveFocusKnob(const double& theta, const double& theta_dot) = 0;
   virtual void moveHorizAngle(const double& theta, const double& theta_dot) = 0;
   virtual void moveVertAngle(const double& phi, const double& phi_dot) = 0;
   virtual std::string getDisplayInfo() = 0;
};

#endif
