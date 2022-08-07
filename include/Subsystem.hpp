#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include "Logger.hpp"
#include "interfaces/ISubsystem.hpp"

class Subsystem : public ISubsystem
{
public:
   Subsystem(std::string subsystemName,  std::shared_ptr<Logger> logger) : 
            mySubsystemName(std::move(subsystemName)), 
            myLogger(logger) {}
   
   bool checkHeartbeat() override
   {
      auto statusFlag = myHeartbeatFlag.load();
      myHeartbeatFlag = false;
      return statusFlag;
   }

   std::string getName() override
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