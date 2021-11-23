/*
 * usercontrols.cpp
 *
 *  Created on: Dec 12, 2011
 *      Author: faturita
 */

#ifdef __APPLE__
#include <GLUT/glut.h>
#elif __linux
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>
#include <iostream>

#include "math/yamathutil.h"

#include "profiling.h"

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
#include "units/Cephalopod.h"

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

extern int aiplayer;

// Mouse offset for camera zoom in and out.
int _xoffset = 0;
int _yoffset = 0;

// Right, left or middle button pressed.
int buttonState;


enum COMMAND_MODES {ATTACK_MODE, DESTINATION_MODE};

int commandmode=DESTINATION_MODE;


static int controlmap[] = {0,1,2,3,4,5,6,7,8};


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
                ((entities[controller.controllingid]->getType() == VehicleTypes::MANTA) || (entities[controller.controllingid]->getType() == CONTROLABLEACTION) ||
                 (entities[controller.controllingid]->getType() == VehicleTypes::WALRUS) ) )
        {
            CLog::Write(CLog::Debug,"Active control\n");
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
            CLog::Write(CLog::Debug,"Mouse down %d,%d\n",x,y);
            if (controller.view == 2)
            {
                if (specialKey == GLUT_ACTIVE_SHIFT && commandmode == ATTACK_MODE)
                {
                    Vec3f target = setLocationOnMap(x,y);


                    Vehicle *s = findNearestEnemyVehicle(entities[controller.controllingid]->getFaction(),target,2000);

                    if (s)
                    {
                        entities[controller.controllingid]->attack(s->getPos());
                        entities[controller.controllingid]->enableAuto();
                    }

                    CLog::Write(CLog::Debug,"Attack set to (%10.2f,%10.2f,%10.2f)\n", target[0],target[1],target[2]);
                } else
                if (specialKey == GLUT_ACTIVE_SHIFT && commandmode == DESTINATION_MODE)
                {
                    Vec3f target = setLocationOnMap(x,y);
                    //@FIXME
                    entities[controller.controllingid]->goTo(target);

                    CLog::Write(CLog::Debug,"Destination set to (%10.2f,%10.2f,%10.2f)\n", target[0],target[1],target[2]);


                } else {
                    centermap(x,y);
                    Vec3f lo = setLocationOnMap(x,y);
                    CLog::Write(CLog::Debug,"Set location on map (%10.5f, %10.5f)\n", lo[0], lo[2]);
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


/**
 * Now this function uses directly not the position of the entity in the list, rather the index.
 * The index does not change as long as the unit is still active.
 *
 * @brief switchControl
 * @param controlposition
 */
void switchControl(size_t id)
{
    //if (controlposition > entities.size())
    //{
    //    controller.controllingid = CONTROLLING_NONE;
    //    return;
    //}
    //size_t id = entities.indexAt(controlposition);

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
    if (!(entities[id]->getFaction() == controller.faction || controller.faction == BOTH_FACTION))
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

                CLog::Write(CLog::Debug,"Controlling %s\n", content);

                switchControl(atoi(content));

            } else
            if (controller.str.find("set") != std::string::npos)
            {
                std::string command = controller.str.substr(3+1,1);
                std::string entityid = controller.str.substr(controller.str.find("#")+1);

                dout << "Command-" << command << " map to " << entityid << std::endl;

                int keycontrol = atoi(command.c_str());

                if (keycontrol>=1 && keycontrol<=9)
                    controlmap[keycontrol-1] = atoi(entityid.c_str());

            }
            else if (controller.str.find("walrus") != std::string::npos)
            {
                // @FIXME What happen the controller.faction is both !!! Need to decide that.
                const char *content = controller.str.substr(6).c_str();

                printf ("Walrus %s\n", content);

                size_t index = CONTROLLING_NONE;
                findWalrusByFactionAndNumber(index, controller.faction, atoi(content));
                switchControl(index);

            } else
            if (controller.str.find("manta") != std::string::npos)
            {
                const char *content = controller.str.substr(5).c_str();

                size_t index = CONTROLLING_NONE;
                findMantaByFactionAndNumber(index, controller.faction, atoi(content));

                printf ("Manta %d\n", index);

                switchControl(index);

            } else
            if (controller.str.find("def") != std::string::npos)
            {
                std::string islandcode = controller.str.substr(3+controller.str.substr(3).find(" "));
                std::string islandname = islandcode.substr(1,islandcode.find("#")-1);
                const char *structurenumber = controller.str.substr(controller.str.find("#")+1).c_str();

                dout << "Island-" << islandname << "-structure " << structurenumber << std::endl;

                size_t index = CONTROLLING_NONE;

                for (int j=0;j<islands.size();j++)
                {
                    if (islandname == islands[j]->getName() && islands[j]->getStructures().size()>atoi(structurenumber))
                    {
                        dout << "Found-" << islandname << "-structure " << structurenumber << std::endl;
                        index = islands[j]->getStructures()[atoi(structurenumber)];
                    }
                }
                switchControl(index);

            } else
            if (controller.str.find("attack") != std::string::npos)
            {
                commandmode = ATTACK_MODE;

            } else
            if (controller.str.find("destination") != std::string::npos)
            {
                commandmode = DESTINATION_MODE;
            } else
            if (controller.str.find("info") != std::string::npos)
            {
                std::string islandcode = controller.str.substr(4+controller.str.substr(4).find(" "));
                std::string islandname = islandcode.substr(1);

                dout << "Island-" << islandname << std::endl;

                for (int j=0;j<islands.size();j++)
                {
                    if (islandname == islands[j]->getName())
                    {
                        std::vector<size_t> str = islands[j]->getStructures();

                        CommandCenter *cc = (CommandCenter*)islands[j]->getCommandCenter();

                        if (cc)
                        {
                            if (cc->getIslandType() == ISLANDTYPES::DEFENSE_ISLAND)
                                dout << "Defense Island" << std::endl;
                            else if (cc->getIslandType() == ISLANDTYPES::FACTORY_ISLAND)
                                dout << "Factory Island" << std::endl;
                            else if (cc->getIslandType() == ISLANDTYPES::LOGISTICS_ISLAND)
                                dout << "Logistic Island" << std::endl;
                        }

                        for(size_t i=0;i<str.size();i++)
                        {
                            if (entities[str[i]]->getFaction()==controller.faction)
                            {
                                dout << entities[str[i]]->subTypeText(entities[str[i]]->getSubType()) << "-"
                                          << entities[str[i]]->getHealth() << std::endl;
                            }
                        }
                    }
                }
            } else
            if (controller.str.find("taxi") != std::string::npos)
            {
                taxiManta(entities[controller.controllingid]);
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

                if (entities.isValid(controller.controllingid) && entities[controller.controllingid]->getSubType()==CEPHALOPOD)
                {
                    Cephalopod *w = (Cephalopod*) entities[controller.controllingid];
                    // Check if this invading unit is already on the island
                    BoxIsland *island = w->getIsland();

                    if (island)
                    {
                        captureIsland(w,island,w->getFaction(),typeofisland,space, world);
                    }
                }
            } else
            if (controller.str.find("goback") != std::string::npos)
            {
                if (entities.isValid(controller.controllingid) && entities[controller.controllingid]->getType()==WALRUS)
                {
                    Walrus *w = (Walrus*) entities[controller.controllingid];
                    Vehicle *b = findCarrier(w->getFaction());
                    Vec3f t = b->getPos() - w->getPos();
                    t = t.normalize();

                    Vec3f up = Vec3f(0,1,0);

                    t = t.cross(up);
                    t = t.normalize();

                    w->goTo(b->getPos()+(b->getForward().normalize()*150));
                    w->enableAuto();
                }
            } else
            if (controller.str.find("godmode") != std::string::npos)
            {
                controller.faction = BOTH_FACTION;
            } else
            if (controller.str.find("greenmode") != std::string::npos)
            {
                controller.faction = GREEN_FACTION;
            } else
            if (controller.str.find("bluemode") != std::string::npos)
            {
                controller.faction = BLUE_FACTION;
            } else
            if (controller.str.find("aiplayergreen") != std::string::npos)
            {
                aiplayer = GREEN_AI;
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
        case 'a':controller.registers.roll-=1.0f;break;
        case 'A':controller.registers.roll-=50.0f;break;
        case 'd':controller.registers.roll+=1.0f;break;
        case 'D':controller.registers.roll+=50.0f;break;
        case '`':controller.registers.roll=0;break;
        case 'w':controller.registers.pitch-=1.0f;break;
        case 's':controller.registers.pitch+=1.0f;break;
        case 'z':controller.registers.yaw-=1.0f;break;
        case 'c':controller.registers.yaw+=1.0f;break;
        case 'v':controller.registers.precesion-=1.0f;break;
        case 'b':controller.registers.precesion+=1.0f;break;
        case 'i':controller.registers.bank-=1.0f;break;
        case 'k':controller.registers.bank+=1.0f;break;
        case 'p':controller.pause = !controller.pause;break;
        case 'r':controller.registers.thrust+=0.05;break;
        case 'R':controller.registers.thrust+=10.00;break;
        case 'f':controller.registers.thrust-=0.05;break;
        case 'F':controller.registers.thrust-=10.00;break;
        case 'q':controller.reset();break;
        case 'Q':controller.stabilize();break;
        case 'j':entities[controller.controllingid]->enableAuto();break;
        case 'J':entities[controller.controllingid]->disableAuto();break;

        case '0':
            controller.controllingid = CONTROLLING_NONE;
            controller.reset();
            Camera.reset();
        break;
        case '1':case '2':case '3': case '4': case '5':case '6':case '7':case '8':case '9':
        {
            size_t index = CONTROLLING_NONE;
            index = controlmap[(int)(key-48)-1];

            switchControl(index);
        }
        break;
        case 'I':
            {
            int param = 0;
            dout << "Param:" << std::endl; std::cin >> param;
            dout << "Value:" << std::endl; std::cin >> controller.param[param] ;
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
        case 'l':
            synchronized(entities.m_mutex)
            {
                if (entities[controller.controllingid]->getType()==CARRIER || entities[controller.controllingid]->getType()==LANDINGABLE )
                {
                    Cephalopod* m = (Cephalopod*)(entities[controller.controllingid]->spawn(world,space,CEPHALOPOD,findNextNumber(entities[controller.controllingid]->getFaction(),MANTA,CEPHALOPOD)));

                    size_t idx = entities.push_back(m, m->getGeom());
                }
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
                                switchControl(i);

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



