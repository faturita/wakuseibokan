//
//  Buggy.h
//  wakuseibokan
//
//  Created by Rodrigo Ramele on 6/26/15.
//  Copyright (c) 2015 Baufest. All rights reserved.
//

#ifndef __wakuseibokan__Buggy__
#define __wakuseibokan__Buggy__

#include "Vehicle.h"

class Buggy : public Vehicle
{
    dGeomID geom;
    
public:
    void virtual init();
    int  virtual getType();
    void virtual drawModel(float yRot, float xRot, float x, float y, float z);
    void virtual drawModel();
    void virtual drawDirectModel();
    void doMaterial();
    void virtual doDynamics(dBodyID);
    void doDynamics();
    void doControl(Controller controller);
    
    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);
};

#endif /* defined(__wakuseibokan__Buggy__) */
