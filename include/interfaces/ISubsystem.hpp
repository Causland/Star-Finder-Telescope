#ifndef I_SUBSYSTEM_HPP
#define I_SUBSYSTEM_HPP

class ISubsystem
{
public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void configureInterfaces() = 0;
    virtual bool checkHeartbeat() = 0;
    virtual void threadLoop() = 0;
};

#endif