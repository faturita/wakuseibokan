//
//  MultiBodyVehicle.hpp
//  wakuseibokan
//
//  Created by Rodrigo Ramele on 1/12/17.
//

#ifndef MultiBodyVehicle_h
#define MultiBodyVehicle_h

#include <stdio.h>

#include "Vehicle.h"


#define CMASS 1		// chassis mass
#define WMASS 0.2	// wheel mass


class MultiBodyVehicle : public Vehicle
{
    GLuint _textureBox;
    
    
    // dynamics and collision objects (chassis, 3 wheels, environment)
    dBodyID carbody[5];
    dJointID carjoint[4];	// joint[0] is the front wheel
    dSpaceID car_space;
    dGeomID box[1];
    dGeomID sphere[4];
    dGeomID ground_box;
    
public:
    void virtual init();
    int  virtual getType();
    EntityTypeId virtual getTypeId();
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

#endif /* MultiBodyVehicle_hpp */
