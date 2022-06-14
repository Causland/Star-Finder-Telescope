#include "StarDatabaseAdapter.hpp"

StarDatabaseAdapter::StarDatabaseAdapter() : myDatabaseConnection(sqlpp::sqlite3::connection_config(myDatabasePath))
{
}

StarDatabaseAdapter::~StarDatabaseAdapter()
{
}

void StarDatabaseAdapter::queryTargetPosition(std::string targetName)
{
   using sqlpp::select;
   using sqlpp::from;
   using sqlpp::where;
   using StarDatabaseSchema::TargetBody;
   using StarDatabaseSchema::Ephemeris;
   TargetBody tb;
   auto result = myDatabaseConnection(select(tb.bodyId, tb.bodyName)
                                     .from(tb)
                                     .where(tb.bodyName == targetName));
}

void StarDatabaseAdapter::queryTargetsWithinRange(double rangeInLightMinutes)
{
   
}