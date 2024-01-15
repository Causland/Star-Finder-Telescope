#ifndef OPTICS_MANAGER_HPP
#define OPTICS_MANAGER_HPP

#include "CommandTypes.hpp"
#include "Subsystem.hpp"
#include <chrono>

class InformationDisplay;

class OpticsManager : public Subsystem
{
public:
   OpticsManager(std::string subsystemName) : Subsystem(subsystemName) {}
   virtual ~OpticsManager() = default;

   // Includes from ISubsystem
   void start() override;
   void stop() override;
   void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                             static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) override;

   virtual std::string takePhoto(const CmdTakePhoto& cmd);
   virtual std::string takeVideo(const CmdTakeVideo& cmd);
   virtual std::string takeTimelapse(const CmdTakeTimelapse& cmd);
   virtual void userChangeFocus(const CmdUserFocus& cmd);

   static const std::string NAME;
private:
   void threadLoop() override;
    
   std::weak_ptr<InformationDisplay> myInformationDisplay;
};

#endif
