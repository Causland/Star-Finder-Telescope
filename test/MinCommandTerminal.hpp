#include "CommandTerminal.hpp"

class MinCommandTerminal : public CommandTerminal
{
public:
   MinCommandTerminal() : CommandTerminal{nullptr} {};
   ~MinCommandTerminal() override = default;
   
   MinCommandTerminal(const MinCommandTerminal&) = delete;
   MinCommandTerminal& operator=(const MinCommandTerminal&) = delete;
   MinCommandTerminal(MinCommandTerminal&&) = delete;
   MinCommandTerminal& operator=(MinCommandTerminal&&) = delete;

   void start() override {}
   void stop() override {}
   void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                                      static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) override {}
};
