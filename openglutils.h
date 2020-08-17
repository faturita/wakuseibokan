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
void drawArrow(float x, float y, float z,float red, float green, float blue);
void drawArrow(float x, float y, float z,float red, float green, float blue, float linewidth);
void doTransform (float pos[3], float R[12]);
void doTransform(float R[12]);
void drawRectangularBox(float width, float height, float length);
void drawRectangularBox(float width, float height, float length, GLuint _textureId);
void drawTheRectangularBox(GLuint _textureId, float xx, float yy, float zz);
void drawTexturedBox(GLuint _textureId, float xx, float yy, float zz);
void drawBox(GLuint texturedId, float xx, float yy, float zz);
void drawBox(float xx, float yy, float zz);

void drawBoxIsland(GLuint _textureId, float xx, float yy, float zz, float side,float height);
void drawBoxIsland(float xx, float yy, float zz, float side, float height);

void drawFloor(float x, float y, float z);

void drawLightning();

void drawSky (float x,float y, float z);

void initTextures();

float getFPS();

#endif /* OPENGLUTILS_H_ */
