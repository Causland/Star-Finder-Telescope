#ifndef COMMAND_TERMINAL_HPP
#define COMMAND_TERMINAL_HPP

#include "CommandTypes.hpp"
#include "Subsystem.hpp"
#include "interfaces/ICommandTerminal.hpp"
#include "interfaces/IInformationDisplay.hpp"
#include "interfaces/IOpticsManager.hpp"
#include "interfaces/IPositionManager.hpp"
#include "interfaces/IStarTracker.hpp"
#include <algorithm>
#include <cstdarg>
#include <queue>
#include <string>
#include <type_traits>
#include <iostream>

class CommandTerminal : public ICommandTerminal, public Subsystem
{
public:
    CommandTerminal(std::string subsystemName,  std::shared_ptr<Logger> logger, std::shared_ptr<std::atomic<bool>> exitingSignal) : 
        Subsystem(subsystemName, logger), myExitingSignal(exitingSignal)
    {
        myInputWaitingThread = std::thread(&CommandTerminal::cinWaitThreadLoop, this);
    }

    // Includes from ISubsystem
    void start() override;
    void stop() override;
    void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override;

private:
    void threadLoop() override;
    void cinWaitThreadLoop();
    bool interpretCommand(const std::string& command);

    bool validateParameters(const std::string& input)
    {
        if (!input.empty())
        {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Too many parameters provided for the command");
            return false;
        }
        return true;
    }

    template<typename T, typename... Args>
    bool validateParameters(const std::string& input, T& currParam, Args&... outParams)
    {
        if (input.empty())
        {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Not enough parameters provided for command");
            return false;
        }
        auto pos = input.find(' ');
        std::string val = input.substr(0, pos);
        std::string remainder;
        if (pos == std::string::npos)
        {
            remainder = "";
        }
        else
        {
            remainder = input.substr(pos + 1);
        }

        // Store result of converted parameter in the current parameter
        if (std::is_same_v<decltype(currParam), std::string&>)
        {
            currParam = reinterpret_cast<T&>(val);
        }
        else if (std::is_same_v<decltype(currParam), uint64_t&>)
        {
            char* end = nullptr;
            constexpr auto BASE_TEN = 10;
            uint64_t temp = strtoul(val.c_str(), &end, BASE_TEN);
            if (end == val.c_str() || *end != '\0')
            {
                myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Parameter " + val + " cannot be converted to long integer type");
                return false;
            }
            currParam = temp;
        }
        else if (std::is_same_v<decltype(currParam), double&>)
        {
            char* end = nullptr;
            double temp = strtod(val.c_str(), &end);
            if (end == val.c_str() || *end != '\0')
            {
                myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Parameter " + val + " cannot be converted to double type");
                return false;
            }
            currParam = temp;
        }
        else
        {
            myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unsupported type " + std::string(typeid(currParam).name()) + " passed into command");
            return false;
        }

        // There are more parameters to unpack
        return validateParameters(remainder, outParams...);
    }

    std::queue<std::string> myCommandQueue;
    std::thread myInputWaitingThread;
    std::shared_ptr<std::atomic<bool>> myExitingSignal;

    std::weak_ptr<IInformationDisplay> myInformationDisplay;
    std::weak_ptr<IOpticsManager> myOpticsManager;
    std::weak_ptr<IPositionManager> myPositionManager;
    std::weak_ptr<IStarTracker> myStarTracker;
};

#endif