#ifndef ACTIONS_H
#define ACTIONS_H

#include "../math/yamathutil.h"
#include "../openglutils.h"
#include "../odeutils.h"

class Action
{
public:
    void virtual fire()=0;

    void virtual setOrigin(dBodyID whofired) = 0;
    virtual dBodyID getOrigin() = 0;
};

#endif // ACTIONS_H




