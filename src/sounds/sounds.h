#ifndef SOUNDS_H
#define SOUNDS_H


#ifdef __linux
#define PLAYSOUNDCOMMAND "mpg321"
#elif __APPLE__
#define PLAYSOUNDCOMMAND "afplay"
#include <GLUT/glut.h>
#endif



void firesound(int times);
void enginestart();
void takeoff();
void explosion();
void honk();
void gunshot();
void artilleryshot();
void smallenginestart();
void soaring();
void bullethit();
void droneflying();

void intro();


#endif // SOUNDS_H
