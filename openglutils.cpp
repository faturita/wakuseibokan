/*
 * openglutils.cpp
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#include "openglutils.h"

#include <iostream>
#include <vector>

GLuint _textureIdSea;
GLuint _textureBox;

GLuint _textureSky;

GLuint _textureLand;

GLuint _textureMetal;

GLuint _textureRoad;

std::vector<GLuint*> textures;

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

    glPopMatrix();
}

void drawArrow(float x, float y, float z, float red, float green, float blue)
{
    drawArrow(x,y,z,red,green,blue,3.0f);
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
    drawBoxIsland(_textureBox,xx,yy,zz,side, height);
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
void drawRectangularBox(float width, float height, float length)
{
    float x = width/2, y = height/2, z = length/2;
    glBegin(GL_QUADS);                // Begin drawing the color cube with 6 quads
    // Top face (y = 1.0f)
    // Define vertices in counter-clockwise (CCW) order with normal pointing out
    glColor3f(0.0f, 1.0f, 0.0f);     // Green
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
    glVertex3f( x, -y, -z);
    glVertex3f(-x, -y, -z);
    glVertex3f(-x,  y, -z);
    glVertex3f( x,  y, -z);
    
    // Left face (x = -1.0f)
    glColor3f(0.0f, 0.0f, 1.0f);     // Blue
    glVertex3f(-x,  y,  z);
    glVertex3f(-x,  y, -z);
    glVertex3f(-x, -y, -z);
    glVertex3f(-x, -y,  z);
    
    // Right face (x = 1.0f)
    glColor3f(1.0f, 0.0f, 1.0f);     // Magenta
    glVertex3f(x,  y, -z);
    glVertex3f(x,  y,  z);
    glVertex3f(x, -y,  z);
    glVertex3f(x, -y, -z);
    glEnd();  // End of drawing color-cube
    
}

void drawTheRectangularBox(GLuint _textureId, float xx, float yy, float zz)
{
    float x=xx/2.0f, y=yy/2.0f, z=zz/2.0f;

    //glLoadIdentity();
    glPushMatrix();
    glBegin(GL_QUADS);

    //Top face
    glColor3f(1.0f, 1.0f, 0.0f);
    glNormal3f(0.0, 0.0f, 1.0f);
    glVertex3f(-x, -y, z);
    glVertex3f(x, -y,  z);
    glVertex3f(x, y, z);
    glVertex3f(-x, y, z);

    //Bottom face
    glColor3f(1.0f, 0.0f, 1.0f);
    glNormal3f(0.0, -1.0f, 0.0f);
    glVertex3f(-x, -y, -z);
    glVertex3f(x, -y, - z);
    glVertex3f(x, -y,  z);
    glVertex3f(-x, -y, z);

    //Left face
    glNormal3f(-1.0, 0.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-x, -y, -z);
    glVertex3f(- x, - y, z);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-x, y, z);
    glVertex3f(-x, y, -z);

    //Right face
    glNormal3f(1.0, 0.0f, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(x, -y, -z);
    glVertex3f(x, y, -z);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(x, y, z);
    glVertex3f(x, -y, z);

    glEnd();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    glColor3f(1.0f, 1.0f, 1.0f);
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

    Image* image = loadBMP("units/metal.bmp");
    _textureMetal = loadTexture(image);
    delete image;

    image = loadBMP("reflection.bmp");
	_textureIdSea = loadTexture(image);
    delete image;
    
    image = loadBMP("vtr.bmp");
	_textureBox = loadTexture(image);
    delete image;
    
    image = loadBMP("clouds.bmp");
	_textureSky = loadTexture(image);
    delete image;
    
    image = loadBMP("terrain/grass.bmp");
    _textureLand = loadTexture(image);
    delete image;

    image = loadBMP("terrain/road.bmp");
    _textureRoad = loadTexture(image);
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
    glBindTexture(GL_TEXTURE_2D, _textureIdSea);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
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
    
    glDisable(GL_TEXTURE_2D);
    glEnd();
    glPopMatrix();
}


void drawSky (float posX, float posY, float posZ)
{
    float sky_scale=1.0f;
    float sky_height=55.0f;
    glDisable (GL_LIGHTING);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureSky);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /**if (use_textures) {
     glEnable (GL_TEXTURE_2D);
     sky_texture->bind (0);
     }**/
    //else {
    //glDisable (GL_TEXTURE_2D);
    //glColor3f (0,0.5,1.0);
    //}
    
    // make sure sky depth is as far back as possible
    glShadeModel (GL_FLAT);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
    glDepthRange (1,1);
    
    const float ssize = 100.0f;
    float offset = 0.0f;
    
    float view_xyz[3];
    
    //printf("%10.8f - %10.8f - %10.8f \n", posX, posY, posZ);
    
    posX=0;
    posY=30;
    posZ=-58;
    
    view_xyz[0] = posX;
    view_xyz[1] = posY;
    view_xyz[2] = posZ;
    

    
    float x = ssize*sky_scale;
    float z = view_xyz[2] + sky_height;
    
    glBegin (GL_QUADS);
    glNormal3f (0,0,-1);
    glTexCoord2f (-x+offset,-x+offset);
    glVertex3f (-ssize+view_xyz[0],-ssize+view_xyz[1],z);
    glTexCoord2f (-x+offset,x+offset);
    glVertex3f (-ssize+view_xyz[0],ssize+view_xyz[1],z);
    glTexCoord2f (x+offset,x+offset);
    glVertex3f (ssize+view_xyz[0],ssize+view_xyz[1],z);
    glTexCoord2f (x+offset,-x+offset);
    glVertex3f (ssize+view_xyz[0],-ssize+view_xyz[1],z);
    glEnd();
    
    offset = offset + 0.002f;
    if (offset > 1) offset -= 1;
    
    glDepthFunc (GL_LESS);
    glDepthRange (0,1);

}



void drawLightning()
{
    // Lighting, ambient light...
    //GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat ambientLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
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
    
    //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientLight);
    //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseLight);
    //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    //glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, lightPos);
    
    
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
    
    drawBox(_textureBox,xx,yy,zz);
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





