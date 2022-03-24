// Star Finder Telescope
// Track and maintain focus on targets across the sky with a telescope

#include <memory>

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "interfaces/ISubsystem.hpp"
#include "CommandTerminal.hpp"
#include "StarDatabase.hpp"
#include "InformationDisplay.hpp"
#include "MotionController.hpp"
#include "OpticsManager.hpp"
#include "PositionManager.hpp"
#include "StarTracker.hpp"

// The main function creates all resources for each subsystem of the telescope
int main()
{
    // Things to implement in the main function
    // - Create loggers
    auto console = spdlog::stdout_color_mt("console");
    std::shared_ptr<spdlog::logger> general_logger;
    std::shared_ptr<spdlog::logger> display_logger;
    std::shared_ptr<spdlog::logger> database_logger;
    std::shared_ptr<spdlog::logger> star_tracker_logger;
    std::shared_ptr<spdlog::logger> motion_control_logger;
    std::shared_ptr<spdlog::logger> position_logger;
    std::shared_ptr<spdlog::logger> optics_logger;
    std::shared_ptr<spdlog::logger> command_logger;
    try 
    {
        general_logger = spdlog::basic_logger_mt("general", "logs/log.txt");
        display_logger = spdlog::basic_logger_mt("display", "logs/log.txt");
        database_logger = spdlog::basic_logger_mt("database", "logs/log.txt");
        star_tracker_logger = spdlog::basic_logger_mt("star_tracker", "logs/log.txt");
        motion_control_logger = spdlog::basic_logger_mt("motion_controller", "logs/log.txt");
        position_logger = spdlog::basic_logger_mt("position_manager", "logs/log.txt");
        optics_logger = spdlog::basic_logger_mt("optics_manager", "logs/log.txt");
        command_logger = spdlog::basic_logger_mt("command_terminal", "logs/log.txt");
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        console->error("Log init failed: " + std::string(ex.what()));
    }

    // - Construct all subsystems and default initialize
    std::shared_ptr<ISubsystem> infoDisplay = std::make_shared<InformationDisplay>();
    std::shared_ptr<ISubsystem> starTracker = std::make_shared<StarTracker>();
    std::shared_ptr<ISubsystem> motionController = std::make_shared<MotionController>();
    std::shared_ptr<ISubsystem> opticsManager = std::make_shared<OpticsManager>();
    std::shared_ptr<ISubsystem> positionManager = std::make_shared<PositionManager>();
    std::shared_ptr<ISubsystem> commandTerminal = std::make_shared<CommandTerminal>();
        
    // - Start all subsystems to begin functionality
    infoDisplay->start();
    starTracker->start();
    motionController->start();
    opticsManager->start();
    positionManager->start();
    commandTerminal->start();

    // - Check heartbeat until program termination
    volatile bool running = true;
    while (running)
    {
        if (infoDisplay->checkHeartbeat())
            general_logger->error("Heartbeat failure for Info Display");
        if (starTracker->checkHeartbeat())
            general_logger->error("Heartbeat failure for Star Tracker");
        if (motionController->checkHeartbeat())
            general_logger->error("Heartbeat failure for Motion Controller");
        if (opticsManager->checkHeartbeat())
            general_logger->error("Heartbeat failure for Optics Manager");
        if (positionManager->checkHeartbeat())
            general_logger->error("Heartbeat failure for Position Manager");
        if (commandTerminal->checkHeartbeat())
            general_logger->error("Heartbeat failure for Command Terminal");
    }

    // - Stop all subsystems and exit cleanly
    infoDisplay->stop();
    starTracker->stop();
    motionController->stop();
    opticsManager->stop();
    positionManager->stop();
    commandTerminal->stop();
}