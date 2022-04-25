#ifndef I_OPTICS_MANAGER_HPP
#define I_OPTICS_MANAGER_HPP

#include "CommandTypes.hpp"
#include <chrono>
#include <string>

class IOpticsManager
{
public:
    virtual std::string takePhoto(const CmdTakePhoto& cmd) = 0;
    virtual std::string takeVideo(const CmdTakeVideo& cmd) = 0;
    virtual std::string takeTimelapse(const CmdTakeTimelapse& cmd) = 0;
    virtual void userChangeFocus(const CmdUserFocus& cmd) = 0;

    static const std::string NAME;
};

#endif