#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <unordered_map>

#include <assert.h>

#include "stk/Stk.h"
#include "stk/FileWvIn.h"
#include "stk/RtAudio.h"

#include "SoundGenerator.h"
#include "SoundRender.h"
#include "sounds.h"

#include "../camera.h"
#include "../profiling.h"


extern  Camera camera;
extern bool mute;

SoundGenerator s;

struct SoundOrder {
    Vec3f source;
    char soundname[256];
} soundelement ;

bool newsound = false;

void playthissound_(Vec3f source, char fl[256]);
void initsoundsystem_();

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

    initsoundsystem_();

    while (!mute && !s.isInterrupted())
    {
        if (newsound)
        {
            playthissound_(soundelement.source, soundelement.soundname);
            newsound = false;
        }

        usleep(100);
    }
}

// @NOTE: Stk is very tricky, particularly in linux.  It is known (I didn't) that you need to serialize all the access with just one 
// DAC. Then if you want to play a sound, you need to stop the current one, and then start the new one OR create a mixture of the two sounds.
// So, current approach does both things.  It creates a separated thread to serialize the access to the DAC and mixes sound within the same DAC.
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
        s.interrupt();
        close(&s);
    }
}


void initsoundsystem_()
{
    try {
        if (!mute) {
                while (!s.isFinished())
                {
                    s.interrupt();
                    stk::Stk::sleep( 0 );
                }
                play(&s);
            }
        }  catch (stk::StkError) {
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

            if (dist.magnitude()<SOUND_DISTANCE_LIMIT)
            {
                stk::StkFloat amplitude = SOUND_DISTANCE_LIMIT-dist.magnitude() / SOUND_DISTANCE_LIMIT;
                amplitude = 1.0;   // @FIXME: This is a hack to make the sound always play at full volume.
                s.setPlayerSource(fl);
            }
        }
    }  catch (stk::StkError) {
        dout << "Sound error reported, but ignored." << std::endl;
    }

}


void setflyingengine(Vec3f source, float speed)
{
    if (!mute)
    {
        Vec3f dist = source - camera.pos;

        if (dist.magnitude()<SOUND_DISTANCE_LIMIT)
        {
            stk::StkFloat amplitude = SOUND_DISTANCE_LIMIT-dist.magnitude() / SOUND_DISTANCE_LIMIT;
            s.setFlyingVehicleSpeed(speed);
        }
    }
}

void setsailingengine(Vec3f source, float speed)
{
    if (!mute)
    {
        Vec3f dist = source - camera.pos;

        if (dist.magnitude()<SOUND_DISTANCE_LIMIT)
        {
            stk::StkFloat amplitude = SOUND_DISTANCE_LIMIT-dist.magnitude() / SOUND_DISTANCE_LIMIT;
            s.setSailingVehicleSpeed(speed);
        }
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
    playthissound(Vec3f(0,0,0), "sounds/intro.wav");
}
