#include "OpticsManager.hpp"

class MinOpticsManager : public OpticsManager
{
public:
   MinOpticsManager() = default;
   ~MinOpticsManager() override = default;

   MinOpticsManager(const MinOpticsManager&) = delete;
   MinOpticsManager& operator=(const MinOpticsManager&) = delete;
   MinOpticsManager(MinOpticsManager&&) = delete;
   MinOpticsManager& operator=(MinOpticsManager&&) = delete;
   
   std::string takePhoto(const CmdTakePhoto& cmd) override { return ""; }
   std::string takeVideo(const CmdTakeVideo& cmd) override { return ""; } 
   std::string takeTimelapse(const CmdTakeTimelapse& cmd) override { return ""; }
   void userChangeFocus(const CmdUserFocus& cmd) override {}
   void start() override {}
   void stop() override {}
   void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                                      static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) override {}
};
