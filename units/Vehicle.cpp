/*
 * Vehicle.cpp
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#include "Vehicle.h"

Vehicle::Vehicle()
{
	for(int i=0;i<12;i++)
	{
		R[i]=0;
	}
	R[0]=R[5]=R[10]=1;
}

void Vehicle::setXRotAngle(float xRotAngle)
{
	Vehicle::xRotAngle = xRotAngle;
}
void Vehicle::setYRotAngle(float yRotAngle)
{
	Vehicle::yRotAngle = yRotAngle;
}
float Vehicle::getSpeed()
{
	return speed;
}
void Vehicle::setSpeed(float speed)
{
	Vehicle::speed=speed;
}
void Vehicle::setPos(const Vec3f &newpos)
{
	pos[0] = newpos[0];
	pos[1] = newpos[1];
	pos[2] = newpos[2];
}
void Vehicle::setPos(float x, float y, float z)
{
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;
}
Vec3f Vehicle::getPos()
{
	return pos;
}

void Vehicle::setForward(float x, float y, float z)
{
	forward[0]=x;
	forward[1]=y;
	forward[2]=z;
}

Vec3f Vehicle::getForward()
{
	return forward;
}

void Vehicle::setLocation(float fPos[3], float R[12])
{
	pos[0] = fPos[0];
	pos[1] = fPos[1];
	pos[2] = fPos[2];

	for(int i=0; i<12; i++)
	{
		Vehicle::R[i] =  R[i];
	}
}

void Vehicle::init()
{
	//Load the model
	_model = MD2Model::load("manta.md2");
    if (_model != NULL)
        _model->setAnimation("run");

}

void  Vehicle::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
	position = getPos();
	forward = getForward();
	Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

	Vec3f orig;


	forward = forward.normalize();
	orig = position;
	Up[0]=Up[2]=0;Up[1]=4;
	position = position - 20*forward + Up;
	forward = orig-position;
}

void Vehicle::drawModel(float yRot, float xRot, float x, float y, float z)
{
    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);
		glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(-180.0f, 0.0f, 0.0f, 1.0f);

		glRotatef(yRot, 0.0f, 0.0f, 1.0f);

        glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        _model->draw();
        drawArrow();
        glPopMatrix();
    }
    else
    {
    	printf ("model is null\n");
    }
}

void Vehicle::doDynamics(dBodyID body)
{

}

void Vehicle::drawModel()
{
    float modX=0.0f, modY=0.0f, modZ=0.0f;

    modX = pos[0];
    modY = pos[1];
    modZ = pos[2];

    Vec3f forward = toVectorInFixedSystem(0, 0, -0.1,-xRotAngle,yRotAngle);

    modX+=speed*forward[0]; modY+=speed*forward[1];modZ+=speed*forward[2];

    pos += speed * forward;

    drawModel(xRotAngle, yRotAngle, modX, modY, modZ);
}

void Vehicle::setThrottle(float throttle)
{
	Vehicle::throttle = throttle;
}

float Vehicle::getThrottle()
{
	return Vehicle::throttle;
}

