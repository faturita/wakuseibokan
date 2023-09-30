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
 * @brief This function works very well.  It gets a vector in dx,dy,dz and rotate it around azimuth (0-360 clockwise) and
 * declination (0 horizon, -90 zenith).  It returns Vec3f object rotated accordingly.  This are World Coordinates.
 *
 * @param dx
 * @param dy
 * @param dz
 * @param azimuth           In degrees.  glRotatef(-Structure::azimuth,0.0f,1.0f,0.0f);
 * @param inclination       In degrees.  glRotatef(-Structure::inclination,0.0f,0.0f,1.0f);
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

float getContinuosAzimuthRadians(Vec3f aim)
{
    static float quadrant = 0;
    static float sp = 0;

    float x = getAzimuthRadians(aim);

    float currquadrant;  // clockwise 1,2,3,4
    if (aim[0]<0 && aim[2]>0)
        currquadrant = 1;
    if (aim[0]<0 && aim[2]<0)
        currquadrant = 2;
    if (aim[0]>0 && aim[2]<0)
        currquadrant = 3;
    if (aim[0]>0 && aim[2]>0)
        currquadrant = 4;

    if (quadrant == 3 && currquadrant == 2)
    {sp++;x = x + sp * 2 * PI;}
    if (quadrant == 2 && currquadrant == 3)
    {sp--;x = x + sp * 2 * PI;}

    quadrant = currquadrant;

    return x;
}


/**
 * @brief getAzimuthRadians: ODE works with a different orientation for azimuth. Positive anticlockwise, negative clockwise from north and pi/2
 * at south.
 * @param orientation
 * @return Azimuth orientation in radians.
 */
float getAzimuthRadians(Vec3f orientation)
{
    float x = getAzimuth(orientation)* (PI/180.0f) * (-1);

    if (getAzimuth(orientation)>180 && getAzimuth(orientation)<360)
    {
        x = getAzimuth(orientation)-360;
        x = x * (PI/180.0f) * (-1);
    }
    return x;
}

/**
 * @FIXME: I need to invert the (-1) so that it is exactly standard declination convention.
 * @brief This is the inverse operation of the one above.  Given a vector, it returs the declination of the given vector.
 * @param aim
 * @return The declinarion in degrees.  -90 is the cenit, 0 is the horizon, positive looking down towards the floor and 90 is your feet.
 */
float getDeclination(Vec3f aim)
{
    aim = aim.normalize();

    float decl = atan2(aim[1], sqrt(pow(aim[0],2) + pow(aim[2],2))) * 180.0/PI;

    return decl*(-1);
}

float clipmin(float val, float defval)
{
    if (val < defval)
        return defval;

    return val;

}
float clipmax(float val, float defval)
{
    if (val>defval)
        return defval;
    return val;
}

float clipped(float val, float minval, float maxval)
{
    if (val < minval)
        return minval;
    else if (val > maxval)
        return maxval;
    else
        return val;
}

float sgn(float val)
{
    if (val>=0)
        return 1;
    else
        return -1;
}

Vec3f getRandomCircularSpot(Vec3f origin, float radius)
{
    float t = (rand() % 360 + 1);

    t = t * PI/180.0f;


    float x = cos(t);
    float z = sin(t);

    x = x * radius;
    z = z * radius;

    return origin+Vec3f(x,0,z);
}


int getRandomInteger(int min, int max)
{
    int val = (rand() % ((max-min)+1) + min);
    return val;
}

float getRandom(float min, float max, int resolution)
{
    float Res = pow(10.0,resolution);
    int Max = (int) (max*Res);
    int Min = (int) (min*Res);

    int Val = getRandomInteger(Min,Max);

    return ( (float)Val/100.0);
}

float _acos(float val)
{
    float out = acos(val);
    if (isnan(out)) out=0.001;
    return out;
}

float max(float val1, float val2)
{
    if (val1>=val2) return val1;
    else return val2;
}

float min(float val1, float val2)
{
    if (val1<=val2) return val1;
    else return val2;
}

/**
 * https://barrgroup.com/embedded-systems/how-to/crc-calculation-c-code
 *
 * @brief crcFast
 * @param message
 * @param nBytes
 * @return
 */
/*
 * The width of the CRC calculation and result.
 * Modify the typedef for a 16 or 32-bit CRC standard.
 */
crc
crcSlow(uint8_t const message[], int nBytes)
{
    crc  remainder = 0;


    /*
     * Perform modulo-2 division, a byte at a time.
     */
    for (int byte = 0; byte < nBytes; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
        remainder ^= (message[byte] << (CRC_WIDTH - 8));

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (uint8_t bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & TOPBIT)
            {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    /*
     * The final remainder is the CRC result.
     */
    return (remainder);

}   /* crcSlow() */

