#ifndef USERCONTROLS_H
#define USERCONTROLS_H

#include <assert.h>
#include <string>
#include <iostream>


class Controller
{
public:
    
	// Device ID to be controller.
	int controlling;
    
	// Index to Observable interfaces.
	int camera;

    // Which view mode is currently active.
    int view=1;
    
	// R+,F-
	float thrust;
    
	// ModAngleX
	float roll;
    
	// ModAngleY
	float pitch;
    
	// ModAngleZ
	float yaw;
    
	// ModAngleP
	float precesion;

    // Custom parameters that can be entered from controller.
    float param[10];
    
	bool pause=false;
    
	bool pp;
    
    bool finish=false;

    bool teletype=false;

    std::string str;
    
    void reset()
    {
        roll=pitch=precesion=yaw=0;
        thrust=0;
    };
    
    void interrupt()
    {
        finish=true;
    }
    
    bool isInterrupted()
    {
        return finish;
    };

    bool isTeletype()
    {
        return teletype;
    };
};


GLint gltWriteTGA(const char *szFileName);

void processMouseEntry(int state) ;

void processMouse(int button, int state, int x, int y) ;

void processMouseActiveMotion(int x, int y) ;

void processMousePassiveMotion(int x, int y) ;

void handleKeypress(unsigned char key, int x, int y) ;

void handleSpecKeypress(int key, int x, int y);

GLbyte *gltReadTGABits(const char *szFileName, GLint *iWidth, GLint *iHeight,
		GLint *iComponents, GLenum *eFormat);

GLint gltWriteTGA(const char *szFileName);


#endif /* USERCONTROLS_H */

