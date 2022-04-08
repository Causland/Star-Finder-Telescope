#ifndef INFORMATION_DISPLAY_HPP
#define INFORMATION_DISPLAY_HPP

#include "interfaces/IInformationDisplay.hpp"
#include "Subsystem.hpp"

class InformationDisplay : public IInformationDisplay, public Subsystem
{
    // Things for information display to do
    // Query desired telemetry information from multiple subsystems
    // Display information in an easily viewable format
    // Select what information is displayed

public:
    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces() override;
    bool checkHeartbeat() override;
    void threadLoop() override;
};

#endif