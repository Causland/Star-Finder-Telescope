#ifndef STAR_TRACKER_HPP
#define STAR_TRACKER_HPP

#include <memory>
#include "interfaces/IStarTracker.hpp"
#include "StarDatabase.hpp"

class StarTracker : public IStarTracker
{
    // Things for Star Tracker to do
    // - Wait for particular star input
    //         - If tracking mode, set update frequency and query database for position at Hz
    //         - If position query mode, get the position from the database
    // - Inform position manager of new coordinates
public:
    void pointToTarget(std::string targetName);
    void trackTarget(std::string targetName, unsigned short updateFreqInHz);
    void queryTargetPosition(std::string targetName);
    void queryTargetsWithinRange(double rangeInLightMinutes); 
private:
    StarDatabase database;
};

#endif