#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>
#include <math.h>

#include <GL/glut.h>

#include <ode/ode.h>

#include "FractalNoise.h"
#include "imageloader.h"
#include "md2model.h"
#include "carrier/yamathutil.h"
#include "terrain/Terrain.h"
#include "font/DrawFonts.h"

#include "odeutils.h"
#include "units/Manta.h"
#include "units/Walrus.h"



float modAngleY=0, modAngleX = 0, modAngleZ=0;

float modAngleP=0;

int control = 1;

// control = 1  Manta
// control = 2 Walrus

int camera = 0;

float thrust=0;
float spd = 0,spdX=0,spdZ=0;

float _angle = 0;
float _angleY = 0;
#include "camera.cpp"

using namespace std;




GLuint _textureId;           //The OpenGL id of the texture

GLuint _textureIdSea;

int _direction = 1;

MD2Model* _modelManta;
MD2Model* _modelWalrus;

Terrain* _terrain;
Terrain* _vulcano;

Terrain* _baltimore;

Terrain* _nemesis;

Manta _manta;

Walrus _walrus;

int specialKey = 0;

float posx=0, posz=0;

float speed=0.05;

float speedwidth=0.0;
float speedheight=0.0;


bool ctlPause = false;


void processMouseEntry(int state) {
    /**if (state == GLUT_LEFT)
        _angle = 0.0;
    else
        _angle = 1.0;**/
}



int _xoffset = 0;
int _yoffset = 0;
    



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

bool pp=false;

void handleKeypress(unsigned char key, int x, int y) {
	switch (key) {
		case 27: //Escape key
			exit(0);
        case '+':speed+=0.01;break;
        case '-':speed-=0.01;break;
        case 32 :speed=0; thrust = 0;break;
        case '^':pp = !(pp);break;
        case 'a':modAngleX-=0.1f;break;
        case 'd':modAngleX+=0.1f;break;
        case 'w':modAngleY-=0.01f;break;
        case 's':modAngleY+=0.01f;break;
        case 'z':modAngleZ-=0.1f;break;
        case 'c':modAngleZ+=0.1f;break;
        case 'v':modAngleP-=0.1f;break;
        case 'b':modAngleP+=0.1f;break;
        case '9':thrust+=20.0f;break;
        case 'p':ctlPause = !ctlPause;break;
        case 'r':thrust+=0.05;break;
        case 'f':thrust-=0.05;break;
        case 'q':modAngleY=modAngleX=modAngleZ=modAngleP=0;break;
        case '0':
        	control = 0;
        break;
        case '1':
        	control = 1;
        	spd = _manta.getThrottle();

        break;
        case '2':
        	control = 2;
        	spd = _walrus.getThrottle();

        break;
        case '!':camera = 1;break;
        case '"':camera = 2;break;
        case '~':camera = 0;break;
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
#define RADIdUS (0.1732f)	/* sphere radius */
#define RADIUS (1.0f)


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

  //printf ("Collision near\n");

  b1 = dGeomGetBody(o1);
  b2 = dGeomGetBody(o2);
  if (b1 && b2 && dAreConnected (b1,b2)) return;

  contact.surface.mode = dContactBounce ;
  contact.surface.mu = 0;
  contact.surface.mu2 = 1;
  contact.surface.bounce = 0;

  if (dCollide (o1,o2,1,&contact.geom,sizeof(dContactGeom))) {
    dJointID c = dJointCreateContact (world,contactgroup,&contact);
    dJointAttach (c,b1,b2);
  }
}

// **********************************************************************


//Makes the image into a texture, and returns the id of the texture
/**GLuint loadTexture(Image* image) {
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
}**/




float tangle = 0.0f;

void drawTerrain(Terrain *_landmass, float fscale,float r,float g,float b);

void drawTerrain(Terrain *_landmass, float fscale)
{
	drawTerrain(_landmass, fscale, 0.3f, 0.9f, 0.0f);
}

void drawTerrain(Terrain *_landmass, float fscale,float r,float g,float b)
{
	float scale = 1.0f;//fscale / max(_landmass->width() - 1, _landmass->length() - 1);
	glScalef(scale, scale, scale);
	glTranslatef(-(float)(_landmass->width() - 1) / 2,
				 0.0f,
				 -(float)(_landmass->length() - 1) / 2);

	glColor3f(r,g,b);
	for(int z = 0; z < _landmass->length() - 1; z++) {
		//Makes OpenGL draw a triangle at every three consecutive vertices
		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 0; x < _landmass->width(); x++) {
			Vec3f normal = _landmass->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, _landmass->getHeight(x, z), z);
			normal = _landmass->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, _landmass->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}
}


