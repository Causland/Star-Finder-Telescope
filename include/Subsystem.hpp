#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include <array>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

constexpr std::chrono::milliseconds HEARTBEAT_CHECK_INTERVAL_MS(2000);
constexpr std::chrono::milliseconds HEARTBEAT_UPDATE_INTERVAL_MS(HEARTBEAT_CHECK_INTERVAL_MS / 2);

enum class SubsystemEnum
{
   INFORMATION_DISPLAY,
   STAR_TRACKER,
   OPTICS_MANAGER,
   POSITION_MANAGER,
   COMMAND_TERMINAL,
   NUM_SUBSYSTEMS,
};

class Subsystem
{
public:
   explicit Subsystem(std::string subsystemName) : mySubsystemName(std::move(subsystemName)) {}
   virtual ~Subsystem() = default;
   
   virtual void start() = 0;
   virtual void stop() = 0;
   virtual void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                                      static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) = 0;

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
   std::thread myThread;
   std::condition_variable myCondVar;
   std::mutex myMutex;
   std::atomic<bool> myHeartbeatFlag{true};
   std::atomic<bool> myExitingFlag{false};
};

#endif
