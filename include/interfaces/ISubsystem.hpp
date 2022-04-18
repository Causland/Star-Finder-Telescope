#ifndef I_SUBSYSTEM_HPP
#define I_SUBSYSTEM_HPP

#include <chrono>
#include <memory>
#include <string>
#include <vector>

static const std::chrono::duration<uint64_t, std::milli> HEARTBEAT_CHECK_INTERVAL_MS(2000);
static const std::chrono::duration<uint64_t, std::milli> HEARTBEAT_UPDATE_INTERVAL_MS(HEARTBEAT_CHECK_INTERVAL_MS / 2);

class ISubsystem
{
public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) = 0;
    virtual bool checkHeartbeat() = 0;
    virtual std::string getName() = 0;
};

#endif