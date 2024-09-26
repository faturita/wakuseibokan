#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <unordered_map>

#include "soundtexture.h"

#include "sounds.h"

#include "../camera.h"


extern  Camera camera;
extern bool mute;

//std::unordered_map<std::string, SoundTexture*> soundtextures;
SoundTexture s;

struct soundorder {
    Vec3f source;
    char soundname[256];
} soundelement ;

bool newsound = false;

void playthissound_(Vec3f source, char fl[256]);

void playthissound(Vec3f source, char fl[256])
{
    soundelement.source = source;
    strncpy(soundelement.soundname, fl, 256);

    newsound = true;
}

void * sound_handler(void *arg)
{
    int sd;

    sd = *((int*)arg);

    while (!mute && !s.interrupt)
    {
        if (newsound)
        {
            printf("Playing %s \n", soundelement.soundname);
            playthissound_(soundelement.source, soundelement.soundname);
            newsound = false;
        }

        usleep(100);
    }
}


void initSound()
{
    //SoundTexture* so = new SoundTexture();
    //so->init("sounds/takeoff.wav");
    //soundtextures["takeoff"] = so;

    //so = new SoundTexture();
    //so->init("sounds/gunshot.wav");
    //soundtextures["gunshot"] = so;

    pthread_t th;

    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    int sd=29;

    // @NOTE: If you create the thread too quickly, the sd value is replaced by accept when two connections arrive simultaneously.
    //        and that means that one socket identifier is lost and the client will hang.
    pthread_create(&th, &attr, &sound_handler, (void*)&sd);
    usleep(3000);  

}

void clearSound()
{
    if (!mute)
    {
        s.interrupt = true;
        s.close();
    }
}

void playsound(char *filename)
{
    // @FIXME: This is extremly risky !!!
    char temp[200];
    sprintf(temp, "%s %s &",PLAYSOUNDCOMMAND, filename);
    system(temp);
}


void playthissound(char fl[256])
{
    try {
        if (!mute) {
            while (!s.done)
            {
                s.interrupt = true;
                Stk::sleep( 0 );
            }
            //static SoundTexture s;
            s.init(fl);
            s.amplitude = 1.0;
            s.play();
        }
    }  catch (StkError) {

    }

}

void playthissound_(Vec3f source, char fl[256])
{
    // @NOTE: Use the camera location to determine if the sound should be reproduced or not
    //   and with which intensity.
    try {
        if (!mute) {
            Vec3f dist = source - camera.pos;
            if (dist.magnitude()<SOUND_DISTANCE_LIMIT)
            {
                StkFloat amplitude = SOUND_DISTANCE_LIMIT-dist.magnitude() / SOUND_DISTANCE_LIMIT;
                amplitude = 1.0;
                while (!s.done)
                {
                    s.interrupt = true;
                    Stk::sleep( 0 );
                }
                //static SoundTexture s;
                s.init(fl);
                s.amplitude = amplitude;
                s.play();
            }
        }
    }  catch (StkError) {

    }

}




void firesound(int times)
{
    for(int i=0;i<times;i++)
        printf ("%c", 7);

extern bool mute;

//std::unordered_map<std::string, SoundTexture*> soundtextures;
SoundTexture s;
}

void bullethit(Vec3f source)
{
    playthissound(source,"sounds/bullethit.wav");
}


void smallenginestart(Vec3f source)
{
    playthissound(source,"sounds/boozing.wav");
}

void enginestart(Vec3f source)
{
    playthissound(source,"sounds/cruise.wav");
}

void takeoff(Vec3f source)
{
    playthissound(source,"sounds/takeoff.wav");
}

void explosion(Vec3f source)
{
    playthissound(source,"sounds/explosion.wav");
}

void radarbeep(Vec3f source)
{
    playthissound(source,"sounds/radarbeep.wav");
}

void splash(Vec3f source)
{
    playthissound(source,"sounds/splash.wav");
}

void coast(Vec3f source)
{
    playthissound(source,"sounds/coast.wav");
}

void honk(Vec3f source)
{
    playthissound(source,"sounds/boathonk.wav");
}

void soaring(Vec3f source)
{
    playthissound(source,"sounds/soaring.wav");
}

void gunshot(Vec3f source)
{
    playthissound(source,"sounds/gunshot.wav");
}

void artilleryshot(Vec3f source)
{
    playthissound(source,"sounds/artillery.wav");
}

void droneflying(Vec3f source)
{
    playthissound(source,"sounds/cephalopod.wav");
}

void intro()
{
    playthissound( "sounds/intro.wav");
}
