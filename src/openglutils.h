/*
 * openglutils.h
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#ifndef OPENGLUTILS_H_
#define OPENGLUTILS_H_


#include <cassert>
#ifdef __linux
#include <GL/glut.h>
#elif __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#endif

#include <vector>
#include "math/vec3f.h"
#include "imageloader.h"

void CheckGLError();
GLuint loadTexture(Image* image) ;
void drawLine(float x, float y, float z, float red, float green, float blue);
void drawLine(float x, float y, float z,float red, float green, float blue, float linewidth);
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
void drawRedBox(float width, float height, float length);

void drawBoxIsland(GLuint _textureId, float xx, float yy, float zz, float side,float height);
void drawBoxIsland(float xx, float yy, float zz, float side, float height);

void drawFloor(float x, float y, float z);

void drawLightning();

void drawSky (float x,float y, float z);

void initTextures();

float getFPS();

void getScreenLocation(float &winX, float &winY, float &winZ, float xx, float zz, float yy);

void Draw_Texture(double x, double y, double z, double width, double height, double Angle, GLuint texture);


class SmokeParticle
{
private:
    double x,y,z,alpha,size,speed,direction, rotation;
public:
    void Move();
    void Draw();
    void drawModel(float x, float y, float z, float width, float height, float angle, GLuint texture);
    SmokeParticle();
    Vec3f pos;
    Vec3f axis;
    float getAlpha();
};

class Smoke
{
private:
    std::vector<SmokeParticle> Smoke_Vector;
public:
    void drawModel(Vec3f pos, Vec3f axis);
    void clean();
};

#endif /* OPENGLUTILS_H_ */
