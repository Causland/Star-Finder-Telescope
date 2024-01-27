#ifndef SIM_GPSMODULE_HPP
#define SIM_GPSMODULE_HPP

#include "interfaces/GpsModule/IGpsModule.hpp"

static constexpr double SIM_LATITUDE{28.4186};
static constexpr double SIM_LONGITUDE{-81.5812};
static constexpr double SIM_ELEVATION{15};

/*
 * This is a Simulated GPS Module. It reports a constant GPS position.
 */
class SimGpsModule : public IGpsModule
{
public:
   /*!
    * Creates a SimGpsModule.
    */
   SimGpsModule() = default;

   /*!
    * Destroys a SimGpsModule.
    */ 
   ~SimGpsModule() override = default;

   SimGpsModule(const SimGpsModule&) = delete;
   SimGpsModule& operator=(const SimGpsModule&) = delete;
   SimGpsModule(SimGpsModule&&) = delete;
   SimGpsModule& operator=(SimGpsModule&&) = delete;

   /*!
    * Get the GPS position reported by the module.
    * \param[out] position the GPS position data.
    * \return true if the position is known.
    */ 
   [[nodiscard]] bool getGpsPosition(GpsPosition* position) override;

   /*!
    * Get the GPS position as a formatted string.
    * \return a formatted string with the GPS position.
    */ 
   [[nodiscard]] std::string getDisplayInfo() override;

private:
   GpsPosition myGpsPosition{SIM_LATITUDE, SIM_LONGITUDE, SIM_ELEVATION};
};

#endif
