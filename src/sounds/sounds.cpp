#include "sounds.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

extern bool mute;

void bullethit()
{
    if (!mute) system("afplay sounds/bullethit.wav &");
}

void firesound(int times)
{
    for(int i=0;i<times;i++)
        printf ("%c", 7);

}

void smallenginestart()
{
    if (!mute) system("afplay sounds/boozing.m4a &");
}

void enginestart()
{
    if (!mute) system("afplay sounds/cruise.m4a &");
}

void takeoff()
{
    if (!mute) system("afplay sounds/takeoff.mp3 &");
}

void explosion()
{
    if (!mute) system("afplay sounds/explosion.mp3 &");
}

void coast()
{
    if (!mute) system("afplay sounds/Coast.m4a &");
}

void honk()
{
    if (!mute) system("afplay sounds/BoatHonk.m4a &");
}

void soaring()
{
    if (!mute) system("afplay sounds/soaring.m4a &");
}

void gunshot()
{
    if (!mute) system("afplay sounds/Gunshot.m4a &");
}

void artilleryshot()
{
    if (!mute) system("afplay sounds/Artillery.wav &");
}

void droneflying()
{
    if (!mute) system("afplay sounds/Cephalopod.m4a &");
}

void intro()
{
    if (!mute) system("afplay sounds/intro.mp3 &");
}
