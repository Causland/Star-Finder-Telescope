#ifndef PROPERTY_MANAGER_HPP
#define PROPERTY_MANAGER_HPP

#include <atomic>
#include <map>
#include <string>

class PropertyManager
{
public:
   static bool initialize(const std::string& propFilePath);

   static void terminate();

   static bool getProperty(const std::string& propName, std::string* value);
   static bool getProperty(const std::string& propName, int64_t* value);
   static bool getProperty(const std::string& propName, double* value);
   static bool getProperty(const std::string& propName, bool* value);

   static std::string toStringAllProperties();

private:
   static std::atomic<bool> theInitializedFlag;
   static std::map<std::string, std::string> theStringProps;
   static std::map<std::string, int64_t> theIntProps;
   static std::map<std::string, double> theDoubleProps;
   static std::map<std::string, bool> theBoolProps;
};

#endif