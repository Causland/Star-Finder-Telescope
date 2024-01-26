#ifndef STAR_TRACKER_HPP
#define STAR_TRACKER_HPP

#include "CommandTypes.hpp"
#include "InformationDisplay.hpp"
#include "PositionManager.hpp"
#include "Subsystem.hpp"
#include "interfaces/GpsModule/IGpsModule.hpp"
#include "interfaces/StarDatabase/IStarDatabase.hpp"

#include <memory>
#include <queue>
#include <variant>

/*!
 * The StarTracker subsystem is responsible for retrieving pointing position data for specific
 * targets and informing the PositionManager. It uses a database interface to query for the specified data.
 */ 
class StarTracker : public Subsystem
{
public:
   /*!
    * Constructs a Star Tracker with a provided database and GPS module.
    * \param[in] starDatabase a pointer to the database.
    * \param[in] gpsModule a pointer to the GPS module.
    */ 
   StarTracker(std::shared_ptr<IStarDatabase> starDatabase, std::shared_ptr<IGpsModule> gpsModule) : 
                  Subsystem{"StarTracker"}, myStarDatabase{std::move(starDatabase)}, myGpsModule{std::move(gpsModule)} {}

   /*!
    * Destroys a Star Tracker.
    */ 
   ~StarTracker() override = default;

   StarTracker(const StarTracker&) = delete;
   StarTracker& operator=(const StarTracker&) = delete;
   StarTracker(StarTracker&&) = delete;
   StarTracker& operator=(StarTracker&&) = delete;

   /*!
    * Start the Star Tracker.
    */ 
   void start() override;

   /*!
    * Stop the Star Tracker.
    */ 
   void stop() override;

   /*!
    * Acquire pointers to other subsystems in which this object interacts with from the provided
    * array of subsystems.
    * \param[in] subsystems an array of pointers to all available subsystems.
    */ 
   void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                             static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) override;

   /*!
    * Get the pointing position data for a specific target and inform the PositionManager.
    * \param[in] cmd the go to target command.
    */ 
   virtual void pointToTarget(const CmdGoToTarget& cmd);

   /*!
    * Get the pointing position data for a specific target continuously and inform the PositionManager
    * of new pointing data.
    * \param[in] cmd the follow target command.
    */ 
   virtual void trackTarget(const CmdFollowTarget& cmd);

   /*!
    * Search for targets within specified search criteria.
    * \param[in] cmd the search target command.
    */ 
   virtual void searchForTargets(const CmdSearchTarget& cmd);

   /*!
    * Process go to command.
    * \param[in] cmd the go to target command.
    */ 
   void processGoTo(const CmdGoToTarget& cmd);

   /*!
    * Process follow command.
    * \param[in] cmd the follow target command.
    */ 
   void processFollow(const CmdFollowTarget& cmd);

   /*!
    * Process search command.
    * \param[in] cmd the search target command.
    */ 
   void processSearch(const CmdSearchTarget& cmd);

private:
   /*!
    * The Star Tracker threadloop handles the processing of incoming commands. Additionally,
    * in the event of following a target, this thread continously updates position information
    * in the PositionManager.
    */ 
   void threadLoop() override;
    
   GpsPosition myGpsPosition; //!< The current GPS position.
   std::queue<std::variant<CmdGoToTarget, CmdFollowTarget, CmdSearchTarget>> myCommandQueue{}; //!< Holds various commands to process.
   std::shared_ptr<IStarDatabase> myStarDatabase; //!< The database to query for searches and position data.
   std::shared_ptr<IGpsModule> myGpsModule; //!< The GPS module to use to get current position.
   std::weak_ptr<InformationDisplay> myInformationDisplay; //!< A weak pointer to the Information Display subsystem.
   std::weak_ptr<PositionManager> myPositionManager; //!< A weak pointer to the Position Manager subsystem.
};

#endif
