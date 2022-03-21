#ifndef POSITION_MANAGER_HPP
#define POSITION_MANAGER_HPP

#include "interfaces/IPositionManager.hpp"

class PositionManager : public IPositionManager
{
    // Things for position manager to do
    // - Listen for telescope position update commands from command interface
    // - Listen for star position update commands from star tracker
    // - Calculate motion required to move to new position
    // - Command the motion controller for telescope pointing
};

#endif