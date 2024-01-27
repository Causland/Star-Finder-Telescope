#ifndef OPTICS_MANAGER_HPP
#define OPTICS_MANAGER_HPP

#include "CommandTypes.hpp"
#include "InformationDisplay.hpp"
#include "Subsystem.hpp"

class OpticsManager : public Subsystem
{
public:
   OpticsManager() : Subsystem{"OpticsManager"} {}

   ~OpticsManager() override = default;

   OpticsManager(const OpticsManager&) = delete;
   OpticsManager& operator=(const OpticsManager&) = delete;
   OpticsManager(OpticsManager&&) = delete;
   OpticsManager& operator=(OpticsManager&&) = delete;

   // Includes from ISubsystem
   void start() override;
   void stop() override;
   void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                             static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) override;

   [[nodiscard]] virtual std::string takePhoto(const CmdTakePhoto& cmd);
   [[nodiscard]] virtual std::string takeVideo(const CmdTakeVideo& cmd);
   [[nodiscard]] virtual std::string takeTimelapse(const CmdTakeTimelapse& cmd);
   virtual void userChangeFocus(const CmdUserFocus& cmd);

private:
   void threadLoop() override;
    
   std::weak_ptr<InformationDisplay> myInformationDisplay;
};

#endif
