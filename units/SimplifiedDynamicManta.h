#ifndef SIMPLIFIEDDYNAMICMANTA_H
#define SIMPLIFIEDDYNAMICMANTA_H

#include "Manta.h"

class SimplifiedDynamicManta : public Manta
{
public:
    float angularPos[3] = {0,0,0};
    void doDynamics(dBodyID body);
    void doControl(Controller controller);
    void embody(dBodyID myBodySelf);
    void embody(dWorldID world, dSpaceID space);


    Vehicle* fire(dWorldID world, dSpaceID space);
};

#endif // SIMPLIFIEDMANTA_H
