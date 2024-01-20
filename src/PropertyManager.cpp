#include "Logger.hpp"
#include "PropertyManager.hpp"

#include <sstream>
#include <toml++/toml.h>

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
std::mutex PropertyManager::thePropMutex;
bool PropertyManager::theInitializedFlag{false};
std::unordered_map<std::string, PropertyManager::Property> PropertyManager::theProps;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

bool PropertyManager::initialize(std::istream& propStream)
{
   std::scoped_lock<std::mutex> lock{thePropMutex};

   // Read in the toml properties file and place into properties container
   try 
   {
      theInitializedFlag = true;

      auto parsedProps{toml::parse(propStream)};
      parsedProps.for_each([](const auto& key, auto&& val)
      {
         if constexpr (toml::is_string<decltype(val)>)
         {
            theProps.insert({std::string{key.str()}, 
                             Property{std::in_place_type<std::string>, val.get()}});
         }
         else if constexpr (toml::is_integer<decltype(val)>)
         {
            theProps.insert({std::string{key.str()},
                             Property{std::in_place_type<int64_t>, val.get()}});
         }
         else if constexpr (toml::is_floating_point<decltype(val)>)
         {
            theProps.insert({std::string{key.str()},
                             Property{std::in_place_type<double>, val.get()}});
         }
         else if constexpr (toml::is_boolean<decltype(val)>)
         {
            theProps.insert({std::string{key.str()},
                             Property{std::in_place_type<bool>, val.get()}});
         }
         else
         {
            // val is not a supported property type
            std::ostringstream oss;
            oss << "Parsed value not supported (" << key << "," << val 
               << ") Supported types are string, int, double, bool\n"; 
            LOG_ERROR(oss.str());
            theInitializedFlag = true;
         }
      });
   } 
   catch(const toml::parse_error& e)
   {
      LOG_ERROR("Unable to parse properties: " + std::string{e.what()});
      theInitializedFlag = false;
   }

   return theInitializedFlag;
}

void PropertyManager::terminate()
{
   std::scoped_lock<std::mutex> lock{thePropMutex};
   theProps.clear();
   theInitializedFlag = false;
}

std::string PropertyManager::toStringAllProperties()
{
   struct MakeStringFunctor
   {
      std::string operator()(const std::string& val) const { return val; }
      std::string operator()(const int64_t& val) const { return std::to_string(val); }
      std::string operator()(const double& val) const { return std::to_string(val); }
      std::string operator()(const bool val) const {return val ? "true" : "false"; }
   };

   std::scoped_lock<std::mutex> lock{thePropMutex};
   std::ostringstream oss;
   oss << "Properties (name, value)\n";
   for (const auto& [key, variant] : theProps) // cppcheck-suppress unassignedVariable
   {
      oss << "(" << key << ",\"" << std::visit(MakeStringFunctor(), variant) << "\")\n";
   }
   return oss.str();
}
