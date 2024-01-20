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

class StarTracker : public Subsystem
{
public:
   StarTracker(std::shared_ptr<IStarDatabase> starDatabase, std::shared_ptr<IGpsModule> gpsModule) : 
                  Subsystem{"StarTracker"}, myStarDatabase{std::move(starDatabase)}, myGpsModule{std::move(gpsModule)} {}
   ~StarTracker() override = default;

   StarTracker(const StarTracker&) = delete;
   StarTracker& operator=(const StarTracker&) = delete;
   StarTracker(StarTracker&&) = delete;
   StarTracker& operator=(StarTracker&&) = delete;

   // Includes from ISubsystem
   void start() override;
   void stop() override;
   void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                             static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) override;

   void pointToTarget(const CmdGoToTarget& cmd);
   void trackTarget(const CmdFollowTarget& cmd);
   void searchForTargets(const CmdSearchTarget& cmd);

   void processGoTo(const CmdGoToTarget& cmd);
   void processFollow(const CmdFollowTarget& cmd);
   void processSearch(const CmdSearchTarget& cmd);

private:
   void threadLoop() override;
    
   double myGpsLong{0.0};
   double myGpsLat{0.0};
   double myGpsElev{0.0};
   std::queue<std::variant<CmdGoToTarget, CmdFollowTarget, CmdSearchTarget>> myCommandQueue{};
   std::shared_ptr<IStarDatabase> myStarDatabase;
   std::shared_ptr<IGpsModule> myGpsModule;
   std::weak_ptr<InformationDisplay> myInformationDisplay;
   std::weak_ptr<PositionManager> myPositionManager;
};

#endif
