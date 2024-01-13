#include "CommandTerminal.hpp"

class MinCommandTerminal : public CommandTerminal
{
public:
   MinCommandTerminal() : CommandTerminal(NAME, nullptr) {};
   void start() override {}
   void stop() override {}
   void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                                      static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) override {}
   bool checkHeartbeat() override { return true; }
   std::string getName() override { return NAME; }
};
