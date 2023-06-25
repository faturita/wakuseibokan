#ifndef SIMPLIFIEDMANTA_H
#define SIMPLIFIEDMANTA_H

#include "Manta.h"

class SimplifiedManta : public Manta
{
public:
    float angularPos[3] = {0,0,0};
    void doDynamics(dBodyID body);
    void doControl(Controller controller);
    EntityTypeId virtual getTypeId();
};

#endif // SIMPLIFIEDMANTA_H