void drawFloor(float x, float y, float z)
{
	// Slide the texture always from the starting position to generate
	// the effect that we are moving...
	const float floor_size = 100.0f;

    float horizon = 1450.0f;   /// 10.0f es
    float floorSize = horizon / floor_size;
    
    float start = -floorSize-x/floor_size;
    float stop =   floorSize-x/floor_size;
    float sstart =-floorSize+z/floor_size;
    float sstop =  floorSize+z/floor_size;

    glPushMatrix();

    glTranslatef(x,y,z);
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





void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	


	GLfloat ambientLight[] = { 0.3f  , 0.3f, 0.3f,   1.0f};
	GLfloat diffuseLight[] = { 0.7f  , 0.7f, 0.7f,   1.0f};
	GLfloat specular[]     = { 1.0f  , 1.0f, 1.0f,   1.0f};
	GLfloat lightPos[]     = { -50.0f,50.0f,100.0f,  1.0f};

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR,specular);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	glEnable(GL_COLOR_MATERIAL);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);



	//glTranslatef(0.0f+posx, 0.0f+posy, -20.0f+posz);
	
    // Lighting, ambient light...
	//GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    
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
	
	Vec3f forward,pos;
	static Vec3f posi(0.0f, 30.0f, -70.0f);
	Vec3f up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

	Vec3f Up;

	Vec3f orig;

    switch (camera)
    {
    case 0:
        forward = toVectorInFixedSystem(speedheight, speedwidth, speed,_angle,-_angleY);

        lookAtFrom(posi, forward);

        posi[0]+=(forward[0]);
        posi[1]+=(forward[1]);
        posi[2]+=(forward[2]);

        pos = posi;
        break;
    case 1:
        pos = _manta.getPos();
        forward = _manta.getForward();



        //forward[0]=0.5;forward[1]=0;forward[2]=0.5;
        forward = forward.normalize();
        orig = pos;
        Up[0]=Up[2]=0;Up[1]=4;
        pos = pos - 10*forward + Up;
		forward = orig-pos;

        lookAtFrom(up,pos, forward);
        break;
    case 2:
        pos = _walrus.getPos();
        forward = _walrus.getForward();

        Up[0]=Up[2]=0;Up[1]=4;
        pos = pos + 1*forward+Up;
        lookAtFrom(up,pos, forward);
        break;
    default:
    	break;

    }

	//Look

    drawArrow();
    drawBox(10,10,-40);
    drawBox(10,20,-80);
    
    drawBox(10,10,10);
    drawBox(-10,-10,-10);
    
    drawBox(50,50,50);
    drawBox(113,45,22);
    drawBox(50,50,50);
    drawBox(313,45,22);
    drawBox(-32,13,-50);
    drawBox(13,45,222);
    drawBox(250,50,50);
    drawBox(87,45,222);

    //drawSea(0,0,0);
    drawFloor(pos[0],0.0f,pos[2]);

    float f[3];
    f[0]=pos[0];
    f[1]=pos[1];
    f[2]=pos[2];
    //drawSky(f);

    _manta.drawModel();
    _walrus.drawModel();

    
    float islandCenter = 1100.0f;
    coneIsland(30,15.0f, islandCenter+20.0f,0.0f,islandCenter+100.0f,32.0f);
    islandCenter = -1200.0f;
    coneIsland(97.0f, 30.0f, islandCenter+70.0f, 0.0f, islandCenter+270.0f, 32.0f);

    glPushMatrix();
    glTranslatef(640.0f, 0.0f, 340.0f);
    drawTerrain(_terrain, 600.0f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-940.0f, 0.0f, -740.0f);
    drawTerrain(_terrain,600.0f);
    glPopMatrix();


    glPushMatrix();
    glTranslatef(-1340.0f, 0.0f, -1740.0f);
    drawTerrain(_vulcano,600.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    drawTerrain(_baltimore,600.0f);
    glPopMatrix();


    glPushMatrix();
    glTranslatef(2340.0f, 0.0f, 1220.0f);
    drawTerrain(_nemesis,600.0f);
    glPopMatrix();


    drawHUD();


    
	glDisable(GL_TEXTURE_2D);
	
	glutSwapBuffers();
}

void initRendering() {
	// Lightning

	glEnable(GL_LIGHTING);

	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);
	//glEnable(GL_LIGHT2);

	// Normalize the normals (this is very expensive).
	glEnable(GL_NORMALIZE);


	// Do not show hidden faces.
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_COLOR_MATERIAL);

	// Do not show the interior faces....
	//glEnable(GL_CULL_FACE);




	_manta.init();
	_manta.setPos(0.0f,0.0f,300.0f);




	_walrus.init();
	_walrus.setPos(150.0f,0.0f, 0.0f);



	Image* image = loadBMP("vtr.bmp");

	// _textureId points to the texture structure where the texture is located (read from the image)
	_textureId = loadTexture(image);
	delete image;

	image = loadBMP("sea.bmp");
	_textureIdSea = loadTexture(image);

	_terrain = loadTerrain("terrain/island.bmp", 20);

	_vulcano = loadTerrain("terrain/vulcano.bmp",20);

	_baltimore = loadTerrain("terrain/baltimore.bmp", 40);

	_nemesis = loadTerrain("terrain/nemesis.bmp", 40);

	// Blue sky !!!
	glClearColor(0.7f, 0.9f, 1.0f, 1.0f);
}

