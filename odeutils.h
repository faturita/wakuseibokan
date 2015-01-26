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


Vec3f ToVec3f(dReal *val, Vec3f retValue);

Vec3f dBodyVectorToWorldVec3f(dBodyID body,dReal x,dReal y, dReal z,Vec3f vForward);
#endif /* ODEUTILS_H_ */
