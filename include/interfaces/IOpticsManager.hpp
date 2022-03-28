#ifndef I_OPTICS_MANAGER_HPP
#define I_OPTICS_MANAGER_HPP

#include <string>
#include "interfaces/ISubsystem.hpp"

class IOpticsManager : public ISubsystem
{
public:
    std::string takePhoto();
    std::string takeVideo(double durationInSeconds);
    std::string takeTimelapse(double durationInMinutes, double freqInHz);
};

#endif