/*
 * camera.cpp
 * Wakuseibokan
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

void Camera::lookAtFrom(Vec3f up, Vec3f poss, Vec3f forward)
{


    //Vec3f forward = toVectorInFixedSystem(speedheight, speedwidth, speed,_angle,-_angleY);
    //Vec3f pos(0.0f, 0.0f, -20.0f);


    //Vec3f pos = _walrus.getPos();

    //Vec3f forward = _walrus.getForward();

    float xxx=poss[0];
    float yyy=poss[1];
    float zzz=poss[2];//-20.0f;


    gluLookAt(
        //Position
        xxx,
        yyy,
        zzz,

        //View 'direction'
        xxx+forward[0],
        yyy+forward[1],
        zzz+forward[2],

        //Upward vector
        up[0], up[1], up[2]);

    //xxx+=(forward[0]);
    //yyy+=(forward[1]);
    //zzz+=(forward[2]);
    
    // How to move in any direction.
    //pos[0]+=dx;
    //pos[1]+=dy;
    //pos[2]+=dz;
    
    // How to move in forward direction.

    if (dx!=0 && abs(dx)>0.1) {
        //pos[0]+=(forward[0]);
        //pos[1]+=(forward[1]);
        //pos[2]+=(forward[2]);
    }

    
    
    //pos=poss;
}

void Camera::lookAtFrom(Vec3f pos, Vec3f forward)
{
    Vec3f up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,yAngle, xAngle );        //Note: need to calculate at each frame
    lookAtFrom(up,pos,forward);
}

float Camera::getBearing()
{
    Vec3f f = (getForward().normalize())*30;

    f = (fw.normalize())*30;

    float val = atan2(f[2], f[0])*180.0/PI;

    if (val>=90) val -= 90;
    else val += 270;

    return val;
}



