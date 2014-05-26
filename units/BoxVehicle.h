//
//  BoxVehicle.h
//  mycarrier
//
//  Created by Rodrigo Ramele on 24/05/14.
//  Copyright (c) 2014 Baufest. All rights reserved.
//

#ifndef __mycarrier__BoxVehicle__
#define __mycarrier__BoxVehicle__

#include <iostream>

#include "Vehicle.h"

class BoxVehicle : public Vehicle
{
    GLuint _textureBox;
    float boxRotatingAngle;
    dBodyID me;
    bool tweakOde = true;
    
public:
	void virtual init();
	void virtual drawModel(float yRot, float xRot, float x, float y, float z);
	void virtual drawModel();
	void virtual drawDirectModel();
	void doMaterial();
	void virtual doDynamics(dBodyID);
    void virtual doDynamics();
    void virtual embody(dBodyID);
    void virtual doControl(Controller);
    dBodyID virtual getBodyID();
    void virtual stop();
};


#endif /* defined(__mycarrier__BoxVehicle__) */
