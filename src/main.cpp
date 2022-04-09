// Star Finder Telescope
// Track and maintain focus on targets across the sky with a telescope

#include "CommandTerminal.hpp"
#include "InformationDisplay.hpp"
#include "Logger.hpp"
#include "MotionController.hpp"
#include "OpticsManager.hpp"
#include "PositionManager.hpp"
#include "StarDatabase.hpp"
#include "StarTracker.hpp"
#include "interfaces/ISubsystem.hpp"
#include <chrono>
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
    // Create logger(s) to pass to the constructor of each subsystem
    // Use the current date and time to name the log file
    std::ostringstream oss;
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    oss << "logs/" << std::put_time(std::localtime(&t), "%m-%d-%y_%H-%M") << ".log";
    std::string logFileName = oss.str();
    std::shared_ptr<Logger> logger = std::make_shared<Logger>(logFileName);

    // Construct all subsystems with their name and logger and push to subsystem vector
    std::vector<std::shared_ptr<ISubsystem>> subsystems;
    InformationDisplay test("Test", logger);
    subsystems.emplace_back(std::make_shared<InformationDisplay>("InformationDisplay", logger));
    subsystems.emplace_back(std::make_shared<StarTracker>("StarTracker", logger));
    subsystems.emplace_back(std::make_shared<MotionController>("MotionController", logger));
    subsystems.emplace_back(std::make_shared<OpticsManager>("OpticsManager", logger));
    subsystems.emplace_back(std::make_shared<PositionManager>("PositionManager", logger));
    subsystems.emplace_back(std::make_shared<CommandTerminal>("CommandTerminal", logger));

    // Update the subsystem interfaces before starting their thread loops
    for (auto& subsystem : subsystems)
    {
        subsystem->configureInterfaces(subsystems);
    }
        
    // Start all subsystems to begin functionality
    for (auto& subsystem : subsystems)
    {
        subsystem->start();
    }

    // Check heartbeat until program termination
    volatile bool runningFlag = true;
    while (runningFlag)
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

    // Stop all subsystems and exit cleanly
    for (auto& subsystem : subsystems)
    {
        subsystem->stop();
    }
}