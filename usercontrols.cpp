/*
 * usercontrols.cpp
 *
 *  Created on: Dec 12, 2011
 *      Author: faturita
 */

#include <GLUT/glut.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>
#include <iostream>

#include "math/yamathutil.h"

#include "container.h"

#include "camera.h"

#include "sounds/sounds.h"
#include "terrain/Terrain.h"

#include "engine.h"

#include "usercontrols.h"
#include "map.h"

#include "units/Vehicle.h"
#include "units/Balaenidae.h"
#include "units/Manta.h"
#include "units/Walrus.h"

#include "structures/Runway.h"
#include "structures/CommandCenter.h"
#include "structures/Turret.h"

Camera Camera;

extern dWorldID world;
extern dSpaceID space;

Controller controller;

extern std::vector<std::string> messages;

extern container<Vehicle*> entities;

// Mouse offset for camera zoom in and out.
int _xoffset = 0;
int _yoffset = 0;

// Right, left or middle button pressed.
int buttonState;

void processMouseEntry(int state) {
    /**if (state == GLUT_LEFT)
        _angle = 0.0;
    else
        _angle = 1.0;**/
    
}


void processMouse(int button, int state, int x, int y) {


    int specialKey = glutGetModifiers();
    // if both a mouse button, and the ALT key, are pressed  then


    // This should not be here @FIXME
    //glutSetCursor(GLUT_CURSOR_NONE);


    if ((state == GLUT_DOWN) && button == GLUT_LEFT_BUTTON)
    {
        if (controller.controlling != CONTROLLING_NONE &&  entities[controller.controlling]->getType() == VehicleTypes::MANTA)
        {
            printf("Active control\n");
            // Activate airplane controller.
            if (buttonState != 1)
            {
                buttonState = 1;
                glutSetCursor(GLUT_CURSOR_NONE);
            }
            else
            {
                buttonState = 0;
                glutSetCursor(GLUT_CURSOR_CROSSHAIR);
            }

            _xoffset = x;
            _yoffset = y;
            return;
        }
    }


    if ((state == GLUT_DOWN)) {

        _xoffset = _yoffset = 0;

        if (GLUT_RIGHT_BUTTON == button)
        {
        	//buttonState = 0;
            zoommapout();
        }
        // set the color to pure red for the left button
        if (button == GLUT_LEFT_BUTTON) {
			//buttonState = 1;
            printf("Mouse down %d,%d\n",x,y);
            if (controller.view == 2)
            {
                if (specialKey == GLUT_ACTIVE_SHIFT)
                {
                    Vec3f target = setLocationOnMap(x,y);
                    //@FIXME
                    entities[controller.controlling]->setDestination(target);

                    printf("Destination set to (%10.2f,%10.2f,%10.2f)\n", target[0],target[1],target[2]);


                } else {
                    centermap(x,y);
                    zoommapin();
                }
            }
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

    //if (buttonState == 1)
    {
    	Camera.xAngle += ( (x-_xoffset) * 0.005);

        Camera.yAngle += ( (y - _yoffset ) * 0.005) ;
    } //else if (buttonState == 0)
    {
    	//Vec3f forward = Camera.getForward();
    	//Vec3f pos = Camera.getPos();
    	//Camera.setPos(  ( (y - _yoffset ) * 0.05)*forward+pos   );
    	
        
        //Camera.dx += ( (y - _yoffset ) * 0.05) ;
    }
    //}
}




// Movement of the mouse alone.
void processMousePassiveMotion(int x, int y) {

    if (buttonState==1)
    {
        controller.registers.pitch = ( (y-_yoffset) * 0.08);
        controller.registers.roll = ( (x - _xoffset ) * 0.05) ;
    }



    //int specialKey = glutGetModifiers();
    // User must press the SHIFT key to change the
    // rotation in the X axis
    //if (specialKey != GLUT_ACTIVE_SHIFT) {

        // setting the angle to be relative to the mouse
        // position inside the window
    //if (_xoffset ==0 ) _xoffset = x;

    //if (_yoffset == 0 ) _yoffset = y;

    //if (buttonState == 1)
    {
    //    Camera.xAngle += ( (x-_xoffset) * 0.005);

    //    Camera.yAngle += ( (y - _yoffset ) * 0.005) ;
    }
    //processMouseActiveMotion(x,y);
}


void switchControl(int controlposition)
{
    if (controlposition > entities.size())
    {
        controller.controlling = CONTROLLING_NONE;
        return;
    }
    size_t id = entities.indexAt(controlposition);

    if (!entities.isValid(id))
    {
        controller.controlling = CONTROLLING_NONE;
        return;
    }

    // Check if it is controllable ?
    int type=entities[id]->getType();
    if (type == ACTION)
    {
        return;
    }

    if (controller.controlling != CONTROLLING_NONE)
    {
        if (!entities[controller.controlling]->isAuto())
            entities[controller.controlling]->setControlRegisters(controller.registers);
    }

    controller.controlling = id;
    //controller.reset();
    controller.registers = entities[controller.controlling]->getControlRegisters();
}


bool pp=false;

void handleKeypress(unsigned char key, int x, int y) {
    if (controller.isTeletype())
    {
        if (key == 13)
        {
            // Analyze commands.
            size_t n = controller.str.find("control");

            if (n != std::string::npos)
            {
                // @FIXME input data should be verified.
                const char *content = controller.str.substr(7).c_str();

                printf("Controlling %s\n", content);

                switchControl(atoi(content));

            } else
            if (controller.str.find("taxi") != std::string::npos)
            {
                Balaenidae *r = (Balaenidae*)entities[controller.controlling];
                Manta *m = findManta(Manta::ON_DECK);
                if (m)
                {
                    r->taxi(m);
                    char msg[256];
                    sprintf(msg,"Manta %2d is ready for launch.",m->getNumber()+1);
                    messages.insert(messages.begin(), std::string(msg));
                }
            }
            else
            if (controller.str.find("launch") != std::string::npos)
            {
                //const char* content = controller.str.substr(7).c_str();

                launchManta(entities[controller.controlling]);

            } else
            if (controller.str.find("command") != std::string::npos)
            {
                Walrus *w = (Walrus*) entities[controller.controlling];
                // Chek if walrus is on island.
                BoxIsland *island = w->getIsland();

                captureIsland(island,w->getFaction(),space, world);

            } else
            if (controller.str.find("list") != std::string::npos)
            {
                list();
            } else

            if (strcmp(controller.str.c_str(),"map")==0)
                controller.view = 2;
            else {
                // Send message to message board
                messages.insert(messages.begin(),controller.str);

            }

            if (messages.size()>5)
                messages.pop_back();

            controller.teletype = false;
            controller.str.clear();


        } else {
            controller.str += key;
        }
    } else

    switch (key) {
		case 27: //Escape key
            controller.interrupt();
        case '+':Camera.dx+=0.1;break;
        case '-':Camera.dx-=0.1;break;
        case 32 :Camera.dx=0.00001; controller.registers.thrust = 0;break;
        case '^':pp = !(pp);break;
        case 'a':case 'k':controller.registers.roll-=1.0f;break;
        case 'A':case 'K':controller.registers.roll-=50.0f;break;
        case 'd':case 'l':controller.registers.roll+=1.0f;break;
        case 'D':case 'L':controller.registers.roll+=50.0f;break;
        case '`':controller.registers.roll=0;break;
        case 'w':controller.registers.pitch-=1.0f;break;
        case 's':controller.registers.pitch+=1.0f;break;
        case 'z':controller.registers.yaw-=1.0f;break;
        case 'c':controller.registers.yaw+=1.0f;break;
        case 'v':controller.registers.precesion-=1.0f;break;
        case 'b':controller.registers.precesion+=1.0f;break;
        case 'p':controller.pause = !controller.pause;break;
        case 'r':controller.registers.thrust+=0.05;break;
        case 'R':controller.registers.thrust+=10.00;break;
        case 'f':controller.registers.thrust-=0.05;break;
        case 'F':controller.registers.thrust-=10.00;break;
        case 'q':controller.reset();break;
        case 'Q':controller.registers.thrust = 0.0;break;
        case 'j':entities[controller.controlling]->enableAuto();break;
        case 'J':entities[controller.controlling]->disableAuto();break;

        case '0':
            controller.controlling = CONTROLLING_NONE;
            controller.reset();
            Camera.reset();
        break;
        case '1':case '2':case '3': case '4': case '5': case '6':case '7':case '8':case '9':
            switchControl((int)(key-48));
        break;
        case 'i':
            {
            int param = 0;
            std::cout << "Param:" << std::endl; std::cin >> param;
            std::cout << "Value:" << std::endl; std::cin >> controller.param[param] ;
            }
        break;
        case '!':( (controller.view == 1)?controller.view=2:controller.view=1);break;
        case '@':controller.view = 3;break;
        case '~':Camera.control = 0;break;
        case '?':gltWriteTGA("file.tga");break;
        case 't':controller.teletype = true;break;
        case 'S':
            {
            Vehicle *v = entities[controller.controlling];
            v->stop();
            break;
            }
        case 'm':
            {
            synchronized(entities.m_mutex)
            {
                if (entities[controller.controlling]->getType()==CARRIER)
                {
                    spawnManta(space,world,entities[controller.controlling]);
                }
            }
            }
            break;
        case 'M':
            {
                synchronized(entities.m_mutex)
                {
                    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
                    {
                        //printf("Type and ttl: %d, %d\n", vehicles[i]->getType(),vehicles[i]->getTtl());
                        if (entities[i]->getType()==MANTA && entities[i]->getStatus()==Manta::ON_DECK)
                        {
                            //printf("Eliminating....\n");
                            dBodyDisable(entities[i]->getBodyID());
                            entities.erase(i);
                            messages.insert(messages.begin(), std::string("Manta is now on bay."));
                        }
                    }
                }
            }
        break;
        case 'o':
            {
                spawnWalrus(space,world,entities[controller.controlling]);
            }
            break;
        case 'O':
            {
                // @FIXME: Find the walrus that is actually closer to the dock bay.
                synchronized(entities.m_mutex)
                {
                    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
                    {
                        //printf("Type and ttl: %d, %d\n", vehicles[i]->getType(),vehicles[i]->getTtl());
                        if (entities[i]->getType()==WALRUS && entities[i]->getStatus()==Walrus::SAILING)
                        {
                            //printf("Eliminating....\n");
                            dBodyDisable(entities[i]->getBodyID());
                            entities.erase(i);
                            messages.insert(messages.begin(), std::string("Walrus is now back on deck."));
                        }
                    }
                }
            }
        break;
        case 'h':
            Vehicle *action = (entities[controller.controlling])->fire(world,space);
            //int *idx = new int();
            //*idx = vehicles.push_back(action);
            //dBodySetData( action->getBodyID(), (void*)idx);
            if (action != NULL)
            {
                entities.push_back(action);
                gunshot();
            }
        break;
	}
}

void handleSpecKeypress(int key, int x, int y)
{
    //specialKey = glutGetModifiers();

    switch (key) {
        case GLUT_KEY_LEFT :
            ;break;
        case GLUT_KEY_RIGHT :
            ;break;
        case GLUT_KEY_UP :
            ;break;
        case GLUT_KEY_DOWN :
            ;break;
    }

}


// ------------------------
typedef struct
{
	unsigned char idLength;
	unsigned char colorMapType;
	unsigned char imageTypeCode;
	unsigned char colorMapSpec[5];
	unsigned short xOrigin;
	unsigned short yOrigin;
	unsigned short width;
	unsigned short height;
	unsigned char bpp;
	unsigned char imageDesc;
} tgaheader;

typedef struct
{
    GLubyte header[6];          // Holds The First 6 Useful Bytes Of The File
    GLuint bytesPerPixel;           // Number Of BYTES Per Pixel (3 Or 4)
    GLuint imageSize;           // Amount Of Memory Needed To Hold The Image
    GLuint type;                // The Type Of Image, GL_RGB Or GL_RGBA
    GLuint height;              // Height Of Image
    GLuint width;               // Width Of Image
    GLuint bits;             // Number Of BITS Per Pixel (24 Or 32)
} TGAHEADER;

GLbyte *gltReadTGABits(const char *szFileName, GLint *iWidth, GLint *iHeight,
		GLint *iComponents, GLenum *eFormat)
{
	FILE		*pFile;
	tgaheader	tgaHeader;
	unsigned long lImageSize;
	short		sDepth;
	GLbyte		*pBits = NULL;

	*iWidth = 0;
	*iHeight = 0;
	*eFormat = GL_RGB;
	*iComponents = GL_RGB;


	// Open the file
	pFile = fopen(szFileName, "rb");
	if (pFile == NULL)
		return NULL;

	fread(&tgaHeader, 18/* sizeof(TGAHEADER) */,1,pFile);

	*iWidth = tgaHeader.width;
	*iHeight = tgaHeader.height;
	sDepth = tgaHeader.bpp / 8;

	if (tgaHeader.bpp != 8 && tgaHeader.bpp != 24 && tgaHeader.bpp != 32)
		return NULL;

	lImageSize = tgaHeader.width * tgaHeader.height * sDepth;

	pBits = (GLbyte*) malloc(lImageSize * sizeof(GLbyte));

	if (pBits == NULL)
		return NULL;

	if (fread(pBits, lImageSize, 1, pFile)!=1)
	{
		free(pBits);
		return NULL;

	}

	switch (sDepth)
	{
	case 3:
		*eFormat = GL_BGR;
		*iComponents = GL_RGB;
		break;
	case 4:
		*eFormat = GL_BGRA;
		*iComponents = GL_RGBA;
		break;
	case 1:
		*eFormat = GL_LUMINANCE;
		*iComponents = GL_LUMINANCE;
		break;
	default:
		break;
	}

	fclose(pFile);

	return pBits;
}




GLint gltWriteTGA(const char *szFileName)
{
	FILE *pFile;
	tgaheader tgaHeader;
	unsigned long lImageSize;
	GLbyte *pBits = NULL;
	GLint iViewport[4];
	GLint lastBuffer;

	//Get the viewport dimension
	glGetIntegerv(GL_VIEWPORT, iViewport);

	lImageSize = iViewport[2] * 3 * iViewport[3];

	pBits = (GLbyte*)malloc(lImageSize);

	if (pBits == NULL)
		return 0;

	glPixelStorei(GL_PACK_ALIGNMENT,1);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS,0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

	glGetIntegerv(GL_READ_BUFFER, &lastBuffer);
	glReadBuffer(GL_FRONT);
	glReadPixels(0,0,iViewport[2], iViewport[3], GL_BGR, GL_UNSIGNED_BYTE, pBits);
	glReadBuffer(lastBuffer);

	tgaHeader.imageTypeCode = 2;
	tgaHeader.bpp = 24;
	tgaHeader.width = iViewport[2];
	tgaHeader.height = iViewport[3];

	pFile = fopen(szFileName, "wb");
	if (pFile == NULL)
	{
		free(pBits);
		return 0;

	}

	fwrite(&tgaHeader, sizeof(tgaheader), 1, pFile);

	fwrite(pBits, lImageSize, 1, pFile);

	free(pBits);
	fclose(pFile);

	return 1;
}



