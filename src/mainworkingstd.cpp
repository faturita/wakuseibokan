

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>
#include <math.h>

#include <GL/glut.h>

#include <ode/ode.h>

#include "FractalNoise.h"
#include "terrain/imageloader.h"
#include "md2model.h"
#include "terrain/Terrain.h"
#include "font/DrawFonts.h"

#include "math/yamathutil.h"

#include "openglutils.h"

using namespace std;



const float BOX_SIZE = 7.0f; //The length of each side of the cube
float _angle = 0;            //The rotation of the box
GLuint _textureId;           //The OpenGL id of the texture

GLuint _textureIdSea;

int _direction = 1;

MD2Model* _modelManta;
MD2Model* _modelWalrus;

Terrain* _terrain;


int specialKey = 0;

float posx=0, posz=0;

float speed=0.05;

float speedwidth=0.0;
float speedheight=0.0;

float xxx=0.0f, yyy=30.0f, zzz=-70.0f;



void processMouseEntry(int state) {
    /**if (state == GLUT_LEFT)
        _angle = 0.0;
    else
        _angle = 1.0;**/
}

float _angleY = 0;

int _xoffset = 0;
int _yoffset = 0;
    


float spd = 0,spdX=0,spdZ=0;

void processMouse(int button, int state, int x, int y) {


    //int specialKey = glutGetModifiers();
    // if both a mouse button, and the ALT key, are pressed  then
    if ((state == GLUT_DOWN)) {
        
        _xoffset = _yoffset = 0;

        // set the color to pure red for the left button
        if (button == GLUT_LEFT_BUTTON) {

        }
        // set the color to pure green for the middle button
        else if (button == GLUT_MIDDLE_BUTTON) {

        }
        // set the color to pure blue for the right button
        else {

        }
    }
}



void processMouseActiveMotion(int x, int y) {
    //int specialKey = glutGetModifiers();
    // the ALT key was used in the previous function
    //if (specialKey != GLUT_ACTIVE_ALT) {

    //printf ("X value:%d\n", x);
    
    if (_xoffset ==0 ) _xoffset = x;
    
    if (_yoffset == 0 ) _yoffset = y;
    
    _angle += ( (x-_xoffset) * 0.005);
    
    _angleY += ( (y - _yoffset ) * 0.005) ;
    //}
}




    
void processMousePassiveMotion(int x, int y) {
    //int specialKey = glutGetModifiers();
    // User must press the SHIFT key to change the 
    // rotation in the X axis
    //if (specialKey != GLUT_ACTIVE_SHIFT) {

        // setting the angle to be relative to the mouse 
        // position inside the window
   /**     if (x < 0)
            _angle = 0.0;
        else if (x > 400)
            _angle = 180.0;
        else
            _angle = 180.0 * ((float) x)/400;
   **/ //}
}

float modAngleY=0, modAngleX = 0;



void handleKeypress(unsigned char key, int x, int y) {
	switch (key) {
		case 27: //Escape key
			exit(0);
        case '1':_angle = _angleY = 0;break;
        case '2':_angle = 90;_angleY = 0; break;
        case '3':_angle = 0; _angleY = 90; break;
        case '+':speed+=0.01;break;
        case '-':speed-=0.01;break;
        case 32 :speed=0; break;
        case 'o':speedwidth+=0.1;break;
        case 'p':speedwidth-=0.1;break;
        case 'a':modAngleX+=0.5f;break;
        case 'd':modAngleX-=0.5f;break;
        case 'w':modAngleY+=0.5f;break;
        case 's':modAngleY-=0.5f;break;
        case 'r':spd-=0.05;break;
        case 'f':spd+=0.05;break;
        case 't':spdX+=0.05;break;
        case 'g':spdX-=0.05;break;
        case 'y':spdZ+=0.05;break;
        case 'h':spdZ-=0.05;break;
        	}
}
    
void handleSpecKeypress(int key, int x, int y)
{
    //specialKey = glutGetModifiers();
    
    switch (key) {
        case GLUT_KEY_LEFT : 
            posx++;break;
        case GLUT_KEY_RIGHT : 
            posx--;break;
        case GLUT_KEY_UP : 
            posz++;break;
        case GLUT_KEY_DOWN : 
            posz--;break;
    }


}


