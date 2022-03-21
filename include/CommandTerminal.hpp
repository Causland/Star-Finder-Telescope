#ifndef COMMAND_TERMINAL_HPP
#define COMMAND_TERMINAL_HPP

#include "interfaces/ICommandTerminal.hpp"

class CommandTerminal : public ICommandTerminal
{
    // Things for command terminal to do
    // - Process entered commands
    //      - Validate the command
    // - Pass along command to desired subsystem
    // - Process series of timed commands from file
};

#endif