#ifndef PROPERTY_MANAGER_HPP
#define PROPERTY_MANAGER_HPP

#include <atomic>
#include <map>
#include <string>

/*!
 * The PropertyManager class stores all properties defined in the properties file for use by
 * various subsystems around the application. The properties are defined using a key value pair
 * stored in a toml format. Properties must be defined in the properties file before starting
 * the program.
 */
class PropertyManager
{
public:
   /*!
    * Initializes the property manager with a properties file defined at the provided path.
    * The properties file must be a toml formatted series of key/value pairs. The properties
    * are stored in one of four map containers for strings, doubles, integers, and booleans.
    * Properties defined outside one of these options are rejected and an error is reported.
    * \param[in] propFilePath a path to the toml formatted property file.
    * \return true if the property manager is completely initialized.
    */
   static bool initialize(const std::string& propFilePath);

   /*!
    * Terminates the property manager by clearing all existing properties and resetting the
    * initialized flag.
    */
   static void terminate();

   /*!
    * Get the string value of the property with the provided name.
    * \param[in] propName the name of the property.
    * \param[out] value a pointer to store the string value.
    * \return true if the property is found.
    */
   static bool getProperty(const std::string& propName, std::string* value);

   /*!
    * Get the int64 value of the property with the provided name.
    * \param[in] propName the name of the property.
    * \param[out] value a pointer to store the int64 value.
    * \return true if the property is found.
    */
   static bool getProperty(const std::string& propName, int64_t* value);

   /*!
    * Get the double value of the property with the provided name.
    * \param[in] propName the name of the property.
    * \param[out] value a pointer to store the double value.
    * \return true if the property is found.
    */
   static bool getProperty(const std::string& propName, double* value);

   /*!
    * Get the boolean value of the property with the provided name.
    * \param[in] propName the name of the property.
    * \param[out] value a pointer to store the boolean value.
    * \return true if the property is found.
    */
   static bool getProperty(const std::string& propName, bool* value);

   /*!
    * Generate a string containing all the property names and values formatted.
    * \return a formatted string with all properties.
    */
   static std::string toStringAllProperties();

private:
   static std::atomic<bool> theInitializedFlag; //!< Indicates that the property manager is initialized.
   static std::map<std::string, std::string> theStringProps; //!< Map to hold all string valued properties.
   static std::map<std::string, int64_t> theIntProps; //!< Map to hold all integer valued properties.
   static std::map<std::string, double> theDoubleProps; //!< Map to hold all double valued properties.
   static std::map<std::string, bool> theBoolProps; //!< Map to hold all boolean valued properties.
};

#endif