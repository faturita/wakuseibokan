#include <stk/FileWvIn.h>
#include <stk/RtAudio.h>

#include <signal.h>
#include <iostream>
#include <cstdlib>

#include "SoundSource.h"


int inchannels = 2; // Default to stereo
int outchannels = 2; // Default to stereo
RtAudio dac;


// The audio callback is now extremely simple!
int tick_c( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
    stk::StkFloat *samples = (stk::StkFloat *) outputBuffer;

    SoundSource *so = (SoundSource*) userData;

    // Fill the audio buffer with samples from our engine's tick() method
    for (unsigned int i = 0; i < nBufferFrames; i++) {

        // @FIXME: I am assuming input has one channel and output has two channels.
        stk::StkFloat sample = so->tick();

        for(int channel = 0; channel < outchannels; ++channel) {
            *samples++ = sample * so->amplitude;
        }

    }
    if ( so->isDone() || so->interrupt) {
        so->done = true;
        //if (so->dac.isStreamRunning() || so->dac.isStreamOpen())
            //try {so->dac.closeStream();} catch (StkError &) {}
        return 1;
    }
    else
        return 0;   // Return 0 for success
}



void play(SoundSource* p) 
{
    if (dac.isStreamRunning() || dac.isStreamOpen())
        dac.closeStream();

    // Figure out how many bytes in an StkFloat and setup the RtAudio stream.
    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = ( inchannels == 1 ) ? 2 : inchannels; //  Play mono files as stereo.
    RtAudioFormat format = ( sizeof(stk::StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
    unsigned int bufferFrames = stk::RT_BUFFER_SIZE;

    try {
      dac.openStream( &parameters, NULL, format, (unsigned int)stk::Stk::sampleRate(), &bufferFrames, &tick_c, (void *)p );
    }
    catch ( RtAudioError &error ) {
      error.printMessage();
      printf("Error opening the stream.  Now set to done.");
      p->done = true;
      return;
    }

    try {
      dac.startStream();
    }
    catch ( RtAudioError &error ) {
      error.printMessage();
      printf("Error starting the stream.");
    }

    p->done = false;
}

void close(SoundSource *p)
{
    // Block waiting until callback signals done.
    while ( !p->done )
      stk::Stk::sleep( 100 );

    // By returning a non-zero value in the callback above, the stream
    // is automatically stopped.  But we should still close it.
    try {
      dac.closeStream();
    }
    catch ( RtAudioError &error ) {
      error.printMessage();
      printf("Error closing the stream.");
    }
}