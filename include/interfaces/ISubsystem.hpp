#ifndef I_SUBSYSTEM_HPP
#define I_SUBSYSTEM_HPP

#include <memory>
#include <string>
#include <thread>
#include "spdlog/sinks/basic_file_sink.h"

class ISubsystem
{
public:
    ISubsystem(std::string subsystemName, std::shared_ptr<spdlog::logger> logger) : 
            mySubsystemName(subsystemName), 
            myLogger(logger), 
            myHeartbeatFlag(false),
            myThread(threadLoop) {};

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void configureInterfaces() = 0;
    virtual bool checkHeartbeat() = 0;
    virtual void threadLoop() = 0;

protected:
    std::shared_ptr<spdlog::logger> myLogger;
    bool myHeartbeatFlag;
    std::string mySubsystemName;
    std::thread myThread;

};

#endif