// *********************** ODE Controlss ********************************
/* some constants */

#define NUM 10			/* number of boxes */
#define SIDE (0.2)		/* side length of a box */
#define MASS (1.0)		/* mass of a box */
#define RADIUS (0.1732f)	/* sphere radius */


/* dynamics and collision objects */

static dWorldID world;
static dSpaceID space;
static dBodyID body[NUM];
static dJointID joint[NUM-1];
static dJointGroupID contactgroup;
static dGeomID sphere[NUM];

static void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
  /* exit without doing anything if the two bodies are connected by a joint */
  dBodyID b1,b2;
  dContact contact;

  b1 = dGeomGetBody(o1);
  b2 = dGeomGetBody(o2);
  if (b1 && b2 && dAreConnected (b1,b2)) return;

  contact.surface.mode = 0;
  contact.surface.mu = 0.1;
  contact.surface.mu2 = 0;
  if (dCollide (o1,o2,1,&contact.geom,sizeof(dContactGeom))) {
    dJointID c = dJointCreateContact (world,contactgroup,&contact);
    dJointAttach (c,b1,b2);
  }
}

// **********************************************************************


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


void drawHUD()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1200, 800, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor4f(1.0f, 1.0f, 1.0f, 1);
	glDisable(GL_DEPTH_TEST);
	glRotatef(180.0f,0,0,1);
	glRotatef(180.0f,0,1,0);

    char str[256];

    sprintf (str, "(%10.8f,%10.8f,%10.8f)\n", xxx,yyy,zzz);
	// width, height, 0 0 upper left
	drawString(0,-30,1,str,0.2f);

	//glPrint(1,10,10,"HUD");

	//glRectf(400.0f,400.0f,450.0f,400.0f);


	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

float tangle = 0.0f;


void drawTerrain(float fscale)
{
	float scale = fscale / max(_terrain->width() - 1, _terrain->length() - 1);
	glScalef(scale, scale, scale);
	glTranslatef(-(float)(_terrain->width() - 1) / 2,
				 0.0f,
				 -(float)(_terrain->length() - 1) / 2);

	glColor3f(0.3f, 0.9f, 0.0f);
	for(int z = 0; z < _terrain->length() - 1; z++) {
		//Makes OpenGL draw a triangle at every three consecutive vertices
		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 0; x < _terrain->width(); x++) {
			Vec3f normal = _terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, _terrain->getHeight(x, z), z);
			normal = _terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, _terrain->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}
}

const float FLOOR_TEXTURE_SIZE = 10.0f; //The size of each floor "tile"
//The forward position of the guy relative to an arbitrary floor "tile"
float _guyPos = yyy;

void drawFloor()
{
    float floorSize = 2000.0f + 0;
    float horizon = 10050.0f;
    
    glPushMatrix();
    //Draw the floor
    glTranslatef(0.0f, 0.0f, 0.0f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureIdSea);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBegin(GL_QUADS);
    
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(floorSize / FLOOR_TEXTURE_SIZE, _guyPos / FLOOR_TEXTURE_SIZE);
    glVertex3f(-horizon, 0.0f, -horizon);
    glTexCoord2f(floorSize / FLOOR_TEXTURE_SIZE,
                 (floorSize + _guyPos) / FLOOR_TEXTURE_SIZE);
    glVertex3f(-horizon, 0.0f, horizon);
    glTexCoord2f(0.0f, (floorSize + _guyPos) / FLOOR_TEXTURE_SIZE);
    glVertex3f(horizon, 0.0f, horizon);
    glTexCoord2f(0.0f, _guyPos / FLOOR_TEXTURE_SIZE);
    glVertex3f(horizon, 0.0f, -horizon);
    
    glEnd();
    glPopMatrix();
}

