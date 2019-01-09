#ifndef ACTIONS_H
#define ACTIONS_H

#include "math/yamathutil.h"
#include "openglutils.h"

class Action
{
public:
    void virtual fire()=0;
};

#endif // ACTIONS_H




