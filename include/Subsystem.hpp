#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include <array>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

static constexpr std::chrono::milliseconds HEARTBEAT_CHECK_INTERVAL_MS{2000};
static constexpr std::chrono::milliseconds HEARTBEAT_UPDATE_INTERVAL_MS{HEARTBEAT_CHECK_INTERVAL_MS / 2};

/*!
 * Enum class of the available subsystems.
 */
enum class SubsystemEnum
{
   INFORMATION_DISPLAY,
   STAR_TRACKER,
   OPTICS_MANAGER,
   POSITION_MANAGER,
   COMMAND_TERMINAL,
   NUM_SUBSYSTEMS,
};

/*!
 * The Subsystem class defines the common interfaces and members required by all subsystems.
 */ 
class Subsystem
{
public:
   /*!
    * Creates a subsystem with the provided name.
    * \param[in] subsystemName a string name of the subsystem moved into the object.
    */
   explicit Subsystem(std::string subsystemName) : mySubsystemName{std::move(subsystemName)} {}

   /*!
    * Destroys a Subsystem.
    */
   virtual ~Subsystem() = default;

   Subsystem(const Subsystem&) = delete;
   Subsystem(Subsystem&&) = delete;
   Subsystem& operator=(const Subsystem&) = delete;
   Subsystem& operator=(Subsystem&&) = delete;
   
   /*!
    * Start the subsystem.
    */ 
   virtual void start() = 0;

   /*!
    * Stop the subsystem.
    */ 
   virtual void stop() = 0;

   /*!
    * Acquire pointers to other subsystems in which this object interacts with from the provided
    * array of subsystems.
    * \param[in] subsystems an array of pointers to all available subsystems.
    */ 
   virtual void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                                      static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) = 0;

   /*!
    * Get the name of the subsystem.
    * \return a string_view of the name
    */ 
   [[nodiscard]] virtual std::string_view getName() const
   {
      return mySubsystemName;
   }

   /*!
    * Check the heatbeat of the subsystem. All subsystem threadLoops must set
    * the heartbeat status flag to true within the heartbeat interval.
    */ 
   bool checkHeartbeat()
   {
      const auto statusFlag{myHeartbeatFlag.load()};
      myHeartbeatFlag = false;
      return statusFlag;
   }

protected:
   /*!
    * Perform subsystem processing.
    */ 
   virtual void threadLoop() = 0;

   std::string mySubsystemName; //! The name of the subsystem. 
   std::thread myThread; //! The thread to perform processing in.
   std::condition_variable myCondVar; //! The condition variable used to signal the processing thread.
   std::mutex myMutex; //! The mutex used to guard access by multiple subsystems to interface variables.
   std::atomic<bool> myHeartbeatFlag{true}; //! Indicates the heartbeat status of the subsystem.
   std::atomic<bool> myExitingFlag{false}; //! The flag used to control the thread loop. True when exiting the thread.
};

#endif