void coneIsland(float diameter,float height, float fIslandX, float fIslandY, float fIslandZ, float precision=16)
{
	GLfloat x,y,angle;
	int iPivot = 1;

    glPushMatrix();
    glTranslatef(fIslandX,fIslandY,fIslandZ);
    glRotatef(0, 0.0f, 0.0f, 1.0f);

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.0f,1.0f,0.0f);
    glVertex3f(0.0f,height,0.0f);

    for(angle = 0.0f,iPivot=0; iPivot<=precision;iPivot++,angle += (PI/(precision/2)))
    {
    	x = diameter*sin(angle);
    	y = diameter*cos(angle);

    	/**if ((iPivot % 2==0))
    		glColor3f(0.0f,1.0f,0.0f);
    	else**/
    		glColor3f(1.0f,1.0f,0.1f);

    	glVertex3f(x,0.0f,y);
    }

    glEnd();
    glPopMatrix();
}

void drawIsland2(float fIslandX, float fIslandY, float fIslandZ)
{
    //glLoadIdentity();
    glPushMatrix();
    glTranslatef(fIslandX,fIslandY,fIslandZ);
    glRotatef(0, 0.0f, 0.0f, 1.0f);



    int sideLength = 129;	// Accepts 2^n+1, for (4 <= n <= 10).
    TFracVal* buff = new TFracVal[sideLength*sideLength];
    CFractalNoise myNoise(buff, sideLength);

    for (int i=0 ; i<sideLength*sideLength ; i++)
    	buff[i] = FRACVAL_UNINIT;

    int seed = 13;
    float adjust = 10.0f;
    float frequency = 2.5f;
    myNoise.Generate(seed, adjust, frequency);

    for(int i=0;i<sideLength;i++)
    	for(int j=0;j<sideLength;j++)
    	{
    	    glBegin(GL_QUADS);
    	    glColor3f(0.0f, 1.0f, 0.0f);
    	    glVertex3f( i*1.0f,  myNoise.GetVal(i,j), j*1.0f);    // C
    	    glVertex3f( (i+1)+1.0f,myNoise.GetVal(i,j), j*1.0f);
    	    glVertex3f( (i+1)+1.0f,myNoise.GetVal(i,j), (j+1)*1.0f);
    	    glVertex3f( (i)+1.0f,myNoise.GetVal(i,j), (j+1)*1.0f);

    	    glEnd();
    	}

    glEnd();
    glPopMatrix();
}


void drawIsland(float fIslandX, float fIslandY, float fIslandZ)
{
    //glLoadIdentity();
    glPushMatrix();
    glTranslatef(fIslandX,fIslandY,fIslandZ);
    glRotatef(0, 0.0f, 0.0f, 1.0f);

    glBegin(GL_TRIANGLES);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-100.0f, 0.6f, 0.0f);    // A
 
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f( 100.0f, 0.6f, 0.0f);    // B

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f( 0.0f,  0.6f, 100.0f);    // C

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f( 0.0f,  0.6f, -100.0f);    // D

    glEnd();
    glPopMatrix();

     tangle += 0.5f;

    if (tangle >= 360.f)
        tangle = 0.0f;

}

static void drawSky (float view_xyz[3])
{
	float sky_scale=10.0f;
	float sky_height=10.0f;
  glDisable (GL_LIGHTING);
  /**if (use_textures) {
    glEnable (GL_TEXTURE_2D);
    sky_texture->bind (0);
  }**/
  //else {
    glDisable (GL_TEXTURE_2D);
    glColor3f (0,0.5,1.0);
  //}

  // make sure sky depth is as far back as possible
  glShadeModel (GL_FLAT);
  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);
  glDepthRange (1,1);

  const float ssize = 1000.0f;
  static float offset = 0.0f;

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

void drawSea(float x, float y, float z)
{
    //glLoadIdentity();
    glPushMatrix();
    glBegin(GL_QUADS);
    
    glNormal3f(0.0f,1.0f,0.0f);
    glColor3f(0.2f,0.2f,1.0f);
    glVertex3f(x-1000 / 2, 0, z-1000 / 2);
    glVertex3f(x+1000 / 2, 0, z-1000 / 2);
    glVertex3f(x+1000/ 2, 0, z+1000 / 2);
    glVertex3f(x-1000 / 2, 0, z+1000 / 2);
    
    glEnd();
    
    glPointSize(3.0f);

    glBegin(GL_POINTS);

    glNormal3f(0.0f,1.0f,0.0f);
    glColor3f(1.0f,1.0f,1.0f);
    for(int i=-2000/8;i<2000/8;i+=10) 
    {
        for (int j=-2000/8;j<2000/8;j+=10) 
        {
            glVertex3f(x+i,0.5f,z+j);
        }
    }
    
    
    glEnd();    
    glPopMatrix();
}

