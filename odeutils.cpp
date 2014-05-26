/*
 * odeutils.cpp
 *
 *  Created on: May 10, 2011
 *      Author: faturita
 */

#include <ode/ode.h>
#include "odeutils.h"

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
