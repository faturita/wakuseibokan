#ifndef USERCONTROLS_H
#define USERCONTROLS_H

#include <assert.h>


class Controller
{
public:
    
	// Device ID to be controller.
	int controlling;
    
	// Index to Observable interfaces.
	int camera;
    
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
    
	bool pause=false;
    
	bool pp;
    
    bool finish=false;
    
    void reset()
    {
        roll=pitch=precesion=yaw=0;
        //thrust=0;
    };
    
    void interrupt()
    {
        finish=true;
    }
    
    bool isInterrupted()
    {
        return finish;
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

