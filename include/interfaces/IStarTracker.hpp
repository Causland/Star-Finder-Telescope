#ifndef I_STAR_TRACKER_HPP
#define I_STAR_TRACKER_HPP

#include "ISubsystem.hpp"

class IStarTracker : public ISubsystem
{
public:
    void pointToTarget(std::string targetName);
    void trackTarget(std::string targetName, unsigned short updateFreqInHz);
    void queryTargetPosition(std::string targetName);
    void queryTargetsWithinRange(double rangeInLightMinutes); 
};

#endif