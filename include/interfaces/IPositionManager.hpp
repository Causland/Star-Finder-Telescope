#ifndef I_POSITION_MANAGER_HPP
#define I_POSITION_MANAGER_HPP

#include <cmath>
#include "interfaces/ISubsystem.hpp"

struct StarPosition
{
    StarPosition(double x, double y, double z) : x(x), y(y), z(z)
    {
        dist = sqrt(x*x + y*y + z*z);
    }
    double x;
    double y;
    double z;
    double dist;
};

class IPositionManager : public ISubsystem
{
public:
    void userChangePosition(double theta, double phi);
    void pointAtTarget(StarPosition position);
};

#endif