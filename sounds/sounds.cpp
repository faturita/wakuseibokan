#include "sounds.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void firesound(int times)
{
    for(int i=0;i<times;i++)
        printf ("%c", 7);

}

void smallenginestart()
{
    system("afplay sounds/fishstart.m4a &");
}

void enginestart()
{
    system("afplay sounds/cruise.m4a &");
}

void takeoff()
{
    system("afplay sounds/takeoff.mp3 &");
}

void explosion()
{
    system("afplay sounds/explosion.mp3 &");
}

void coast()
{
    system("afplay sounds/Coast.m4a &");
}

void honk()
{
    system("afplay sounds/BoatHonk.m4a &");
}

void soaring()
{
    system("afplay sounds/soaring.m4a &");
}

void gunshot()
{
    system("afplay sounds/Gunshot.m4a &");
}

void intro()
{
    system("afplay sounds/intro.mp3 &");
}
