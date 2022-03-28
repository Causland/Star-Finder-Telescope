#ifndef STAR_DATABASE_HPP
#define STAR_DATABASE_HPP

class StarDatabase
{
    // Things for database interface to do
    // - Formulate API query for specific star information
    // - Provide this information to the caller
    // - Create database cache of information to use on future requests
public:
    void queryTargetPosition(std::string targetName);
    void queryTargetsWithinRange(double rangeInLightMinutes);
};

#endif