/*
 * odeutils.cpp
 *
 *  Created on: May 10, 2011
 *      Author: faturita
 */

#define dSINGLE

#include <ode/ode.h>
#include "odeutils.h"

void firesound(int times)
{
    for(int i=0;i<times;i++)
        printf ("%c", 7);
    
}

void enginestart()
{
    system("afplay sounds/cruise.m4a &");
}

void takeoff()
{
    system("afplay sounds/takeoff.mp3 &");
}

void explosion()
{
    system("afplay sounds/explosion.mp3 &");
}

void coast()
{
    system("afplay sounds/Coast.m4a &");
}

void honk()
{
    system("afplay sounds/BoatHonk.m4a &");
}

Vec3f ToVec3f(dReal *val, Vec3f retValue)
{
	retValue[0]=val[0];
	retValue[1]=val[1];
	retValue[2]=val[2];

	return retValue;
}


Vec3f dBodyVectorToWorldVec3f(dBodyID body,dReal x,dReal y, dReal z,Vec3f vForward)
{
    dVector3 result;
    dBodyVectorToWorld(body,x,y,z,result);

    vForward = ToVec3f(result, vForward);

    return vForward;
}
