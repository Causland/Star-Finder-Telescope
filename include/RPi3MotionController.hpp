#ifndef RPI3_MOTION_CONTROLLER_HPP
#define RPI3_MOTION_CONTROLLER_HPP

#include "interfaces/IMotionController.hpp"
#include <fstream>
#include <iostream>

class RPi3MotionController : public IMotionController
{
public:
    RPi3MotionController();
    ~RPi3MotionController();

    // Includes from IMotionController
    void moveFocusKnob(double theta) override;
    void moveHorizAngle(double theta) override;
    void moveVertAngle(double phi) override;

private:
    std::string generateServoblasterFormat(const uint8_t& pinNum, const uint16_t& posInUs);

    std::ofstream myServoControlStream;
    const std::string myServoControlFilePath{"/dev/servoblaster"};
};

#endif