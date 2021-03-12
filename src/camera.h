/*
 * camera.h
 *
 *  Created on: Feb 5, 2012
 *      Author: faturita
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#ifdef __linux
#include <GL/glut.h>
#elif __APPLE__
#include <GLUT/glut.h>
#endif
#include "math/yamathutil.h"
#include "observable.h"



class Camera : Observable
{
public:
    Vec3f fw;
	Camera();
	void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
	Vec3f getForward();
	Vec3f pos;
    // Keep track of a non-body camera position.
    Vec3f posi;
	Vec3f getPos();
	void setPos(Vec3f newpos);
	float dx;
	float dy;
	float dz;
	float yAngle;
	float xAngle;

	int control;
    
    void lookAtFrom(Vec3f up, Vec3f pos, Vec3f forward);
    
    void lookAtFrom(Vec3f pos, Vec3f forward);
    
    void reset();

    float getBearing();

} ;







#endif /* CAMERA_H_ */
