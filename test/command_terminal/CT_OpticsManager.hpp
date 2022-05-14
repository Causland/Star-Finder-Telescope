#include "MinOpticsManager.hpp"

class CT_OpticsManager : public MinOpticsManager
{
public:
   std::string takePhoto(const CmdTakePhoto& cmd) override 
   {
      return ""; 
   }
   std::string takeVideo(const CmdTakeVideo& cmd) override 
   {
      return ""; 
   } 
   std::string takeTimelapse(const CmdTakeTimelapse& cmd) override 
   {
      return ""; 
   }
};