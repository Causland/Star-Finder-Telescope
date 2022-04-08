#ifndef STAR_TRACKER_HPP
#define STAR_TRACKER_HPP

#include <memory>
#include "interfaces/IStarTracker.hpp"
#include "Subsystem.hpp"
#include "StarDatabase.hpp"

class StarTracker : public IStarTracker, public Subsystem
{
    // Things for Star Tracker to do
    // - Wait for particular star input
    //         - If tracking mode, set update frequency and query database for position at Hz
    //         - If position query mode, get the position from the database
    // - Inform position manager of new coordinates
public:
    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces() override;
    bool checkHeartbeat() override;
    void threadLoop() override;

    // Includes from IStarTracker
    void pointToTarget(std::string targetName) override;
    void trackTarget(std::string targetName, uint16_t updateFreqInHz) override;
    void queryTargetPosition(std::string targetName) override;
    void queryTargetsWithinRange(double rangeInLightMinutes) override; 
private:
    StarDatabase database;
};

#endif