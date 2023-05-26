#ifndef OTTER_H
#define OTTER_H

#include "Wheel.h"
#include "AdvancedWalrus.h"

class Otter : public AdvancedWalrus
{

protected:
    bool fly = false;
    Wheel *left;
    Wheel *right;

    Wheel *backleft;
    Wheel *backright;

public:
    Otter(int newfaction);
    void init();
    void addWheels(Wheel *left, Wheel *right, Wheel *backleft, Wheel *backright);
    void getWheels(Wheel* &left, Wheel* &right, Wheel* &backleft, Wheel* &backright );
    void doControl(Controller controller);

    void doDynamics();
    void doDynamics(dBodyID body);

    void embody(dBodyID myBodySelf);
    void embody(dWorldID world, dSpaceID space);

    dSpaceID embody_in_space(dWorldID world, dSpaceID space);

    void drawModel(float yRot, float xRot, float x, float y, float z);

    int  virtual getSubType();
    EntityTypeId virtual getTypeId();
};

#endif // OTTER_H
