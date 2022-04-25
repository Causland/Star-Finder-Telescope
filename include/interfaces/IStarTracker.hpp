#ifndef I_STAR_TRACKER_HPP
#define I_STAR_TRACKER_HPP

#include "CommandTypes.hpp"
#include <string>

class IStarTracker
{
public:
    virtual void pointToTarget(const CmdGoToTarget& cmd) = 0;
    virtual void trackTarget(const CmdFollowTarget& cmd) = 0;
    virtual void queryTarget(const CmdSearchTarget& cmd) = 0;

    static const std::string NAME;
};

#endif