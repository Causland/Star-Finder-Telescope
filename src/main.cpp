// Star Finder Telescope
// Track and maintain focus on targets across the sky with a telescope

#include "CommandTerminal.hpp"
#include "InformationDisplay.hpp"
#include "Logger.hpp"
#include "OpticsManager.hpp"
#include "PositionManager.hpp"
#include "PropertyManager.hpp"
#include "StarTracker.hpp"
#include "Subsystem.hpp"
#include "interfaces/GpsModule/SimGpsModule.hpp"
#include "interfaces/MotionController/RPi3MotionController.hpp"
#include "interfaces/MotionController/SimMotionController.hpp"
#include "interfaces/StarDatabase/SimStarDatabase.hpp"
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

    // Initialize the logger to be used throughout the program.
    // Use the current date and time to name the log file
    std::ostringstream oss;
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    oss << "logs/" << std::put_time(std::localtime(&t), "%m-%d-%Y-%I-%M%p") << ".log";
    std::string logFileName = oss.str();
    Logger::initialize(logFileName);

    // Initialize the properties manager for use across all subsystems
    if (!PropertyManager::initialize("properties.toml"))
    {
        Logger::log("main", LogCodeEnum::ERROR, "Unable to initialize the property manager");
        return 1;
    }

    // Create supporting modules for various subsystems
    std::shared_ptr<IMotionController> motionController;
#ifdef RASPBERRY_PI
    try
    {
        motionController = std::make_shared<RPi3MotionController>(); // Raspberry Pi 3 interface
    }
    catch (const std::runtime_error& e)
    {
        Logger::log("main", LogCodeEnum::ERROR, "Unable to create RPi3 Motion Controller");
        return 1;
    }
#else
    motionController = std::make_shared<SimMotionController>();
#endif

    std::shared_ptr<IGpsModule> gpsModule = std::make_shared<SimGpsModule>();
    std::shared_ptr<IStarDatabase> starDatabase = std::make_shared<SimStarDatabase>();

    // Construct all subsystems with their name and logger and push to subsystem vector
    std::vector<std::shared_ptr<Subsystem>> subsystems;
    subsystems.emplace_back(std::make_shared<InformationDisplay>("InformationDisplay"));
    subsystems.emplace_back(std::make_shared<StarTracker>("StarTracker", starDatabase, gpsModule));
    subsystems.emplace_back(std::make_shared<OpticsManager>("OpticsManager"));
    subsystems.emplace_back(std::make_shared<PositionManager>("PositionManager", motionController));
    subsystems.emplace_back(std::make_shared<CommandTerminal>("CommandTerminal", exitSignal));

    // Update the subsystem interfaces before starting their thread loops
    for (auto& subsystem : subsystems)
    {
        subsystem->configureSubsystems(subsystems);
    }
        
    // Start all subsystems to begin functionality
    for (auto& subsystem : subsystems)
    {
        Logger::log(subsystem->getName(), LogCodeEnum::INFO, "Starting");
        subsystem->start();
    }

    // Check heartbeat until program termination
    while (!(*exitSignal))
    {
        for (auto& subsystem : subsystems)
        {
            if (!subsystem->checkHeartbeat())
            {
                Logger::log(subsystem->getName(), LogCodeEnum::ERROR, "Heartbeat failure");
            }
        }
        std::this_thread::sleep_for(HEARTBEAT_CHECK_INTERVAL_MS);
    }

    // Stop all subsystems
    for (auto& subsystem : subsystems)
    {
        Logger::log(subsystem->getName(), LogCodeEnum::INFO, "Stopping");
        subsystem->stop();
    }

    PropertyManager::terminate();
    Logger::terminate();
    
    return 0;
}