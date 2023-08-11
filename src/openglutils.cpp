/*
 * openglutils.cpp
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#include "openglutils.h"

#include <iostream>
#include <vector>
#include <unordered_map>

#ifdef __APPLE__
#include <GLUT/glut.h>
#elif __linux
#include <GL/glut.h>
#endif

#include "profiling.h"
#include "math/yamathutil.h"

std::unordered_map<std::string, GLuint> textures;

extern float horizon;

void CheckGLError() {
	GLuint err = glGetError();
	if (err != GL_NO_ERROR)
		std::cerr << std::hex << err << std::endl;
}

//Makes the image into a texture, and returns the id of the texture
GLuint loadTexture(Image* image) {
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 GL_RGB,
				 image->width, image->height,
				 0,
				 GL_RGB,
				 GL_UNSIGNED_BYTE,
				 image->pixels);
	return textureId;
}

/**
 * Receives a pos and a rotation matrix (which can be retrieved from ODE) and builds an extended
 * rotation matrix for OpenGL.  This is a transpose matrix.
 *
 *  0  4  8   0
 *  1  5  9   0
 *  2  6 10   0
 * P0 P1 P2   1
 *
 * After that uses OpenGL multiply function.
 *
 * @brief doTransform
 * @param pos
 * @param R
 */
void doTransform (float pos[3], float R[12])
{
  GLfloat matrix[16];
  matrix[0]=R[0];
  matrix[1]=R[4];
  matrix[2]=R[8];
  matrix[3]=0;
  matrix[4]=R[1];
  matrix[5]=R[5];
  matrix[6]=R[9];
  matrix[7]=0;
  matrix[8]=R[2];
  matrix[9]=R[6];
  matrix[10]=R[10];
  matrix[11]=0;
  matrix[12]=pos[0];
  matrix[13]=pos[1];
  matrix[14]=pos[2];
  matrix[15]=1;
  //glPushMatrix();
  glMultMatrixf (matrix);
}

/**
 * Performs a matrix rotation at the origin.
 *
 * @brief doTransform
 * @param R
 */
void doTransform(float R[12])
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;
    
    doTransform(f, R);
}


void drawArrow(float x, float y, float z)
{
    // Draw a pink arrow.
    drawArrow(x,y,z,1.0f,0.5f,0.5f);
}

