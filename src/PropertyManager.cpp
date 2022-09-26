#include "Logger.hpp"
#include "PropertyManager.hpp"
#include <iostream>
#include <sstream>
#include <toml++/toml.h>

std::atomic<bool> PropertyManager::theInitializedFlag{false};
std::map<std::string, std::string> PropertyManager::theStringProps{};
std::map<std::string, int64_t> PropertyManager::theIntProps{};
std::map<std::string, double> PropertyManager::theDoubleProps{};
std::map<std::string, bool> PropertyManager::theBoolProps{};

bool PropertyManager::initialize(const std::string& propFilePath)
{
   // Read in the toml properties file and filter into
   // a the correct std::map
   toml::table propsTable;
   try
   {
      propsTable = toml::parse_file(propFilePath);
   }
   catch(const toml::parse_error& e)
   {
      Logger::log("PropManager", LogCodeEnum::ERROR, "Unable to parse properties file");
      return false;
   }
   auto initializeSuccess = true;
   propsTable.for_each([&initializeSuccess](const toml::key& key, auto&& val)
   {
      if constexpr (toml::is_string<decltype(val)>)
      {
         theStringProps.insert({std::string(key.str()), val.get()}); // val is a toml::value<std::string>
      }
      else if constexpr (toml::is_integer<decltype(val)>)
      {
         theIntProps.insert({std::string(key.str()), val.get()}); // val is a toml::value<int64_t>
      }
      else if constexpr (toml::is_floating_point<decltype(val)>)
      {
         theDoubleProps.insert({std::string(key.str()), val.get()}); // val is a toml::value<double>
      }
      else if constexpr (toml::is_boolean<decltype(val)>)
      {
         theBoolProps.insert({std::string(key.str()), val.get()}); // val is a toml::value<boolean>
      }
      else
      {
         // val is not a supported property type
         std::stringstream ss;
         ss << "Parsed value not supported (" << key << "," << val 
            << ") Supported types are string, int, double, bool\n"; 
         Logger::log("PropManager", LogCodeEnum::ERROR, ss.str());

         initializeSuccess = false;
      }
   });

   if (initializeSuccess)
   {
      theInitializedFlag = true;
      return true;
   }
   return false;
}

void PropertyManager::terminate()
{
   // Clear out the properties
   theStringProps.clear();
   theIntProps.clear();
   theDoubleProps.clear();
   theBoolProps.clear();
   theInitializedFlag = false;
}

bool PropertyManager::getProperty(const std::string& propName, std::string* value)
{
   if (theInitializedFlag)
   {
      auto it = theStringProps.find(propName);
      if (it != theStringProps.end())
      {
         *value = it->second;
         return true;
      }
      else
      {
         Logger::log("PropManager", LogCodeEnum::WARNING, "Unable to find property: " + propName);
      }
   }
   else
   {
      Logger::log("PropManager", LogCodeEnum::ERROR, "Property Manager is uninitialized. No properties are loaded");
   }
   return false;
}

bool PropertyManager::getProperty(const std::string& propName, int64_t* value)
{
   if (theInitializedFlag)
   {
      auto it = theIntProps.find(propName);
      if (it != theIntProps.end())
      {
         *value = it->second;
         return true;
      }
      else
      {
         Logger::log("PropManager", LogCodeEnum::WARNING, "Unable to find property: " + propName);
      }
   }
   else
   {
      Logger::log("PropManager", LogCodeEnum::ERROR, "Property Manager is uninitialized. No properties are loaded");
   }
   return false;
}

bool PropertyManager::getProperty(const std::string& propName, double* value)
{
   if (theInitializedFlag)
   {
      auto it = theDoubleProps.find(propName);
      if (it != theDoubleProps.end())
      {
         *value = it->second;
         return true;
      }
      else
      {
         Logger::log("PropManager", LogCodeEnum::WARNING, "Unable to find property: " + propName);
      }
   }
   else
   {
      Logger::log("PropManager", LogCodeEnum::ERROR, "Property Manager is uninitialized. No properties are loaded");
   }
   return false;
}

bool PropertyManager::getProperty(const std::string& propName, bool* value)
{
   if (theInitializedFlag)
   {
      auto it = theBoolProps.find(propName);
      if (it != theBoolProps.end())
      {
         *value = it->second;
         return true;
      }
      else
      {
         Logger::log("PropManager", LogCodeEnum::WARNING, "Unable to find property: " + propName);
      }
   }
   else
   {
      Logger::log("PropManager", LogCodeEnum::ERROR, "Property Manager is uninitialized. No properties are loaded");
   }
   return false;
}

std::string PropertyManager::toStringAllProperties()
{
   std::stringstream ss;
   ss << "Properties (name, value)\n";
   for (const auto& [key, val] : theStringProps)
   {
      ss << "(" << key << ",\"" << val << "\")\n";
   }
   for (const auto& [key, val] : theIntProps)
   {
      ss << "(" << key << "," << val << ")\n";
   }
   for (const auto& [key, val] : theDoubleProps)
   {
      ss << "(" << key << "," << val << ")\n";
   }
   for (const auto& [key, val] : theBoolProps)
   {
      auto outVal = val ? "true" : "false";
      ss << "(" << key << "," << outVal << ")\n";
   }
   return ss.str();
}