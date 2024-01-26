#include "InformationDisplay.hpp"

class MinInformationDisplay : public InformationDisplay
{
public:
   MinInformationDisplay() : InformationDisplay{nullptr, nullptr, nullptr} {};
   ~MinInformationDisplay() override = default;

   MinInformationDisplay(const MinInformationDisplay&) = delete;
   MinInformationDisplay(MinInformationDisplay&&) = delete;
   MinInformationDisplay& operator=(const MinInformationDisplay&) = delete;
   MinInformationDisplay& operator=(MinInformationDisplay&&) = delete;

   void start() override {}
   void stop() override {}
   void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                                      static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) override {}
};
