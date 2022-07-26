#ifndef I_POSITION_MANAGER_HPP
#define I_POSITION_MANAGER_HPP

#include "CommandTypes.hpp"
#include <chrono>
#include <cmath>
#include <string>
#include <vector>

struct Position
{
    double azimuth{0.0};
    double elevation{0.0};
    std::chrono::system_clock::time_point time;
};

struct PositionTable
{
    std::vector<Position> myPositions;
};

class IPositionManager
{
public:
    virtual void updatePosition(const CmdUpdatePosition& cmd) = 0;
    virtual void trackTarget(const PositionTable& positions) = 0;
    virtual void calibrate(const CmdCalibrate& cmd) = 0;

    static const std::string NAME;
};

#endif