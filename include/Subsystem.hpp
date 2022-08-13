#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include "Logger.hpp"

constexpr std::chrono::milliseconds HEARTBEAT_CHECK_INTERVAL_MS(2000);
constexpr std::chrono::milliseconds HEARTBEAT_UPDATE_INTERVAL_MS(HEARTBEAT_CHECK_INTERVAL_MS / 2);

class Subsystem
{
public:
   Subsystem(std::string subsystemName,  std::shared_ptr<Logger> logger) : 
            mySubsystemName(std::move(subsystemName)), 
            myLogger(logger) {}
   
   virtual void start() = 0;
   virtual void stop() = 0;
   virtual void configureSubsystems(const std::vector<std::shared_ptr<Subsystem>>& subsystems) = 0;

   virtual bool checkHeartbeat()
   {
      auto statusFlag = myHeartbeatFlag.load();
      myHeartbeatFlag = false;
      return statusFlag;
   }

   virtual std::string getName()
   {
      return mySubsystemName;
   }

protected:
   virtual void threadLoop() = 0;

   std::string mySubsystemName;
   std::shared_ptr<Logger> myLogger;
   std::thread myThread;
   std::condition_variable myCondVar;
   std::mutex myMutex;
   std::atomic<bool> myHeartbeatFlag{true};
   std::atomic<bool> myExitingFlag{false};
};

#endif