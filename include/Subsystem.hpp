#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include <memory>
#include <string>
#include <thread>
#include "interfaces/ISubsystem.hpp"
#include "spdlog/sinks/basic_file_sink.h"

class Subsystem : public ISubsystem
{
public:
   Subsystem(std::string subsystemName, std::shared_ptr<spdlog::logger> logger) : 
            mySubsystemName(subsystemName), 
            myLogger(logger), 
            myHeartbeatFlag(false),
            myThread(threadLoop) {};

protected:
   std::shared_ptr<spdlog::logger> myLogger;
   bool myHeartbeatFlag;
   std::string mySubsystemName;
   std::thread myThread;
};

#endif