/*
 * yamathutil.h
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#ifndef YAMATHUTIL_H_
#define YAMATHUTIL_H_

#include <math.h>
#include "vec3f.h"

#define PI 3.1415926536

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

// constants to convert degrees to radians and the reverse
#define RAD_TO_DEG (180.0/M_PI)
#define DEG_TO_RAD (M_PI/180.0)

Vec3f toVectorInFixedSystem(float dx, float dy, float dz,float yAngle, float xAngle);
void UnitarizeNormal(float vector[3]);
void Normalize(float v[3][3], float out[3][3]);
float getAzimuth(Vec3f aim);
float getInclination(Vec3f aim);

#endif /* YAMATHUTIL_H_ */
