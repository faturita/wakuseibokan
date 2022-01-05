#include "sounds.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

extern bool mute;

void playsound(char *filename)
{
    // @FIXME: This is extremly risky !!!
    char temp[200];
    sprintf(temp, "%s %s &",PLAYSOUNDCOMMAND, filename);
    system(temp);
}

void bullethit()
{
    if (!mute) playsound("sounds/bullethit.wav");
}

void firesound(int times)
{
    for(int i=0;i<times;i++)
        printf ("%c", 7);

}

void smallenginestart()
{
    if (!mute) playsound("sounds/boozing.m4a");
}

void enginestart()
{
    if (!mute) playsound("sounds/cruise.m4a");
}

void takeoff()
{
    if (!mute) playsound("sounds/takeoff.mp3");
}

void explosion()
{
    if (!mute) playsound("sounds/explosion.mp3");
}

void radarbeep()
{
    if (!mute) playsound("sounds/radarbeep.wav");
}

void splash()
{
    if (!mute) playsound("sounds/splash.wav");
}

void coast()
{
    if (!mute) playsound("sounds/Coast.m4a");
}

void honk()
{
    if (!mute) playsound("sounds/BoatHonk.m4a");
}

void soaring()
{
    if (!mute) playsound("sounds/soaring.m4a");
}

void gunshot()
{
    if (!mute) playsound("sounds/Gunshot.m4a");
}

void artilleryshot()
{
    if (!mute) playsound("sounds/Artillery.wav");
}

void droneflying()
{
    if (!mute) playsound("sounds/Cephalopod.m4a");
}

void intro()
{
    if (!mute) playsound("sounds/intro.mp3");
}