void drawLine(float x, float y, float z,float red, float green, float blue, float linewidth)
{
    glPushMatrix();
    glLineWidth(linewidth);

    // RED
    glTranslatef(0.0f,0.0f,0.0f);
    glColor3f(red,green,blue);
    glBegin(GL_LINES);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(x,y,z);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawArrow(float x, float y, float z,float red, float green, float blue, float linewidth)
{
    glPushMatrix();
    glLineWidth(linewidth);

    // RED
    glTranslatef(0.0f,0.0f,0.0f);
    glColor3f(red,green,blue);
    glBegin(GL_LINES);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(x,y,z);
    glEnd();

    glTranslatef(x,y,z);
    glRotatef(90.0f,0.0f,1.0f,0.0f);
    glutSolidCone(0.100,0.5f,10,10);

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawArrow(float x, float y, float z, float red, float green, float blue)
{
    drawArrow(x,y,z,red,green,blue,3.0f);
}

void drawLine(float x, float y, float z, float red, float green, float blue)
{
    drawLine(x,y,z,red,green,blue,3.0f);
}

void drawArrow()
{
	drawArrow(3.0f);
}

void drawArrow(float scale)
{
    glPushMatrix();

    // RED
    glTranslatef(0.0f,0.0f,0.0f);
    glColor3f(1.0f,0.0f,0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(scale,0.0f,0.0f);
    glEnd();

    glTranslatef(scale,0.0f,0.0f);
    glRotatef(90.0f,0.0f,1.0f,0.0f);
    glutSolidCone(0.100,0.5f,10,10);

    glPopMatrix();

    // GREEN
    glPushMatrix();
    glTranslatef(0.0f,0.0f,0.0f);
    glColor3f(0.0f,1.0f,0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(0.0f,scale,0.0f);
    glEnd();

    glTranslatef(0.0f,scale,0.0f);
    glRotatef(-90.0f,1.0f,0.0f,0.0f);
    glutSolidCone(0.100,0.5f,10,10);

    glPopMatrix();

    // BLUE
    glPushMatrix();
    glTranslatef(0.0f,0.0f,0.0f);
    glColor3f(0.0f,0.0f,1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(0.0f,0.0f,scale);
    glEnd();

    glTranslatef(0.0f,0.0f,scale);
    glRotatef(-90.0f,0.0f,0.0f,1.0f);
    glutSolidCone(0.100,0.5f,10,10);

    glPopMatrix();

}


float boxangle = 0;

void drawBoxIsland(float xx, float yy, float zz, float side, float height)
{
    drawBoxIsland(textures["metal"],xx,yy,zz,side, height);
}

void drawBoxIsland(GLuint _textureId, float xx, float yy, float zz, float side, float height)
{
    int x=0, y=0, z=0;
    
     float BOX_SIZE = 7.0f; //The length of each side of the cube
    
    BOX_SIZE=side;
    
    //glLoadIdentity();
    glPushMatrix();
    glTranslatef(xx,yy,zz);
    glBegin(GL_QUADS);
    
    //Top face
    glColor3f(1.0f, 1.0f, 0.0f);
    glNormal3f(0.0, 1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, height / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, height / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, height / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, height / 2 + y, -BOX_SIZE / 2 + z);
    
    //Bottom face
    glColor3f(1.0f, 0.0f, 1.0f);
    glNormal3f(0.0, -1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -height / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -height / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -height / 2 + y, BOX_SIZE /2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, -height / 2 + y, BOX_SIZE / 2 + z);
    
    //Left face
    glNormal3f(-1.0, 0.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -height / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, -height / 2 + y, BOX_SIZE / 2 + z);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, height / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, height / 2 + y, -BOX_SIZE / 2 + z);
    
    //Right face
    glNormal3f(1.0, 0.0f, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, -height / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, height / 2 + y, -BOX_SIZE / 2 + z);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, height / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -height / 2 + y, BOX_SIZE / 2 + z);
    
    glEnd();
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 1.0f);
    
    glBegin(GL_QUADS);
    
    //Front face
    glNormal3f(0.0, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -height / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, -height / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2 + x, height / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, height / 2 + y, BOX_SIZE / 2 + z);
    
    //Back face
    glNormal3f(0.0, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2+ x, -height / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2+ x, height / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2+ x, height / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2+ x, -height / 2+ y, -BOX_SIZE / 2+ z);
    
    glEnd();
    glPopMatrix();
    
}



/**
 * When you draw the object, you move to height/2 to one side, and then height/2 to the other.
 *
 **/
void drawRedBox(float width, float height, float length)
{
    float x = width/2, y = height/2, z = length/2;
    glBegin(GL_QUADS);                // Begin drawing the color cube with 6 quads
    // Top face (y = 1.0f)
    // Define vertices in counter-clockwise (CCW) order with normal pointing out
    glColor3f(1.0f, 0.0f, 0.0f);     // Red
    glVertex3f( x, y, -z);
    glVertex3f(-x, y, -z);
    glVertex3f(-x, y,  z);
    glVertex3f( x, y,  z);

    // Bottom face (y = -1.0f)
    glColor3f(1.0f, 0.0f, 0.0f);     // Red
    glVertex3f( x, -y,  z);
    glVertex3f(-x, -y,  z);
    glVertex3f(-x, -y, -z);
    glVertex3f( x, -y, -z);

    // Front face  (z = 1.0f)
    glColor3f(1.0f, 0.0f, 0.0f);     // Red
    glVertex3f( x,  y, z);
    glVertex3f(-x,  y, z);
    glVertex3f(-x, -y, z);
    glVertex3f( x, -y, z);

    // Back face (z = -1.0f)
    glColor3f(1.0f, 0.0f, 0.0f);     // Red
    glVertex3f( x, -y, -z);
    glVertex3f(-x, -y, -z);
    glVertex3f(-x,  y, -z);
    glVertex3f( x,  y, -z);

    // Left face (x = -1.0f)
    glColor3f(1.0f, 0.0f, 0.0f);     // Red
    glVertex3f(-x,  y,  z);
    glVertex3f(-x,  y, -z);
    glVertex3f(-x, -y, -z);
    glVertex3f(-x, -y,  z);

    // Right face (x = 1.0f)
    glColor3f(1.0f, 0.0f, 0.0f);     // Red
    glVertex3f(x,  y, -z);
    glVertex3f(x,  y,  z);
    glVertex3f(x, -y,  z);
    glVertex3f(x, -y, -z);
    glEnd();  // End of drawing color-cube

}

void drawRectangularBox(float width, float height, float length)
{
    drawRectangularBox(width, height, length, Vec3f(0.0f,1.0f,0.0f),Vec3f(1.0f,1.0f,0.0f),Vec3f(0.0f, 0.0f, 1.0f),Vec3f(1.0f, 0.0f, 1.0f));
}
/**
 * When you draw the object, you move to height/2 to one side, and then height/2 to the other.
 *
 **/
void drawRectangularBox(float width, float height, float length, Vec3f green, Vec3f yellow, Vec3f blue, Vec3f magenta)
{
    float x = width/2, y = height/2, z = length/2;
    glBegin(GL_QUADS);                // Begin drawing the color cube with 6 quads
    // Top face (y = 1.0f)
    // Define vertices in counter-clockwise (CCW) order with normal pointing out
    glColor3f(0.0f, 1.0f, 0.0f);     // Green
    glColor3f(0.0f, 0.2f, 0.0f);
    glColor3f(green[0],green[1],green[2]);
    glVertex3f( x, y, -z);
    glVertex3f(-x, y, -z);
    glVertex3f(-x, y,  z);
    glVertex3f( x, y,  z);
    
    // Bottom face (y = -1.0f)
    glColor3f(1.0f, 0.5f, 0.0f);     // Orange
    glVertex3f( x, -y,  z);
    glVertex3f(-x, -y,  z);
    glVertex3f(-x, -y, -z);
    glVertex3f( x, -y, -z);
    
    // Front face  (z = 1.0f)
    glColor3f(1.0f, 0.0f, 0.0f);     // Red
    glVertex3f( x,  y, z);
    glVertex3f(-x,  y, z);
    glVertex3f(-x, -y, z);
    glVertex3f( x, -y, z);
    
    // Back face (z = -1.0f)
    glColor3f(1.0f, 1.0f, 0.0f);     // Yellow
    glColor3f(0.2, 0.2f, 0.0f);
    glColor3f(yellow[0],yellow[1],yellow[2]);
    glVertex3f( x, -y, -z);
    glVertex3f(-x, -y, -z);
    glVertex3f(-x,  y, -z);
    glVertex3f( x,  y, -z);

    // Left face (x = -1.0f)
    glColor3f(0.0f, 0.0f, 1.0f);     // Blue
    glColor3f(0.0f, 0.0f, 0.2f);
    glColor3f(blue[0],blue[1],blue[2]);
    glVertex3f(-x,  y,  z);
    glVertex3f(-x,  y, -z);
    glVertex3f(-x, -y, -z);
    glVertex3f(-x, -y,  z);
    
    // Right face (x = 1.0f)
    glColor3f(1.0f, 0.0f, 1.0f);     // Magenta
    glColor3f(0.2f, 0.0f, 0.1f);
    glColor3f(magenta[0],magenta[1],magenta[2]);
    glVertex3f(x,  y, -z);
    glVertex3f(x,  y,  z);
    glVertex3f(x, -y,  z);
    glVertex3f(x, -y, -z);
    glEnd();  // End of drawing color-cube
    
}


void drawTexturedBox(GLuint _textureId, float xx, float yy, float zz)
{
    float x=xx/2.0f, y=yy/2.0f, z=zz/2.0f;

    //glLoadIdentity();
    glPushMatrix();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    //Top face
    //glColor3f(1.0f, 1.0f, 0.0f);
    glNormal3f(0.0, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-x, -y, z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x, -y,  z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x, y, z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-x, y, z);

    //glColor3f(1.0f, 1.0f, 1.0f);
    glNormal3f(0.0, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-x, y, -z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-x, y, z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x, y, z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x, y, -z);

    //Back face
    glNormal3f(0.0, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-x, -y, -z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-x, y, -z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x, y, - z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x, -y, -z);

    //Bottom face
    //glColor3f(1.0f, 0.0f, 1.0f);
    glNormal3f(0.0, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-x, -y, -z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x, -y, - z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x, -y,  z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-x, -y, z);

    //Left face
    glNormal3f(-1.0, 0.0f, 0.0f);
    //glColor3f(0.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-x, -y, -z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(- x, - y, z);
    //glColor3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-x, y, z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-x, y, -z);

    //Right face
    glNormal3f(1.0, 0.0f, 0.0f);
    //glColor3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x, -y, -z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x, y, -z);
    //glColor3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x, y, z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x, -y, z);

    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawTheRectangularBox(GLuint _textureId, float xx, float yy, float zz)
{
    float x=xx/2.0f, y=yy/2.0f, z=zz/2.0f;

    //glLoadIdentity();
    glPushMatrix();
    glBegin(GL_QUADS);

    //Top face
    //glColor3f(1.0f, 1.0f, 0.0f);
    glNormal3f(0.0, 0.0f, 1.0f);
    glVertex3f(-x, -y, z);
    glVertex3f(x, -y,  z);
    glVertex3f(x, y, z);
    glVertex3f(-x, y, z);

    //Bottom face
    //glColor3f(1.0f, 0.0f, 1.0f);
    glNormal3f(0.0, -1.0f, 0.0f);
    glVertex3f(-x, -y, -z);
    glVertex3f(x, -y, - z);
    glVertex3f(x, -y,  z);
    glVertex3f(-x, -y, z);

    //Left face
    //glNormal3f(-1.0, 0.0f, 0.0f);
    //glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-x, -y, -z);
    glVertex3f(- x, - y, z);
    //glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-x, y, z);
    glVertex3f(-x, y, -z);

    //Right face
    glNormal3f(1.0, 0.0f, 0.0f);
    //glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(x, -y, -z);
    glVertex3f(x, y, -z);
    //glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(x, y, z);
    glVertex3f(x, -y, z);

    glEnd();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    //glColor3f(1.0f, 1.0f, 1.0f);
    glNormal3f(0.0, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-x, y, -z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-x, y, z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x, y, z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x, y, -z);

    //Back face
    glNormal3f(0.0, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-x, -y, -z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-x, y, -z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x, y, - z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x, -y, -z);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawBox(GLuint _textureId, float xx, float yy, float zz)
{
    int x=0, y=0, z=0;

    const float BOX_SIZE = 7.0f; //The length of each side of the cube

    //glLoadIdentity();
    glPushMatrix();
    glTranslatef(xx,yy,zz);
    glRotatef(boxangle, 0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);

    //Top face
    glColor3f(1.0f, 1.0f, 0.0f);
    glNormal3f(0.0, 1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);

    //Bottom face
    glColor3f(1.0f, 0.0f, 1.0f);
    glNormal3f(0.0, -1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE /2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);

    //Left face
    glNormal3f(-1.0, 0.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);

    //Right face
    glNormal3f(1.0, 0.0f, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);

    glEnd();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    //Front face
    glNormal3f(0.0, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);

    //Back face
    glNormal3f(0.0, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2+ x, -BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2+ x, BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2+ x, BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2+ x, -BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);

    glEnd();
    glPopMatrix();

    boxangle += 0.5f;

    if (boxangle >= 360.f)
        boxangle = 0.0f;
}




void initTextures()
{

    GLuint _texture;

    Image* image = loadBMP("units/metal.bmp");
    _texture = loadTexture(image);              // Box
    textures["metal"] = _texture;
    delete image;

    image = loadBMP("water/reflection.bmp");
    _texture = loadTexture(image);
    textures["sea"] = _texture;
    delete image;
    
    image = loadBMP("sky/clouds.bmp");
    _texture = loadTexture(image);
    textures["sky"] = _texture;
    delete image;
    
    image = loadBMP("terrain/grass.bmp");
    _texture = loadTexture(image);
    textures["land"] = _texture;
    delete image;

    image = loadBMP("terrain/road.bmp");
    _texture = loadTexture(image);
    textures["road"] = _texture;
    delete image;

    image = loadBMP("structures/command.bmp");
    assert( image != NULL || !"Something went wrong." );
    _texture = loadTexture(image);
    textures["military"] = _texture;
    delete image;

    image = loadBMP("sky/saturn.bmp");
    _texture = loadTexture(image);
    textures["saturn"] = _texture;
    delete image;

    image = loadBMP("sky/solar.bmp");
    _texture = loadTexture(image);
    textures["solar"] = _texture;
    delete image;

    image = loadBMP("terrain/boat.bmp");
    _texture = loadTexture(image);
    textures["boat"] = _texture;
    delete image;

    image = loadBMP("terrain/smoke.bmp");
    _texture = loadTexture(image);
    textures["solar"] = _texture;
    delete image;

    image = loadBMP("terrain/sun.bmp");
    _texture = loadTexture(image);
    textures["sun"] = _texture;
    delete image;


    image = loadBMP("terrain/moon.bmp");
    _texture = loadTexture(image);
    textures["moon"] = _texture;
    delete image;


    image = loadBMP("terrain/venus.bmp");
    _texture = loadTexture(image);
    textures["venus"] = _texture;
    delete image;

    image = loadBMP("terrain/stars.bmp");
    _texture = loadTexture(image);
    textures["stars"] = _texture;
    delete image;

    image = loadBMP("terrain/theclouds.bmp");
    _texture = loadTexture(image);
    textures["clouds"] = _texture;
    delete image;

}


void drawFloor(float x, float y, float z)
{
	// Slide the texture always from the starting position to generate
	// the effect that we are moving...
    const float floor_size = 100.0f;
    
    //float horizon = 10450.0f;   /// 1450.0f es
    float floorSize = (y + horizon) / floor_size;
    
    float start = -floorSize-x/floor_size;
    float stop =   floorSize-x/floor_size;
    float sstart =-floorSize+z/floor_size;
    float sstop =  floorSize+z/floor_size;
    
    glPushMatrix();
    
    glTranslatef(x,0.0f,z);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures["sea"]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //glEnable(GL_BLEND);
    //glColor4f (0.0, 0.0, 0.6, 0.2);
    
    glBegin(GL_QUADS);
    
    glNormal3f(0.0f, 1.0f, 0.0f);
    
    glTexCoord2f (stop, sstart);
    glVertex3f(-horizon, 0.0f, -horizon);
    
    glTexCoord2f (stop, sstop);
    glVertex3f(-horizon, 0.0f, horizon);
    
    glTexCoord2f (start, sstop);
    glVertex3f(horizon, 0.0f, horizon);
    
    glTexCoord2f (start, sstart);
    glVertex3f(horizon, 0.0f, -horizon);
    
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}


void drawSky (float posX, float posY, float posZ)
{
    float sky_scale=1.0f;
    float sky_height=55.0f;
    glDisable (GL_LIGHTING);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures["saturn"]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // make sure sky depth is as far back as possible
    glShadeModel (GL_FLAT);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
    glDepthRange (1,1);
    
    const float ssize = 1000.0f;
    static float offset_h = 0.0f;
    static float offset_v = 0.0f;
    
    float view_xyz[3];
    
    //CLog::Write(CLog::Debug,"%10.8f - %10.8f - %10.8f \n", posX, posY, posZ);
    

    offset_h = getAzimuth(Vec3f(posX, posY, posZ))*(1/360.0) * 20;//6
    offset_v = getDeclination(Vec3f(posX, posY, posZ))*(-1/360.0) * 20;

    
    view_xyz[0] = 0;
    view_xyz[1] = 30;
    view_xyz[2] = -58;
    

    
    float x = ssize*sky_scale;
    float z = view_xyz[2] + sky_height;
    
//    glBegin (GL_QUADS);
//    glNormal3f (0,0,-1);
//    glTexCoord2f (-x+offset,-x+offset);
//    glVertex3f (-ssize+view_xyz[0],-ssize+view_xyz[1],z);
//    glTexCoord2f (-x+offset,x+offset);
//    glVertex3f (-ssize+view_xyz[0],ssize+view_xyz[1],z);
//    glTexCoord2f (x+offset,x+offset);
//    glVertex3f (ssize+view_xyz[0],ssize+view_xyz[1],z);
//    glTexCoord2f (x+offset,-x+offset);
//    glVertex3f (ssize+view_xyz[0],-ssize+view_xyz[1],z);
//    glEnd();

    glBegin (GL_QUADS);
    glNormal3f (0,0,-1);
    glTexCoord2f (-x+offset_h,-x+offset_v);
    glVertex3f (-ssize+view_xyz[0],-ssize+view_xyz[1],z);
    glTexCoord2f (-x+offset_h,x+offset_v);
    glVertex3f (-ssize+view_xyz[0],ssize+view_xyz[1],z);
    glTexCoord2f (x+offset_h,x+offset_v);
    glVertex3f (ssize+view_xyz[0],ssize+view_xyz[1],z);
    glTexCoord2f (x+offset_h,-x+offset_v);
    glVertex3f (ssize+view_xyz[0],-ssize+view_xyz[1],z);
    glEnd();



//    const float BOX_SIZE = 0.3f; //The length of each side of the cube

//    x=0;
//    float y=0.0;

//    //glLoadIdentity();
//    glPushMatrix();
//    glTranslatef(0,0,0);
//    //glRotatef(boxangle, 0.0f, 0.0f, 1.0f);
//    glRotatef(-offset_v,1.0f,0.0f,0.0f);
//    glRotatef(-offset_h,0.0f,1.0f,0.0f);
//    glBegin(GL_QUADS);

//    //Top face
//    //glColor3f(1.0f, 1.0f, 0.0f);
//    glNormal3f(0.0, 1.0f, 0.0f);
//    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
//    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
//    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
//    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);

//    //Bottom face
//    //glColor3f(1.0f, 0.0f, 1.0f);
//    glNormal3f(0.0, -1.0f, 0.0f);
//    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
//    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
//    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE /2 + z);
//    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);

//    //Left face
//    glNormal3f(-1.0, 0.0f, 0.0f);
//    //glColor3f(0.0f, 1.0f, 1.0f);
//    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
//    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
//    //glColor3f(0.0f, 0.0f, 1.0f);
//    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
//    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);

//    //Right face
//    glNormal3f(1.0, 0.0f, 0.0f);
//    //glColor3f(1.0f, 0.0f, 0.0f);
//    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
//    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
//    //glColor3f(0.0f, 1.0f, 0.0f);
//    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
//    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);

//    glEnd();

//    //glEnable(GL_TEXTURE_2D);
//    //glBindTexture(GL_TEXTURE_2D, _textureId);
//    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    //glColor3f(1.0f, 1.0f, 1.0f);

//    glBegin(GL_QUADS);

//    //Front face
//    glNormal3f(0.0, 0.0f, 1.0f);
//    glTexCoord2f(0.0f, 0.0f);
//    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
//    glTexCoord2f(1.0f, 0.0f);
//    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
//    glTexCoord2f(1.0f, 1.0f);
//    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
//    glTexCoord2f(0.0f, 1.0f);
//    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);

//    //Back face
//    glNormal3f(0.0, 0.0f, -1.0f);
//    glTexCoord2f(0.0f, 0.0f);
//    glVertex3f(-BOX_SIZE / 2+ x, -BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
//    glTexCoord2f(1.0f, 0.0f);
//    glVertex3f(-BOX_SIZE / 2+ x, BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
//    glTexCoord2f(1.0f, 1.0f);
//    glVertex3f(BOX_SIZE / 2+ x, BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
//    glTexCoord2f(0.0f, 1.0f);
//    glVertex3f(BOX_SIZE / 2+ x, -BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);

//    glEnd();
//    glPopMatrix();

//    boxangle += 0.1f;

//    if (boxangle >= 360.f)
//        boxangle = 0.0f;
    
    //offset = offset + 0.002f;
    //if (offset > 1) offset -= 1;
    
    glDepthFunc (GL_LESS);
    glDepthRange (0,1);

}



void drawLightning()
{
    // Lighting, ambient light...
    GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
    //GLfloat ambientLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    
	GLfloat diffuseLight[] = { 0.7f  , 0.7f, 0.7f,   1.0f};
	GLfloat specular[]     = { 1.0f  , 1.0f, 1.0f,   1.0f};
    GLfloat lightPos[]     = { 0.0f  , -1.0f, 0.0f,   1.0f};
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR,specular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    
	glEnable(GL_COLOR_MATERIAL);
    
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientLight);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseLight);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, lightPos);
    
    
	//glTranslatef(0.0f+posx, 0.0f+posy, -20.0f+posz);
	
    // Lighting, ambient light...
	//GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
     //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    
    // Add positioned light (outisde glBegin-glEnd)
     //GLfloat lightColor0[] = {0.5f, 0.5f, 0.5f, 1.0f};
     //GLfloat lightPos0[] = { 4.0f, 0.0f, 8.0f, 1.0f  };
     //glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColor0);
     //glLightfv(GL_LIGHT2, GL_POSITION, lightPos0);
     
     // Add directed light
     //GLfloat lightColor1[] = {0.5f, 0.2f, 0.2f, 1.0f};
     //GLfloat lightPos1[] = {-1.0f, 0.5f, 0.5f, 0.0f};
     //glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor1);
     //glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
    
    /**
     GLfloat lightColor[] = {0.7f, 0.7f, 0.7f, 1.0f};
     GLfloat lightPos[] = {-2 * BOX_SIZE, BOX_SIZE, 4 * BOX_SIZE, 1.0f};
     glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
     glLightfv(GL_LIGHT0, GL_POSITION, lightPos);**/
}


void drawBox(float xx, float yy, float zz)
{
    int x=0, y=0, z=0;
    
    const float BOX_SIZE = 7.0f; //The length of each side of the cube
    static float boxangle = 0;            //The rotation of the box
    
    drawBox(textures["metal"],xx,yy,zz);
}

float getFPS()
{
    int time=0;
    
    static int frame = 0;
    static int timebase=0;
    static float fps=0;
    
    frame++;
	time=glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
        fps=((float)frame*1000.0/(time-timebase));
		timebase = time;
		frame = 0;
	}
    
    return fps;
}

