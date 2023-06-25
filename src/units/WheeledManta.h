#ifndef WHEELEDMANTA_H
#define WHEELEDMANTA_H

#include "Wheel.h"
#include "AdvancedManta.h"

class WheeledManta : public AdvancedManta
{

protected:
    bool fly = false;
    Wheel *left;
    Wheel *right;

    Wheel *backleft;
    Wheel *backright;
public:
    WheeledManta(int newfaction);
    void addWheels(Wheel *left, Wheel *right, Wheel *backleft, Wheel *backright);
    void doControl(Controller controller);

    void doDynamics();
    void doDynamics(dBodyID body);

    EntityTypeId virtual getTypeId();
};


#endif // WHEELEDMANTA_H
