#ifndef SIM_MOTION_CONTROLLER_HPP
#define SIM_MOTION_CONTROLLER_HPP

#include "interfaces/IMotionController.hpp"

class SimMotionController : public IMotionController
{
public:
   void moveFocusKnob(const double& theta, const double& theta_dot) override;
   void moveHorizAngle(const double& theta, const double& theta_dot) override;
   void moveVertAngle(const double& phi, const double& phi_dot) override;
};

#endif