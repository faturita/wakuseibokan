#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <unordered_map>

#include <assert.h>

#include "soundtexture.h"
#include "Player.h"
#include "SoundRender.h"
#include "sounds.h"

#include "../camera.h"
#include "../profiling.h"


extern  Camera camera;
extern bool mute;

//std::unordered_map<std::string, SoundTexture*> soundtextures;
//SoundTexture s;


Player s;

struct SoundOrder {
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
            playthissound_(soundelement.source, soundelement.soundname);
            newsound = false;
        }

        usleep(100);
    }
}

// @NOTE: Stk is very tricky, particularly in linux.  So I have found that I cannot work on two buffers at the same time and I needed to serialize the access.
//   But this also makes the system so much slower, so it is just better to use a separate thread to handle the sound.
//   This way works much better and there is really no penalty in performance.
void initSound()
{
    pthread_t th;

    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    int sd=29;

    pthread_create(&th, &attr, &sound_handler, (void*)&sd);
    usleep(3000);  

}

void clearSound()
{
    if (!mute)
    {
        s.interrupt = true;
        close(&s);
    }
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
            play(&s);
        }
    }  catch (StkError) {
        dout << "Sound error reported, but ignored." << std::endl;
    }

}

void playthissound_(Vec3f source, char fl[256])
{
    // @NOTE: Use the camera location to determine if the sound should be reproduced or not
    //   and with which intensity.
    try {
        if (!mute) {
            Vec3f dist = source - camera.pos;

            if (dist.magnitude()<SOUND_DISTANCE_LIMIT || true)
            {
                StkFloat amplitude = SOUND_DISTANCE_LIMIT-dist.magnitude() / SOUND_DISTANCE_LIMIT;
                amplitude = 1.0;
                while (!s.done)
                {
                    s.interrupt = true;
                    Stk::sleep( 0 );
                }
                s.init(fl);
                s.amplitude = amplitude;
                play(&s);
            }
        }
    }  catch (StkError) {
        dout << "Sound error reported, but ignored." << std::endl;
    }

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
