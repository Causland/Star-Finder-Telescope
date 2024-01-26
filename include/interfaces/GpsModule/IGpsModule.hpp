#ifndef I_GPS_MODULE_HPP
#define I_GPS_MODULE_HPP

#include <string>

/*
 * Stores information about a GPS position.
 */
struct GpsPosition
{
   double myLatitude{0.0};
   double myLongitude{0.0};
   double myElevation{0.0};
};

/*
 * A GPS Module is responsible for obtaining and reporting GPS positioning data
 * when requested using its interface functions.
 */
class IGpsModule
{
public:
   /*!
    * Constructs an IGpsModule.
    */
   IGpsModule() = default;

   /*!
    * Destroys an IGpsModule.
    */ 
   virtual ~IGpsModule() = default;
   
   IGpsModule(const IGpsModule&) = delete;
   IGpsModule& operator=(const IGpsModule&) = delete;
   IGpsModule(IGpsModule&&) = delete;
   IGpsModule& operator=(IGpsModule&&) = delete;

   /*!
    * Get the GPS position reported by the module.
    * \param[out] position the GPS position data.
    * \return true if the position is known.
    */ 
   virtual bool getGpsPosition(GpsPosition* position) = 0;

   /*!
    * Get the GPS position as a formatted string.
    * \return a formatted string with the GPS position.
    */ 
   virtual std::string getDisplayInfo() = 0;
};

#endif
