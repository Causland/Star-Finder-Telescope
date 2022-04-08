#ifndef I_OPTICS_MANAGER_HPP
#define I_OPTICS_MANAGER_HPP

#include <string>

class IOpticsManager
{
public:
    virtual std::string takePhoto() = 0;
    virtual std::string takeVideo(double durationInSeconds) = 0;
    virtual std::string takeTimelapse(double durationInMinutes, double freqInHz) = 0;
};

#endif