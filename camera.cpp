/*
 * camera.cpp
 *
 *  Created on: May 10, 2011
 *      Author: faturita
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "font/DrawFonts.h"

#include "camera.h"


/**
extern int control ;

// control = 1  Manta
// control = 2 Walrus

extern int camera ;

extern float thrust;
extern float spd ,spdX,spdZ;

extern bool ctlPause ;

extern int _xoffset ;
extern int _yoffset ;


//float _angle = 0;
//float _angleY = 0;

extern float posx, posz;

extern float modAngleY, modAngleX , modAngleZ;

extern float modAngleP;
 
 **/


Camera::Camera()
{
    dx=0.05;
    dy=dz=yAngle=xAngle=0;
    pos[0]=0.0f;pos[1]= 30.0f;pos[2]= -70.0f;
    posi=pos;
}

void Camera::reset()
{
    pos=posi;
}

void Camera::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,xAngle, -yAngle );        //Note: need to calculate at each frame
    forward = getForward();
    position = pos;
    posi=pos;
}

Vec3f Camera::getForward()
{
    // When you move forward dy and dz are static and Dx is the one which accelerates.
    
    float speedAhead = dx;
    if (dx==0) {
        speedAhead=1;
    }
    
    Vec3f forward = toVectorInFixedSystem(dy, dz, speedAhead,xAngle,-yAngle);
    return forward;
}

Vec3f Camera::getPos()
{
    return pos;
}

void Camera::setPos(Vec3f newpos)
{
    pos[0] = newpos[0];pos[1]=newpos[1];pos[2]=newpos[2];
}


//float xxx=0.0f, yyy=30.0f, zzz=-70.0f;

void CrossProd(double x1, double y1, double z1, double x2, double y2, double z2, double res[3])
{
    res[0] = y1*z2 - y2*z1;
    res[1] = x2*z1 - x1*z2;
    res[2] = x1*y2 - x2*y1;
}

void numerciallySafeLookAtFromd(double eyeX, double eyeY, double eyeZ, double lookAtX, double lookAtY, double lookAtZ, double upX, double upY, double upZ)
{
    double f[3];

    f[0] = lookAtX;
    f[1] = lookAtY;
    f[2] = lookAtZ;


/**
    eyeX += lookAtX*5;
    eyeY += lookAtY*5;
    eyeZ += lookAtZ*5;
***/

    double fMag, upMag;

    fMag = sqrt(f[0]*f[0]+f[1]*f[1]+f[2]*f[2]);
    upMag = sqrt(upX*upX + upY*upY + upZ*upZ);

    if (fMag != 0)
    {
        f[0] /= fMag;
        f[1] /= fMag;
        f[2] /= fMag;
    }

    if (upMag != 0)
    {
        upX /= upMag;
        upY /= upMag;
        upZ /= upMag;
    }

    double s[3], u[3];

    CrossProd(f[0], f[1], f[2], upX, upY, upZ, s);
    CrossProd(s[0], s[1], s[2], f[0], f[1], f[2], u);

    double M[]=
    {
    s[0], u[0], -f[0], 0,
    s[1], u[1], -f[1], 0,
    s[2], u[2], -f[2], 0,
    0, 0, 0, 1
    };

    glMultMatrixd(M);
    glTranslated (-eyeX, -eyeY, -eyeZ);


}

void numericallySafeLookAtFrom(float posx,float posy, float posz, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ)
{
    Vec3f f;

    //f[0] = lookAtX - posx;
    //f[1] = lookAtY - posy;
    //f[2] = lookAtZ - posz;

    f[0] = lookAtX;
    f[1] = lookAtY;
    f[2] = lookAtZ;

    f = f.normalize();

    Vec3f up;

    up[0] = upX;
    up[1] = upY;
    up[2] = upZ;

    up = up.normalize();

    Vec3f s = f.cross(up);
    Vec3f u = s.cross(f);

    float M[] = {
        s[0],u[0],-f[0],0,
        s[1],u[1],-f[1],0,
        s[2],u[2],-f[2],0,
        0   ,   0,    0,1
    };

    glMultMatrixf(M);
    glTranslatef(-posx,-posy,-posz);

}

void Camera::lookAtFrom(Vec3f up, Vec3f poss, Vec3f forward)
{


    //Vec3f forward = toVectorInFixedSystem(speedheight, speedwidth, speed,_angle,-_angleY);
    //Vec3f pos(0.0f, 0.0f, -20.0f);


    //Vec3f pos = _walrus.getPos();

    //Vec3f forward = _walrus.getForward();

    float xxx=poss[0];
    float yyy=poss[1];
    float zzz=poss[2];//-20.0f;

    //gluLookAt()
    numerciallySafeLookAtFromd(
        //Position
        (double)xxx,
        (double)yyy,
        (double)zzz,

        //View 'direction'
        (double)forward[0],
        (double)forward[1],
        (double)forward[2],

        //Upward vector
        (double)up[0], (double)up[1], (double)up[2]);

    //xxx+=(forward[0]);
    //yyy+=(forward[1]);
    //zzz+=(forward[2]);
    
    // How to move in any direction.
    //pos[0]+=dx;
    //pos[1]+=dy;
    //pos[2]+=dz;
    
    // How to move in forward direction.

    if (dx!=0 && abs(dx)>0.1) {
        pos[0]+=(forward[0]);
        pos[1]+=(forward[1]);
        pos[2]+=(forward[2]);
    }

    
    
    //pos=poss;
}

void Camera::lookAtFrom(Vec3f pos, Vec3f forward)
{
    Vec3f up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,yAngle, xAngle );        //Note: need to calculate at each frame
    lookAtFrom(up,pos,forward);
}



