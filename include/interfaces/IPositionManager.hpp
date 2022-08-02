#ifndef I_POSITION_MANAGER_HPP
#define I_POSITION_MANAGER_HPP

#include "CommandTypes.hpp"
#include "Common.hpp"
#include <chrono>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

class IPositionManager
{
public:
    virtual void updatePosition(const CmdUpdatePosition& cmd) = 0;
    virtual void trackTarget(std::vector<std::pair<Position, std::chrono::system_clock::time_point>>& positions) = 0;
    virtual void calibrate(const CmdCalibrate& cmd) = 0;

    static const std::string NAME;
};

#endif