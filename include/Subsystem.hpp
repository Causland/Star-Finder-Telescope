#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include <memory>
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
      bool statusFlag = myHeartbeatFlag;
      myHeartbeatFlag = false;
      return statusFlag;
   }

   std::string getName() override
   {
      return mySubsystemName;
   }

protected:
   std::shared_ptr<Logger> myLogger;
   bool myHeartbeatFlag{false};
   bool myExitingFlag{false};
   std::string mySubsystemName;
   std::thread myThread;
};

#endif