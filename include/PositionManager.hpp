#ifndef POSITION_MANAGER_HPP
#define POSITION_MANAGER_HPP

#include "interfaces/IPositionManager.hpp"
#include "Subsystem.hpp"

class PositionManager : public IPositionManager, public Subsystem
{
    // Things for position manager to do
    // - Listen for telescope position update commands from command interface
    // - Listen for star position update commands from star tracker
    // - Calculate motion required to move to new position
    // - Command the motion controller for telescope pointing

public:
    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces() override;
    bool checkHeartbeat() override;
    void threadLoop() override;

    // Includes from IPositionManager
    void userChangePosition(double theta, double phi) override;
    void pointAtTarget(StarPosition position) override;
};

#endif