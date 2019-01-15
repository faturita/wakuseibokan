#ifndef ISLAND_H
#define ISLAND_H

#include <iostream>

class Island
{
public:
    virtual std::string getName() = 0;
    virtual void tick()=0;
};

#endif // ISLAND_H
