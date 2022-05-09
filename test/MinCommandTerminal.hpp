#include "interfaces/ICommandTerminal.hpp"

const std::string ICommandTerminal::NAME{"SimCommandTerminal"};

class MinCommandTerminal : public ICommandTerminal
{
};