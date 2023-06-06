//
//  Buggy.h
//  wakuseibokan
//
//  Created by Rodrigo Ramele on 6/26/15.
//

#ifndef BUGGY_H
#define BUGGY_H


#include "Vehicle.h"
#include "Wheel.h"

class Buggy : public Vehicle
{
    dGeomID geom;

protected:
    float height;
    float width;
    float length;

    Wheel *left;
    Wheel *right;

    Wheel *backleft;
    Wheel *backright;
    
public:
    Buggy(int faction);
    void virtual init();
    int  virtual getType();
    void virtual drawModel(float yRot, float xRot, float x, float y, float z);
    void virtual drawModel();
    void virtual doDynamics(dBodyID);
    void doDynamics();
    void doControl(Controller controller);
    void  doControl();
    void  doControl(struct controlregister conts);
    
    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    void addWheels(Wheel *left, Wheel *right, Wheel *backleft, Wheel *backright);

    EntityTypeId virtual getTypeId();
};

#endif //BUGGY_H
