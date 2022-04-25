#ifndef I_POSITION_MANAGER_HPP
#define I_POSITION_MANAGER_HPP

#include "CommandTypes.hpp"
#include <cmath>
#include <string>

struct StarPosition
{
    StarPosition(double x, double y, double z) : myX(x), myY(y), myZ(z)
    {
        myDist = sqrt(x*x + y*y + z*z);
    }
    double myX;
    double myY;
    double myZ;
    double myDist;
};

class IPositionManager
{
public:
    virtual void userChangePosition(const CmdUserMove& cmd) = 0;
    virtual void pointAtTarget(const CmdGoToTarget& cmd) = 0;
    virtual void calibrate(const CmdCalibrate& cmd) = 0;

    static const std::string NAME;
};

#endif