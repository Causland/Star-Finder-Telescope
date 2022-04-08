#ifndef COMMAND_TERMINAL_HPP
#define COMMAND_TERMINAL_HPP

#include "interfaces/ICommandTerminal.hpp"
#include "Subsystem.hpp"

class CommandTerminal : public ICommandTerminal, public Subsystem
{
    // Things for command terminal to do
    // - Process entered commands
    //      - Validate the command
    // - Pass along command to desired subsystem
    // - Process series of timed commands from file

public:
    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces() override;
    bool checkHeartbeat() override;
    void threadLoop() override;
};

#endif