void getScreenLocation(float &screenX, float &screenY, float &screenZ, float xx, float yy, float zz)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble viewVector[3];
    GLdouble projection[16];

    GLdouble winX, winY, winZ;//2D point

    GLdouble posX, posY, posZ;//3D point
    posX=xx;
    posY=yy;
    posZ=zz;

    //get the matrices
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );

    viewVector[0]=modelview[8];
    viewVector[1]=modelview[9];
    viewVector[2]=modelview[10];

    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );

    int res=gluProject(posX,posY,posZ,modelview,projection,viewport,&winX,&winY,&winZ);

    //if(viewVector[0]*posX+viewVector[1]*posY+viewVector[2]*posZ<0){
            //dout << winX << "," << winY << std::endl;
    //}

    screenX = winX;
    screenY = winY;
    screenZ = winZ;

}



void SmokeParticle::drawModel(float x, float y, float z, float width, float height, float angle, GLuint texture)
{
    glPushAttrib(GL_CURRENT_BIT);
    glPushMatrix();
    {
        glEnable(GL_TEXTURE_2D);
        //glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_NEAREST = no smoothing
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D,texture);
        glTranslatef(x, y, z);
        glRotated(angle, 0, 0,1);
        glBegin(GL_QUADS);
        double x_centre = width/2;
        double y_centre = height/2;
        glTexCoord2d(0.0,0.0); glVertex3f(-x_centre, -y_centre,-y_centre);
        glTexCoord2d(1.0,0.0); glVertex3f(x_centre, -y_centre,-y_centre);
        glTexCoord2d(1.0,1.0); glVertex3f(x_centre, y_centre,+y_centre);
        glTexCoord2d(0.0,1.0); glVertex3f(-x_centre, y_centre,+y_centre);
        glEnd();
    }
    glPopMatrix();
    glPopAttrib();
}


