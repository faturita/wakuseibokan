#ifndef SOUNDS_H
#define SOUNDS_H


#ifdef __linux
#define PLAYSOUNDCOMMAND "mpg321"
#elif __APPLE__
#define PLAYSOUNDCOMMAND "afplay"
#include <GLUT/glut.h>
#endif

#include "../math/yamathutil.h"

#define SOUND_DISTANCE_LIMIT 1000.0


void initSound();
void clearSound();

void firesound(int times);
void enginestart(Vec3f source);
void takeoff(Vec3f source);
void explosion(Vec3f source);
void splash(Vec3f source);
void honk(Vec3f source);
void gunshot(Vec3f source);
void artilleryshot(Vec3f source);
void smallenginestart(Vec3f source);
void soaring(Vec3f source);
void bullethit(Vec3f source);
void droneflying(Vec3f source);

void radarbeep(Vec3f source);

void intro();


#endif // SOUNDS_H