void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)w / (float)h, 1.0, 1450.0f /**+ yyy**/);
}



void update(int value)
{
	// Derive the control to the correct object

	switch (control)
	{
	case 1:
		_manta.setThrottle(thrust);
		spd = _manta.getSpeed();
		//modAngleX = _manta.xRotAngle;
		//modAngleY = _manta.yRotAngle;
		if (pp)
		{
		modAngleX = _angle*0.01f;
		modAngleY = _angleY*0.01f;
		}
		_manta.setXRotAngle(modAngleX);
		_manta.setYRotAngle(modAngleY);
		_manta.rudder = modAngleP;
		_manta.elevator = modAngleZ;
		break;
	case 2:
		_walrus.setThrottle(thrust);
		spd = _walrus.getSpeed();
		_walrus.setXRotAngle(modAngleX);
		_walrus.setYRotAngle(modAngleY);
		break;
	}



	if (!ctlPause)
	{
		_manta.doDynamics(body[0]);
		_walrus.doDynamics(body[1]);



		// Ok, done with the dynamics

		dSpaceCollide (space,0,&nearCallback);
		dWorldStep (world,0.05);

		/* remove all contact joints */
		dJointGroupEmpty (contactgroup);
	}


	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}

void buildTerrainModel(dSpaceID space, Terrain *_landmass, float fscale,float xx,float yy,float zz)
{
	float slopeData[_landmass->width()*_landmass->length()];
	float scale = fscale / max(_landmass->width() - 1, _landmass->length() - 1);
	fscale = 10.0f;
	for(int z = 0; z < _landmass->length() - 1; z++) {

		for(int x = 0; x < _landmass->width(); x++) {
			slopeData[z*_landmass->width() +x] = _landmass->getHeight(x, z)*fscale;
		}
	}

    float xsamples = _landmass->width(),zsamples = _landmass->width(), xdelta = 10, zdelta =10;

    dHeightfieldDataID slopeHeightData = dGeomHeightfieldDataCreate (); // data geom

    float width = xsamples*xdelta; // 5 samples at delta of 1 unit
    float depth = zsamples*zdelta; // 5 samples at delta of 1 unit

    dGeomHeightfieldDataBuildSingle(slopeHeightData,slopeData,
    0,width,depth, xsamples, zsamples, 1.0f, 5.0f,10.0f, 0); // last 4

    //dGeomHeightfieldDataSetBounds (slopeHeightData, 0.0f, 100.0f); // sort

    dGeomID slopeHeightID = dCreateHeightfield(space, slopeHeightData, 1); // fff

    dGeomSetPosition(slopeHeightID,xx,yy,zz);


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
	dWorldSetGravity (world,0,-0.5f,0);
	dCreatePlane (space,0,1,0,0);

	Vec3f pos;
	pos = _manta.getPos();
	body[0] = dBodyCreate(world);
	printf ("%10.8f, %10.8f, %10.8f\n", pos[0],pos[1],pos[2]);
	dBodySetPosition(body[0],pos[0],pos[1],pos[2]);
	dMassSetBox(&m,1,SIDE,SIDE,SIDE);
	dMassAdjust(&m, MASS*1);
	dBodySetMass(body[0],&m);
	sphere[0] = dCreateSphere( space, RADIUS);
	dGeomSetBody(sphere[0], body[0]);


	Vec3f pos2;
	pos2 = _walrus.getPos();
	body[1] = dBodyCreate(world);
	//dBodySetPosition(body[1],100.0f,20.0f,0.0f);

	dBodySetPosition(body[1], pos2[0], pos2[1], pos2[2]);
	dMassSetBox(&m,1,SIDE,SIDE,SIDE);
	dMassAdjust(&m, MASS*3.0f);
	dBodySetMass(body[1],&m);
	sphere[1] = dCreateSphere( space, RADIUS);
	dGeomSetBody(sphere[1], body[1]);


    buildTerrainModel(space,_vulcano,600.0f,-1340.0f, 0.0f, -1740.0f);

    buildTerrainModel(space, _terrain,600.0f, 640.0f, 0.0f, 340.0f );

    buildTerrainModel(space, _terrain,600.0f, -940.0f, 0.0f, -740.0f );

    buildTerrainModel(space, _vulcano,600.0f, -1340.0f, 0.0f, -1740.0f );

    buildTerrainModel(space, _baltimore,600.0f, 0.0f, 0.0f, 0.0f);

    buildTerrainModel(space, _nemesis,600.0f, 2340.0f, 0.0f, 1220.0f );
}




int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	//glutInitWindowSize(1200, 800);

	glutCreateWindow("Carrier Command");
	glutFullScreen();

	//Initialize all the models and structures.
	initRendering();

    //dAllocateODEDataForThread(dAllocateMaskAll);
	initWorldModelling();

	// OpenGL callback functions.
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutSpecialFunc(handleSpecKeypress);

	// Resize callback function.
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









