#ifndef I_STAR_TRACKER_HPP
#define I_STAR_TRACKER_HPP

#include <string>

class IStarTracker
{
public:
    virtual void pointToTarget(std::string targetName) = 0;
    virtual void trackTarget(std::string targetName, unsigned short updateFreqInHz) = 0;
    virtual void queryTargetPosition(std::string targetName) = 0;
    virtual void queryTargetsWithinRange(double rangeInLightMinutes) = 0; 
};

#endif