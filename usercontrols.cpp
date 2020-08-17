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
#include "units/AdvancedWalrus.h"

#include "structures/Runway.h"
#include "structures/CommandCenter.h"
#include "structures/Turret.h"

Camera Camera;

extern dWorldID world;
extern dSpaceID space;

Controller controller;

extern std::vector<Message> messages;

extern container<Vehicle*> entities;

extern std::vector<BoxIsland*> islands;

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
        if (controller.view == 1 && controller.controllingid != CONTROLLING_NONE &&
                ((entities[controller.controllingid]->getType() == VehicleTypes::MANTA) || entities[controller.controllingid]->getType() == CONTROLABLEACTION) )
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
                    entities[controller.controllingid]->setDestination(target);

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
        controller.registers.pitch = ( (y-_yoffset) * 0.08);   // @FIXME these parameters should be dependant on what are you moving.
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
        controller.controllingid = CONTROLLING_NONE;
        return;
    }
    size_t id = entities.indexAt(controlposition);

    if (!entities.isValid(id))
    {
        controller.controllingid = CONTROLLING_NONE;
        return;
    }

    // Check if it is controllable ?
    int type=entities[id]->getType();
    if (type == ACTION)
    {
        return;
    }

    // Check if it is from users faction
    if (!(entities[id]->getFaction() == controller.usercontrolling || controller.usercontrolling == BOTH_FACTION))
    {
        controller.controllingid = CONTROLLING_NONE;
        return;
    }

    if (controller.controllingid != CONTROLLING_NONE)
    {
        if (!entities[controller.controllingid]->isAuto())
            entities[controller.controllingid]->setControlRegisters(controller.registers);
    }

    controller.controllingid = id;
    //controller.reset();
    controller.registers = entities[controller.controllingid]->getControlRegisters();

    // Release mouse
    glutSetCursor(GLUT_CURSOR_CROSSHAIR);
    buttonState = 0;
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
            if (controller.str.find("walrus") != std::string::npos)
            {
                const char *content = controller.str.substr(6).c_str();

                printf ("Walrus %s\n", content);

                size_t pos = CONTROLLING_NONE;
                findWalrusByNumber(pos,atoi(content));
                switchControl(pos);

            } else
            if (controller.str.find("manta") != std::string::npos)
            {
                const char *content = controller.str.substr(5).c_str();

                size_t pos = CONTROLLING_NONE;
                findMantaByNumber(pos,atoi(content));

                printf ("Manta %d\n", pos);

                switchControl(pos);

            } else
            if (controller.str.find("def") != std::string::npos)
            {
                std::string islandcode = controller.str.substr(3+controller.str.substr(3).find(" "));
                std::string islandname = islandcode.substr(1,islandcode.find("#")-1);
                const char *structurenumber = controller.str.substr(controller.str.find("#")+1).c_str();

                std::cout << "Island-" << islandname << "-structure " << structurenumber << std::endl;

                size_t pos = CONTROLLING_NONE;

                for (int j=0;j<islands.size();j++)
                {
                    if (islandname == islands[j]->getName() && islands[j]->getStructures().size()>atoi(structurenumber))
                    {
                        std::cout << "Found-" << islandname << "-structure " << structurenumber << std::endl;
                        pos = entities.indexOf(islands[j]->getStructures()[atoi(structurenumber)]);
                    }
                }
                switchControl(pos);

            } else
            if (controller.str.find("taxi") != std::string::npos)
            {
                if (entities[controller.controllingid]->getType()==CARRIER)
                {
                    Balaenidae *r = (Balaenidae*)entities[controller.controllingid];
                    Manta *m = findManta(r->getFaction(),Manta::ON_DECK);
                    if (m)
                    {
                        r->taxi(m);
                        char msg[256];
                        Message mg;
                        mg.faction = m->getFaction();
                        sprintf(msg,"Manta %2d is ready for launch.",NUMBERING(m->getNumber()));
                        mg.msg = std::string(msg);
                        messages.insert(messages.begin(), mg);
                    }
                } else if (entities[controller.controllingid]->getType()==LANDINGABLE )
                {
                    Runway *r = (Runway*)entities[controller.controllingid];
                    Manta *m = findManta(r->getFaction(),Manta::LANDED);
                    if (m)
                    {
                        r->taxi(m);

                    }
                }
            }
            else
            if (controller.str.find("launch") != std::string::npos)
            {
                //const char* content = controller.str.substr(7).c_str();

                launchManta(entities[controller.controllingid]);

            } else
            if (controller.str.find("land") != std::string::npos)
            {
                landManta(entities[controller.controllingid]);

            } else
            if (controller.str.find("command") != std::string::npos)
            {
                int typeofisland = DEFEND_ISLAND;

                if (controller.str.find("factory") != std::string::npos)
                    typeofisland = FACTORY_ISLAND;

                if (controller.str.find("logistics") != std::string::npos)
                    typeofisland = LOGISTICS_ISLAND;

                if (entities.isValid(controller.controllingid) && entities[controller.controllingid]->getType()==WALRUS)
                {
                    Walrus *w = (Walrus*) entities[controller.controllingid];
                    // Check if walrus is on island.
                    BoxIsland *island = w->getIsland();

                    if (island)
                    {
                        captureIsland(w,island,w->getFaction(),typeofisland,space, world);
                    }
                }

            } else
            if (controller.str.find("godmode") != std::string::npos)
            {
                controller.usercontrolling = BOTH_FACTION;
            } else
            if (controller.str.find("greenmode") != std::string::npos)
            {
                controller.usercontrolling = GREEN_FACTION;
            } else
            if (controller.str.find("bluemode") != std::string::npos)
            {
                controller.usercontrolling = BLUE_FACTION;
            } else
            if (controller.str.find("save") != std::string::npos)
            {
                savegame();
            } else
            if (controller.str.find("list") != std::string::npos)
            {
                list();
            } else

            if (strcmp(controller.str.c_str(),"map")==0)
                controller.view = 2;
            else {
                // Send message to message board
                Message mg;
                mg.faction = BOTH_FACTION;
                mg.msg = std::string(controller.str);
                messages.insert(messages.begin(),mg);

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
        case 'j':entities[controller.controllingid]->enableAuto();break;
        case 'J':entities[controller.controllingid]->disableAuto();break;

        case '0':
            controller.controllingid = CONTROLLING_NONE;
            controller.reset();
            Camera.reset();
        break;
        case '1':case '2':
        {
            switchControl((int)(key-48));
        }
        break;
        case '3': case '4': case '5':
        {
            size_t pos = CONTROLLING_NONE;
            findMantaByNumber(pos,(int)(key-48)-2);

            switchControl(pos);
        }
        break;
        case '6':case '7':case '8':case '9':
        {
            size_t pos = CONTROLLING_NONE;
            findWalrusByNumber(pos,(int)(key-48)-5);

            switchControl(pos);
        }
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
            Vehicle *v = entities[controller.controllingid];
            v->stop();
            break;
            }
        case 'm':
            {
            synchronized(entities.m_mutex)
            {
                if (entities[controller.controllingid]->getType()==CARRIER || entities[controller.controllingid]->getType()==LANDINGABLE )
                {
                    size_t idx = 0;
                    spawnManta(space,world,entities[controller.controllingid],idx);
                }
            }
            }
            break;
        case 'M':
            {
            synchronized(entities.m_mutex)
            {
                dockManta();
            }
            }
        break;
        case 'o':
            {
                spawnWalrus(space,world,entities[controller.controllingid]);
            }
            break;
        case 'O':
            {
                // @FIXME: Find the walrus that is actually closer to the dock bay.
                synchronized(entities.m_mutex)
                {
                    dockWalrus(entities[controller.controllingid]);
                }
            }
        break;
        case 'h':
            {
                synchronized(entities.m_mutex)
                {
                    if (controller.controllingid != CONTROLLING_NONE && entities.isValid(controller.controllingid))
                    {
                        Vehicle *action = (entities[controller.controllingid])->fire(world,space);
                        //int *idx = new int();
                        //*idx = vehicles.push_back(action);
                        //dBodySetData( action->getBodyID(), (void*)idx);
                        if (action != NULL)
                        {
                            size_t i = entities.push_back(action, action->getGeom());
                            gunshot();

                            if (action->getType()==CONTROLABLEACTION)
                            {
                                switchControl(entities.indexOf(i));

                            }
                        }


                    } else
                    {
                        controller.controllingid = CONTROLLING_NONE;
                    }
                }
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



