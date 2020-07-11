#include "sounds.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <thread>         // std::thread

#include "FileWvIn.h"
#include "RtAudio.h"

#include <signal.h>
#include <iostream>
#include <cstdlib>

using namespace stk;

// Eewww ... global variables! :-)
bool done = false;

bool keeping = true;
bool rendersound = false;

StkFrames frames;

int Sound;


// This tick() function handles sample computation only.  It will be
// called automatically when the system needs a new buffer of audio
// samples.
int tick( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
          double streamTime, RtAudioStreamStatus status, void *userData )
{
    FileWvIn *input = (FileWvIn *) userData;
    StkFloat *samples = (StkFloat *) outputBuffer;

    input->tick( frames );

    for ( unsigned int i=0; i<frames.size(); i++ ) {
        *samples++ = frames[i];
        if ( input->channelsOut() == 1 ) *samples++ = frames[i]; // play mono files in stereo
    }

    if ( input->isFinished() ) {
        done = true;
        return 1;
    }
    else
        return 0;
}


int play()
{

    while (keeping)
    {
        while (!rendersound)
            Stk::sleep( 100 );
        char filename[256];

        switch (Sound) {
        case 1:strcpy(filename,"sounds/boozing.wav");break;
        case 2:strcpy(filename,"sounds/cruise.wav");break;
        }


        // Set the global sample rate before creating class instances.
        Stk::setSampleRate( (StkFloat) 44100.0);

        std::cout << "Sound:" << Sound << std::endl;

        // Initialize our WvIn and RtAudio pointers.
        RtAudio dac;
        FileWvIn input;

        // Try to load the soundfile.
        try {
            input.openFile( filename);
        }
        catch ( StkError & ) {
            std::cout << "ERROR!" << std::endl;
            exit( 1 );
        }

        // Set input read rate based on the default STK sample rate.
        double rate = 1.0;
        rate = input.getFileRate() / Stk::sampleRate();
        rate *= 1.0;
        input.setRate( rate );

        input.ignoreSampleRateChange();

        // Find out how many channels we have.
        int channels = input.channelsOut();

        // Figure out how many bytes in an StkFloat and setup the RtAudio stream.
        RtAudio::StreamParameters parameters;
        parameters.deviceId = dac.getDefaultOutputDevice();
        parameters.nChannels = ( channels == 1 ) ? 2 : channels; //  Play mono files as stereo.
        RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
        unsigned int bufferFrames = RT_BUFFER_SIZE;
        try {
            dac.openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &tick, (void *)&input );
        }
        catch ( RtAudioError &error ) {
            error.printMessage();
            std::cout << "ERROR!" << std::endl;
        }

        // Install an interrupt handler function.
        //(void) signal(SIGINT, finish);

        // Resize the StkFrames object appropriately.
        frames.resize( bufferFrames, channels );

        try {
            dac.startStream();
        }
        catch ( RtAudioError &error ) {
            error.printMessage();
            std::cout << "ERROR!" << std::endl;
        }

        // Block waiting until callback signals done.
        while ( !done )
            Stk::sleep( 100 );

        // By returning a non-zero value in the callback above, the stream
        // is automatically stopped.  But we should still close it.
        try {
            dac.closeStream();
        }
        catch ( RtAudioError &error ) {
            error.printMessage();
            std::cout << "ERROR!" << std::endl;
        }

        done = false;
        rendersound = false;
    }
    return 1;

}

void playfile(int sound)
{
    Sound = sound;
    rendersound = true;
    static std::thread first (play); // spawn new thread that calls bar(0)

}

void bullethit()
{
    system("afplay sounds/bullethit.wav &");
}

void firesound(int times)
{
    for(int i=0;i<times;i++)
        printf ("%c", 7);

}

void smallenginestart()
{
    //system("afplay sounds/boozing.m4a &");
    //play();
    playfile(1);
}

void enginestart()
{
    //system("afplay sounds/cruise.m4a &");
    playfile(2);
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
    //system("afplay sounds/BoatHonk.m4a &");
    playfile(2);
}

void soaring()
{
    system("afplay sounds/soaring.m4a &");
}

void gunshot()
{
    system("afplay sounds/Gunshot.m4a &");
}

void artilleryshot()
{
    system("afplay sounds/Artillery.wav &");
}

void intro()
{
    system("afplay sounds/intro.mp3 &");
}
