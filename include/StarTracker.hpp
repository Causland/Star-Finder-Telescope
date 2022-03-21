#ifndef STAR_TRACKER_HPP
#define STAR_TRACKER_HPP

#include "interfaces/IStarTracker.hpp"

class StarTracker : public IStarTracker
{
    // Things for Star Tracker to do
    // - Wait for particular star input
    //         - If tracking mode, set update frequency and query database for position at Hz
    //         - If position query mode, get the position from the database
    // - Inform position manager of new coordinates
};

#endif