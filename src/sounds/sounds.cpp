#include "sounds.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <unordered_map>

#include "soundtexture.h"

extern bool mute;

//std::unordered_map<std::string, SoundTexture*> soundtextures;
SoundTexture s;

void initSound()
{
    //SoundTexture* so = new SoundTexture();
    //so->init("sounds/takeoff.wav");
    //soundtextures["takeoff"] = so;

    //so = new SoundTexture();
    //so->init("sounds/gunshot.wav");
    //soundtextures["gunshot"] = so;

}

void clearSound()
{

}

void playsound(char *filename)
{
    // @FIXME: This is extremly risky !!!
    char temp[200];
    sprintf(temp, "%s %s &",PLAYSOUNDCOMMAND, filename);
    system(temp);
}

void bullethit()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/bullethit.wav");
        s.play();
    }
}

void firesound(int times)
{
    for(int i=0;i<times;i++)
        printf ("%c", 7);

}

void smallenginestart()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/boozing.wav");
        s.play();
    }
}

void enginestart()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/cruise.wav");
        s.play();
    }
}

void takeoff()
{
    if (!mute)
    {
        //static SoundTexture s;
        s.init("sounds/takeoff.wav");
        s.play();
    }
}

void explosion()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/explosion.wav");
        s.play();
    }
}

void radarbeep()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/radarbeep.wav");
        s.play();
    }
}

void splash()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/splash.wav");
        s.play();
    }
}

void coast()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/coast.wav");
        s.play();
    }
}

void honk()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/boathonk.wav");
        s.play();
    }
}

void soaring()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/soaring.wav");
        s.play();
    }
}

void gunshot()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/gunshot.wav");
        s.play();
    }
}

void artilleryshot()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/artillery.wav");
        s.play();
    }
}

void droneflying()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/cephalopod.wav");
        s.play();
    }
}

void intro()
{
    if (!mute) {
        //static SoundTexture s;
        s.init("sounds/intro.wav");
        s.play();
    }
}
