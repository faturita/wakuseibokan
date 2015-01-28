/*
 * Walrus.cpp
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#include "Walrus.h"

void Walrus::init()
{
    _model = MD2Model::load("walrus.md2");
    if (_model != NULL)
        _model->setAnimation("run");
    else
    	printf ("Model has been initialized");

    setForward(0,0,1);

}

void Walrus::doMaterial()
{
    GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f};

    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS,128);
}


void Walrus::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        doTransform(f, R);
        
        drawArrow();

       	glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        //glRotatef(yRot, 0.0f, 1.0f, 0.0f);

        //glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        //doMaterial();

        _model->draw();

        glPopMatrix();
    }
    else
    {
    	printf ("model is null\n");
    }
}

void Walrus::drawModel()
{
	drawModel(0,0,pos[0],pos[1],pos[2]);
}

void Walrus::doControl(Controller controller)
{
    //engine[0] = -controller.roll;
    //engine[1] = controller.yaw;
    //engine[2] = -controller.pitch;
    //steering = -controller.precesion;
    
    
    setThrottle(-controller.pitch);
    
    xRotAngle = -controller.precesion;
    
}


void Walrus::drawDirectModel()
{
    float modX=0.0f, modY=15.0f, modZ=0.0f;

    modX = pos[0];
    modY = pos[1];
    modZ = pos[2];

    Vec3f forward = toVectorInFixedSystem(0, 0, -0.1,-xRotAngle,yRotAngle);

    modX+=speed*forward[0]; modY+=speed*forward[1];modZ+=speed*forward[2];

    pos += speed * forward;

    drawModel(xRotAngle, yRotAngle, modX, modY, modZ);
}

void Walrus::doDynamics()
{
    doDynamics(getBodyID());
}

void Walrus::doDynamics(dBodyID body)
{
	dReal *v = (dReal *)dBodyGetLinearVel(body);

	dVector3 O;
	dBodyGetRelPointPos( body, 0,0,0, O);

	dVector3 F;
	dBodyGetRelPointPos( body, 0,0,1, F);

	F[0] = (F[0]-O[0]);
	F[1] = (F[1]-O[1]);
	F[2] = (F[2]-O[2]);


	Vec3f vec3fF;
	vec3fF[0] = F[0];vec3fF[1] = F[1]; vec3fF[2] = F[2];

	Vec3f vec3fV;
	vec3fV[0]= v[0];vec3fV[1] = v[1]; vec3fV[2] = v[2];

	speed = vec3fV.magnitude();

	Vec3f dump;
	if (vec3fV.magnitude() != 0 && vec3fF.magnitude() != 0)
	{
		dump = - ((vec3fV.cross(vec3fF).magnitude())/(vec3fV.magnitude()*vec3fF.magnitude())*10.0f) * vec3fV - 0.001 * vec3fV;
	}

	if (!isnan(dump[0]) && !isnan(dump[1]) && !isnan(dump[2]))
	{
		dBodyAddForce(body, dump[0], dump[1], dump[2]);
	}


	//dBodyAddForce(body[i],damping[0]*-dumpMedia[i][0],damping[1]*-dumpMedia[i][0],damping[2]*-dumpMedia[i][0]);

	dReal *angulardumping = (dReal *)dBodyGetAngularVel(body);


	dBodyAddTorque(body,angulardumping[0]*-0.1,angulardumping[1]*-0.1,angulardumping[2]*-0.1 );




	// Walrus
	dBodyAddRelForce (body,0, 0,getThrottle());
	dBodyAddRelTorque( body, 0, -xRotAngle*0.001,0 );
	//modAngleX=0;
	//dJointAddHingeTorque(joint1,spd);
	//dJointAddHingeTorque(joint2,spd);


	// This should be after the world step
	/// stuff
    dVector3 result;

    dBodyVectorToWorld(body, 0,0,1,result);
    setForward(result[0],result[1],result[2]);

	const dReal *dBodyPosition = dBodyGetPosition(body);
	const dReal *dBodyRotation = dBodyGetRotation(body);

	setPos(dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);
	setLocation((float *)dBodyPosition, (float *)dBodyRotation);


}
