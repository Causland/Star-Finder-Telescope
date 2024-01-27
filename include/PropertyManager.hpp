#ifndef PROPERTY_MANAGER_HPP
#define PROPERTY_MANAGER_HPP

#include <istream>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

#include "Logger.hpp"

/*!
 * The PropertyManager class stores all properties defined in the properties file for use by
 * various subsystems around the application. The properties are defined using a key value pair
 * stored in a toml format. Properties must be defined in the properties file before starting
 * the program.
 */
class PropertyManager
{
public:
   using Property = std::variant<std::string, int64_t, double, bool>;

   /*!
    * Initializes the property manager with a stream containing toml data.
    * The stream must be a toml formatted series of key/value pairs. The properties
    * are stored in a container and can be a string, int, double, or bool.
    * Properties defined outside one of these options are rejected and an error is reported.
    * \param[in] propStream a stream with the toml formatted properties.
    * \return true if the property manager is initialized succesfully.
    */
   static bool initialize(std::istream& propStream);

   /*!
    * Terminates the property manager by clearing all existing properties and resetting the
    * initialized flag.
    */
   static void terminate();

   /*!
    * Get the value of the property with the provided type. Fails if the provided
    * type does not meet the type of the stored property.
    * \return true if the property is found and assigned to value.
    */
   template <typename OutType>
   inline static bool getProperty(std::string_view propName, OutType* value)
   {
      std::scoped_lock<std::mutex> lock{thePropMutex};

      if (theInitializedFlag)
      {
         const auto iter{theProps.find(std::string{propName})};
         if (iter != theProps.end())
         {
            if (std::holds_alternative<OutType>(iter->second))
            {
               *value = std::get<OutType>(iter->second);
               return true;
            }
            
            LOG_WARN("Unable to find property: " + std::string{propName});
         }
      }
      else 
      {
         LOG_ERROR("Property Manager is uninitialized");
      }

      return false;
   }

   /*!
    * Get a formatted string of all loaded properties.
    * \return the formatted string.
    */
   [[nodiscard]] static std::string toStringAllProperties();

private:
   // NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
   static std::mutex thePropMutex; //!< Protects initialized flag and properties container from multiple access.
   static bool theInitializedFlag; //!< Indicates that the property manager is initialized.
   static std::unordered_map<std::string, Property> theProps; // Container to hold all properties.
   // NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
};

#endif