void SmokeParticle::Move(void)
{
    x += cos(direction) * speed;
    z += sin(direction) * speed;
    y += 1;
    size += 0.1;
    alpha -= alpha*0.01;
    rotation += 0.1;
}

void SmokeParticle::Draw(void)
{
    glColor4d(1, 1, 1, alpha);


    // Each particle grows in a cone with axis (0,1,0), this is Up.
    // So the idea is to pick the axis where you want to make it grow, and rotate x,y,z towards it.
    Vec3f Up(0,1,0);
    Vec3f rot,fw = axis;
    fw = fw.normalize();

    rot = Up.cross(fw);

    float a = _acos(  Up.dot(fw)  );

    Vec3f tran(x,y,z);
    tran = tran.rotateOn(rot, a);

    if (isnan(tran[0])) tran = Vec3f(x,y,z);

    //dout << tran << std::endl;

    drawModel(pos[0]+tran[0], pos[1]+tran[1], pos[2]+tran[2], size, size, rotation, textures["smoke"]);
}

SmokeParticle::SmokeParticle()
{
    x = 0;
    z = 0;
    y = 0;
    size = 0;
    direction = getRandomInteger(0,360);
    rotation = getRandomInteger(0,360);
    speed = 0.05;
    alpha = 0.3;
}

float SmokeParticle::getAlpha()
{
    return alpha;
}

void Smoke::drawModel(Vec3f pos, Vec3f axis)
{
    SmokeParticle s;
    s.pos = pos ; //Vec3f(-5000,1000-2.5,-5000);
    s.axis = (-1)*axis; //(-1)*Vec3f(7,8,9);

    Smoke_Vector.push_back(s);

    if (Smoke_Vector.size()>number_of_particles)
    {
        Smoke_Vector.erase(Smoke_Vector.begin());
    }

    for( size_t i = 0 ; i < Smoke_Vector.size() ; ++i )
        {
        Smoke_Vector[i].Draw();
        Smoke_Vector[i].Move();
        }
}


void Smoke::clean()
{
    Smoke_Vector.clear();
}


