#ifndef STAR_DATABASE_HPP
#define STAR_DATABASE_HPP

#include "interfaces/IStarDatabase.hpp"

class StarDatabase : public IStarDatabase
{
    // Things for database interface to do
    // - Formulate API query for specific star information
    // - Provide this information to the caller
    // - Create database cache of information to use on future requests
};

#endif