// Star Finder Telescope
// Track and maintain focus on targets across the sky with a telescope

#include "CommandTerminal.hpp"
#include "InformationDisplay.hpp"
#include "Logger.hpp"
#include "OpticsManager.hpp"
#include "PositionManager.hpp"
#include "interfaces/RPi3MotionController.hpp"
#include "StarDatabaseAdapter.hpp"
#include "StarTracker.hpp"
#include "interfaces/ISubsystem.hpp"
#include "interfaces/sim/SimMotionController.hpp"
#include <chrono>
#include <exception>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

// The main function creates all resources for each subsystem of the telescope
int main()
{
    // Create the flag which controls the lifetime of the main thread and execution
    // of the program. This flag is set by the CommandTerminal when an "exit" command
    // is entered by the user.
    std::shared_ptr<std::atomic<bool>> exitSignal = std::make_shared<std::atomic<bool>>(false);

    // Create logger(s) to pass to the constructor of each subsystem
    // Use the current date and time to name the log file
    std::ostringstream oss;
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    oss << "logs/" << std::put_time(std::localtime(&t), "%m-%d-%Y-%I-%M%p") << ".log";
    std::string logFileName = oss.str();
    std::shared_ptr<Logger> logger = std::make_shared<Logger>(logFileName);

    // Create supporting modules for various subsystems
    std::shared_ptr<IMotionController> motionController;
#ifdef RASPBERRY_PI
    try
    {
        motionController = std::make_shared<RPi3MotionController>(); // Raspberry Pi 3 interface
    }
    catch (const std::runtime_error& e)
    {
        logger->log("main", LogCodeEnum::ERROR, "Unable to create RPi3 Motion Controller");
        return 1;
    }
#else
    motionController = std::make_shared<SimMotionController>();
#endif

    // Construct all subsystems with their name and logger and push to subsystem vector
    std::vector<std::shared_ptr<ISubsystem>> subsystems;
    subsystems.emplace_back(std::make_shared<InformationDisplay>("InformationDisplay", logger));
    subsystems.emplace_back(std::make_shared<StarTracker>("StarTracker", logger));
    subsystems.emplace_back(std::make_shared<OpticsManager>("OpticsManager", logger));
    subsystems.emplace_back(std::make_shared<PositionManager>("PositionManager", logger, motionController));
    subsystems.emplace_back(std::make_shared<CommandTerminal>("CommandTerminal", logger, exitSignal));

    // Update the subsystem interfaces before starting their thread loops
    for (auto& subsystem : subsystems)
    {
        subsystem->configureInterfaces(subsystems);
    }
        
    // Start all subsystems to begin functionality
    for (auto& subsystem : subsystems)
    {
        logger->log(subsystem->getName(), LogCodeEnum::INFO, "Starting");
        subsystem->start();
    }

    // Check heartbeat until program termination
    while (!(*exitSignal))
    {
        for (auto& subsystem : subsystems)
        {
            if (!subsystem->checkHeartbeat())
            {
                logger->log(subsystem->getName(), LogCodeEnum::ERROR, "Heartbeat failure");
            }
        }
        std::this_thread::sleep_for(HEARTBEAT_CHECK_INTERVAL_MS);
    }

    // Stop all subsystems
    for (auto& subsystem : subsystems)
    {
        logger->log(subsystem->getName(), LogCodeEnum::INFO, "Stopping");
        subsystem->stop();
    }
    
    return 0;
}