float boxangle = 0;

void drawhArrow()
{
    glPushMatrix();
    glLineWidth(3.0f);

    // RED
    glTranslatef(0.0f,0.0f,0.0f);
    glColor3f(1.0f,0.0f,0.0f);
    glBegin(GL_LINE);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(3.0f,0.0f,0.0f);
    glEnd();

    glTranslatef(3.0f,0.0f,0.0f);
    glRotatef(90.0f,0.0f,1.0f,0.0f);
    glutSolidCone(0.100,0.5f,10,10);

    glPopMatrix();

    // GREEN
    glPushMatrix();
    glTranslatef(0.0f,0.0f,0.0f);
    glColor3f(0.0f,1.0f,0.0f);
    glBegin(GL_LINE);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(0.0f,3.0f,0.0f);
    glEnd();

    glTranslatef(0.0f,3.0f,0.0f);
    glRotatef(-90.0f,1.0f,0.0f,0.0f);
    glutSolidCone(0.100,0.5f,10,10);

    glPopMatrix();

    // BLUE
    glPushMatrix();
    glTranslatef(0.0f,0.0f,0.0f);
    glColor3f(0.0f,0.0f,1.0f);
    glBegin(GL_LINE);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(0.0f,0.0f,3.0f);
    glEnd();

    glTranslatef(0.0f,0.0f,3.0f);
    glRotatef(-90.0f,0.0f,0.0f,1.0f);
    glutSolidCone(0.100,0.5f,10,10);

    glPopMatrix();

}

void drawBox(float xx, float yy, float zz)
{
    int x=0, y=0, z=0;
    
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


void drawModel(MD2Model* _model, float yRot, float xRot, float x, float y, float z, int iRotate=0)
{
    //Draw the saved model
    if (_model != NULL) {
        glPushMatrix();
        glTranslatef(x, y, z);
        if (iRotate==1)
        {
        	glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        	glRotatef(-180.0f, 0.0f, 0.0f, 1.0f);
            glRotatef(yRot, 0.0f, 0.0f, 1.0f);
            if (fabs(yRot)>10.0f) glRotatef(0.10f,0.0f,1.0f,0.0f);
        }
        else
        {
        	glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(yRot, 0.0f, 1.0f, 0.0f);
        }
        glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        //glTranslatef(50.0f,0.0f,0.0f);
        //glScalef(4.0f, 4.5f, 4.5f);
        _model->draw();
        drawArrow();
        glPopMatrix();
    } else { printf ("model is null\n"); }    
}








    //Get upward and forward vector, convert vectors to fixed coordinate sstem (similar than for translation 1)
 /**   Vector3f up = toVectorInFixedSystem1(0.0f, 1.0f, 0.0f);        //Note: need to calculate at each frame
    Vector3f forward = toVectorInFixedSystem1(0.0f, 0.0f, 1.0f);
    Vector3f pos = getCameraPosition();**/

    /*
     * Read Lesson 02 for more explanation of gluLookAt.
     */
/**    glDrawable.getGLU().gluLookAt(
        //Position
        getPosition().getX(),
        getPosition().getY(),
        getPosition().getZ(),
       
        //View 'direction'
        getPosition().getX()+forward.getX(),
        getPosition().getY()+forward.getY(),
        getPosition().getZ()+forward.getZ(),
       
        //Upward vector
        up.getX(), up.getY(), up.getZ());
**/
void drawWalrus(MD2Model* _model, float yRot, float xRot, float x, float y, float z)
{
    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);
       	glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(yRot, 0.0f, 1.0f, 0.0f);

        glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        _model->draw();
        drawArrow();
        glPopMatrix();
    }
    else
    {
    	printf ("model is null\n");
    }
}

void drawManta(MD2Model* _model, float yRot, float xRot, float x, float y, float z)
{
    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);
		glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(-180.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(yRot, 0.0f, 0.0f, 1.0f);

        glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        _model->draw();
        drawArrow();
        glPopMatrix();
    }
    else
    {
    	printf ("model is null\n");
    }
}

