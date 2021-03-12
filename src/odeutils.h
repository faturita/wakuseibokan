/*
 * odeutils.h
 *
 *  Created on: May 10, 2011
 *      Author: faturita
 */

#ifndef ODEUTILS_H_
#define ODEUTILS_H_

#include <ode/ode.h>

#include "math/vec3f.h"
#include "math/yamathutil.h"

#define SGN(x) (x>=0?+1:-1)

#define runonce static bool ond = true; for (;ond;ond = false)
#define runonceinclass for (;ond;ond = false)

Vec3f ToVec3f(dReal *val, Vec3f retValue);

Vec3f dBodyVectorToWorldVec3f(dBodyID body,dReal x,dReal y, dReal z,Vec3f vForward);
#endif /* ODEUTILS_H_ */
