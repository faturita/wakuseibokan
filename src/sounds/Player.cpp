#include <stk/FileWvIn.h>
#include <stk/RtAudio.h>

#include <signal.h>
#include <iostream>
#include <cstdlib>

#include "Player.h"


int inchannels = 2; // Default to stereo
RtAudio dac;

// The audio callback is now extremely simple!
int tick_c( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
    stk::StkFloat *samples = (stk::StkFloat *) outputBuffer;

    Player *so = (Player*) userData;

    // Fill the audio buffer with samples from our engine's tick() method
    for (unsigned int i = 0; i < nBufferFrames; i++) {
        stk::StkFloat sample = so->tick();

        for(int channel = 0; channel < inchannels; ++channel) {
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

void Player::init(char fl[256]) {
    try {
        file.openFile(fl);
    } catch (stk::StkError &e) {
        e.printMessage();
    }

    double rate = 1.0;
    rate = file.getFileRate() / stk::Stk::sampleRate();
    rate *= 1.0;
    file.setRate(rate);
    file.ignoreSampleRateChange();

    // Find out how many channels we have.
    channels = file.channelsOut();

    interrupt = false;
    done = false;
}

stk::StkFloat Player::tick()  {
    return file.tick();
}

bool Player::isDone() const {
    return file.isFinished();
}


void play(Player* p) 
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

void close(Player *p)
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