void drawWalrus(float fBaseX, float fBaseY, float fBaseZ)
{
	static float modX=100.0f, modY=15.0f, modZ=0.0f;

    Vec3f forward = toVectorInFixedSystem(0, 0, -0.1,0.0f,0.0f);

    modX+=spd*forward[0]; modY+=spd*forward[1];modZ+=spd*forward[2];

    drawModel(_modelWalrus,0.0f,0.0f,modX,modY,modZ);
}
void drawManta(float fBaseX, float fBaseY, float fBaseZ)
{
    static float modX=0.0f, modY=15.0f, modZ=0.0f;

    Vec3f forward = toVectorInFixedSystem(0, 0, -0.1,-modAngleX,modAngleY);

    modX+=spd*forward[0]; modY+=spd*forward[1];modZ+=spd*forward[2];
    drawModel(_modelManta, modAngleX, modAngleY, modX, modY, modZ,1);
}




GLvoid glPrint(GLuint glfontlist, GLfloat x, GLfloat y, const char *fmt, ...)
{
	if( glIsList( glfontlist ) == GL_FALSE )
	{		// Error Report?
		return;
	}
	glRasterPos2f( x, y );
	char text[256];	va_list ap;
	if(fmt==NULL) return;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);
	glPushAttrib(GL_LIST_BIT);
	glListBase(glfontlist - 32);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}

void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	



	//glTranslatef(0.0f+posx, 0.0f+posy, -20.0f+posz);
	
    // Lighting, ambient light...
	GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    
    // Add positioned light (outisde glBegin-glEnd)
    /**GLfloat lightColor0[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat lightPos0[] = { 4.0f, 0.0f, 8.0f, 1.0f  };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    
    // Add directed light
    GLfloat lightColor1[] = {0.5f, 0.2f, 0.2f, 1.0f};
    GLfloat lightPos1[] = {-1.0f, 0.5f, 0.5f, 0.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor1);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
	
	GLfloat lightColor[] = {0.7f, 0.7f, 0.7f, 1.0f};
	GLfloat lightPos[] = {-2 * BOX_SIZE, BOX_SIZE, 4 * BOX_SIZE, 1.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);**/
	
    
    
    Vec3f up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,_angle,-_angleY);        //Note: need to calculate at each frame
    Vec3f forward = toVectorInFixedSystem(speedheight, speedwidth, speed,_angle,-_angleY);
    Vec3f pos(0.0f, 0.0f, -20.0f);
       

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
        
    xxx+=(forward[0]);
    yyy+=(forward[1]);
    zzz+=(forward[2]);
    


    //drawString(0+xxx,0-yyy,10.0+zzz,str,10.0f);
    
    drawArrow();
    drawBox(10,10,-40);
    drawBox(10,20,-80);
    
    drawBox(10,10,10);
    drawBox(-10,-10,-10);
    
    //drawSea(0,0,0);
    drawFloor();
    
    float xyz[3];

    xyz[0]=0.0f;
    xyz[1]=0.0f;
    xyz[2]=0.0f;


    //drawModel(_modelManta,boxangle, 0.0f,0.0f,35.0f,0.0f);
    drawWalrus(0.0f,10.0f,0.0f);
    drawManta(0.0f,-0.6f,0.0f);
    
    /**
    float islandCenter = 900.0f;
    coneIsland(30,15.0f, islandCenter+20.0f,islandCenter+0.0f,islandCenter+230.0f,islandCenter+32.0f);
    islandCenter = -200.0f;
    coneIsland(97.0f, 30.0f, islandCenter+70.0f, islandCenter+0.0f, islandCenter+270.0f, islandCenter+32.0f);**/

    //drawTerrain(600.0f);
    
    drawHUD();


    
	glDisable(GL_TEXTURE_2D);
	
	glutSwapBuffers();
}

