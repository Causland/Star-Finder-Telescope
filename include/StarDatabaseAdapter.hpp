#ifndef STAR_DATABASE_HPP
#define STAR_DATABASE_HPP

#include "DatabaseSchema.hpp"
#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/sqlite3/sqlite3.h"
#include <string>

class StarDatabaseAdapter
{
public:
    StarDatabaseAdapter();
    ~StarDatabaseAdapter();
    void queryTargetPosition(std::string targetName);
    void queryTargetsWithinRange(double rangeInLightMinutes);
    
private:
    std::string myDatabasePath;
    sqlpp::sqlite3::connection myDatabaseConnection;
};

#endif