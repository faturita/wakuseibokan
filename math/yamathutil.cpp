/*
 * yamathutil.cpp
 *
 *  Created on: Jan 11, 2011
 *      Author: faturita
 */

#include "yamathutil.h"

/**
 * Of course in opengl util functions, a normalization function could not be missing
 *
 */

void UnitarizeNormal(float vector[3])
{
	float length;
	length = (float)sqrt (
			(vector[0]*vector[0]) +
			(vector[1]*vector[1]) +
			(vector[2]*vector[2])
			);

	if (length == 0.0f)
		length = 1.0f;

	vector[0] /= length;
	vector[1] /= length;
	vector[2] /= length;
}

void Normalize(float v[3][3], float out[3])
{
	float v1[3], v2[3];
	static const int x=0;
	static const int y=1;
	static const int z=2;

	v1[x] = v[0][x] - v[1][x];
	v1[y] = v[0][y] - v[1][y];
	v1[z] = v[0][z] - v[1][z];

	v2[x] = v[1][x] - v[2][x];
	v2[y] = v[1][y] - v[2][y];
	v2[z] = v[1][z] - v[2][z];

	out[x] = v1[y]*v2[z] - v1[z]*v2[y];
	out[y] = v1[z]*v2[x] - v1[x]*v2[z];
	out[z] = v1[x]*v2[y] - v1[y]*v2[x];

	UnitarizeNormal(out);
}


/**
 * @brief toVectorInFixedSystem
 * @param dx
 * @param dy
 * @param dz
 * @param azimuth           In degrees.
 * @param inclination       In degrees.
 * @return
 */
Vec3f toVectorInFixedSystem(float dx, float dy, float dz,float azimuth, float inclination)
{
    float xRot = inclination * PI / 180.0f;
    float yRot = azimuth  * PI / 180.0f;

    float x = (float)( dx*cos(yRot) + dy*sin(xRot)*sin(yRot) - dz*cos(xRot)*sin(yRot) );
    float y = (float)(              + dy*cos(xRot)           + dz*sin(xRot)           );
    float z = (float)( dx*sin(yRot) - dy*sin(xRot)*cos(yRot) + dz*cos(xRot)*cos(yRot) );

    Vec3f vec3f(x,y,z);

    return vec3f;
}

/**
 * @brief getAzimuth This is the inverse operation of the one above.  Given a vector, it returns the azimuth of the given vector.
 * @param aim
 * @return General azimuth, 0 is north, 90 east, 180 south and 270 west.
 */
float getAzimuth(Vec3f aim)
{
    aim = aim.normalize();

    float val = atan2(aim[2], aim[0])*180.0/PI;

    if (val>=90) val -= 90;
    else val += 270;

    return val;
}

/**
 * @brief getInclination This is the inverse operation of the one above.  Given a vector, it returs the inclination of the given vector.
 * @param aim
 * @return The inclination in degrees.  -90 is the cenit, 0 is the horizon, positive looking down towards the floor and 90 is your feet.
 */
float getInclination(Vec3f aim)
{
    aim = aim.normalize();

    float incl = atan2(aim[1], aim[0]) * 180.0/PI;

    if (getAzimuth(aim) < 180.0f)
        incl += 180.0;

    return incl;
}