void initRendering() {
	// Lightning

	glEnable(GL_LIGHTING);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	// Normalize the normals.
	glEnable(GL_NORMALIZE);


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_CULL_FACE);

	//Load the model
    _modelManta = MD2Model::load("units/manta.md2");
    if (_modelManta != NULL)
        _modelManta->setAnimation("run");

    _modelWalrus = MD2Model::load("units/walrus.md2");
    if (_modelWalrus != NULL)
        _modelWalrus->setAnimation("run");


	Image* image = loadBMP("vtr.bmp");

	// _textureId points to the texture structure where the texture is located (read from the image)
	_textureId = loadTexture(image);
	delete image;

	image = loadBMP("sea.bmp");
	_textureIdSea = loadTexture(image);

	_terrain = loadTerrain("terrain/heightmap.bmp", 20);

	// Blue sky !!!
	glClearColor(0.7f, 0.9f, 1.0f, 1.0f);
}

void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)w / (float)h, 1.0, 1450.0f + yyy);
}

//Called every 25 milliseconds
void update(int value) {
	//_angle += 1.0f;
	//if (_angle > 360) {
	//	_angle -= 360;
	//}
    
        
    //_angle = atan( (1.0f + _deltax) / (40) ) * 180 / PI; 
    
    
    //Advance the animation
    //if (_model != NULL) {
        //_model->advance(0.025f);
    //}   //Advance the animation

	  int i;
	  //if (!pause) {
	    static double angle = 0;
	    angle += 0.05;
	    //dBodyAddForce (body[NUM-1],0,0,1.5*(sin(angle)+1.0));

	    dSpaceCollide (space,0,&nearCallback);
	    dWorldStep (world,0.05);

	    /* remove all contact joints */
	    dJointGroupEmpty (contactgroup);
	  //}

	  //dsSetColor (1,1,0);
	  //dsSetTexture (DS_WOOD);
	  /**for (i=0; i<NUM; i++) dsDrawSphere (dBodyGetPosition(body[i]),
					      dBodyGetRotation(body[i]),RADIUS);**/
	  const dReal *dr = dBodyGetPosition(body[0]);

	  printf ("(%10.6f,%10.6f,%10.6f)\n", dr[0],dr[1],dr[2]);
    
	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}




void initWorldModelling()
{
	dReal k;
	dMass m;

	/* create world */
	dInitODE2(0);
	world = dWorldCreate();
	space = dHashSpaceCreate (0);
	contactgroup = dJointGroupCreate (1000000);
	dWorldSetGravity (world,0,0,-0.5);
	dCreatePlane (space,0,0,1,0);

	body[0] = dBodyCreate(world);
	dBodySetPosition(body[0],0,0,10);
	dMassSetBox(&m,1,SIDE,SIDE,SIDE);
	dMassAdjust(&m, MASS);
	dBodySetMass(body[0],&m);
	sphere[0] = dCreateSphere( space, RADIUS);
	dGeomSetBody(sphere[0], body[0]);

	/**for (i=0; i<NUM; i++) {
		body[i] = dBodyCreate (world);
		k = i*SIDE;
		dBodySetPosition (body[i],k,k,k+0.4);
		dMassSetBox (&m,1,SIDE,SIDE,SIDE);
		dMassAdjust (&m,MASS);
		dBodySetMass (body[i],&m);
		sphere[i] = dCreateSphere (space,RADIUS);
		dGeomSetBody (sphere[i],body[i]);
	}
	for (i=0; i<(NUM-1); i++) {
		joint[i] = dJointCreateBall (world,0);
		dJointAttach (joint[i],body[i],body[i+1]);
		k = (i+0.5)*SIDE;
		dJointSetBallAnchor (joint[i],k,k,k+0.4);
	}

	/* run simulation */
	/**
	dsSimulationLoop (argc,argv,352,288,&fn);

	dJointGroupDestroy (contactgroup);
	dSpaceDestroy (space);
	dWorldDestroy (world);
	dCloseODE();**/
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1200, 800);

	glutCreateWindow("Carrier Command");
	initRendering();

    //dAllocateODEDataForThread(dAllocateMaskAll);
	initWorldModelling();

	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutSpecialFunc(handleSpecKeypress);

	glutReshapeFunc(handleResize);

	//adding here the mouse processing callbacks
	glutMouseFunc(processMouse);
	glutMotionFunc(processMouseActiveMotion);
	glutPassiveMotionFunc(processMousePassiveMotion);
	glutEntryFunc(processMouseEntry);

	// this is the first time to call to update.
	glutTimerFunc(25, update, 0);

	// main loop, hang here.
	glutMainLoop();
	return 0;
}









