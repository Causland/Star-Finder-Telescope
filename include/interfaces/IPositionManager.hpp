#ifndef I_POSITION_MANAGER_HPP
#define I_POSITION_MANAGER_HPP

#include <cmath>
#include <string>

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

class IPositionManager
{
public:
    virtual void userChangePosition(double theta, double phi) = 0;
    virtual void pointAtTarget(StarPosition position) = 0;

    static const std::string NAME;
};

#endif