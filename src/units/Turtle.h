#ifndef TURTLE_H
#define TURTLE_H

#include "Wheel.h"
#include "AdvancedWalrus.h"

class Turtle : public AdvancedWalrus
{

protected:
    bool fly = false;
    Wheel *left=NULL;
    Wheel *right=NULL;

    Wheel *backleft=NULL;
    Wheel *backright=NULL;

public:
    Turtle(int newfaction);
    void init();
    void addWheels(Wheel *left, Wheel *right, Wheel *backleft, Wheel *backright);
    void getWheels(Wheel* &left, Wheel* &right, Wheel* &backleft, Wheel* &backright );
    void doControl(Controller controller);

    void virtual setNameByNumber(int number);

    void doDynamics();
    void doDynamics(dBodyID body);

    void embody(dBodyID myBodySelf);
    void embody(dWorldID world, dSpaceID space);

    dSpaceID embody_in_space(dWorldID world, dSpaceID space);

    void drawModel(float yRot, float xRot, float x, float y, float z);

    int  virtual getSubType();
    EntityTypeId virtual getTypeId();
};

#endif // TURTLE_H
