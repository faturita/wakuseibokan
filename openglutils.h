/*
 * openglutils.h
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#ifndef OPENGLUTILS_H_
#define OPENGLUTILS_H_

#include <GLUT/glut.h>

#include "imageloader.h"

void CheckGLError();
GLuint loadTexture(Image* image) ;
void drawArrow();
void drawArrow(float scale);
void drawArrow(float x, float y, float z);
void doTransform (float pos[3], float R[12]);

void drawBox(GLuint texturedId, float xx, float yy, float zz);
void drawBox(float xx, float yy, float zz);

void drawFloor(float x, float y, float z);

void drawLightning();

void drawSky (float x,float y, float z);

void initTextures();

float getFPS();

#endif /* OPENGLUTILS_H_ */
