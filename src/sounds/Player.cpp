#include <stk/FileWvIn.h>
#include <stk/RtAudio.h>

#include <signal.h>
#include <iostream>
#include <cstdlib>

#include "Player.h"

// The audio callback is now extremely simple!
int tick_c( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
    stk::StkFloat *samples = (stk::StkFloat *) outputBuffer;

    Player *so = (Player*) userData;

    // Fill the audio buffer with samples from our engine's tick() method
    for (unsigned int i = 0; i < nBufferFrames; i++) {
        stk::StkFloat sample = so->tick();

        for(int channel = 0; channel < so->channels; ++channel) {
            *samples++ = sample * so->amplitude;
        }

    }
    return 0; // Return 0 for success
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
}

stk::StkFloat Player::tick()  {
    return file.tick();
}

bool Player::isDone() const {
    return file.isFinished();
}


void Player::play() 
{
    // Find out how many channels we have.
    channels = file.channelsOut();

    if (dac.isStreamRunning() || dac.isStreamOpen())
        dac.closeStream();

    // Figure out how many bytes in an StkFloat and setup the RtAudio stream.
    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = ( channels == 1 ) ? 2 : channels; //  Play mono files as stereo.
    RtAudioFormat format = ( sizeof(stk::StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
    unsigned int bufferFrames = stk::RT_BUFFER_SIZE;

    try {
      dac.openStream( &parameters, NULL, format, (unsigned int)stk::Stk::sampleRate(), &bufferFrames, &tick_c, (void *)this );
    }
    catch ( RtAudioError &error ) {
      error.printMessage();
      printf("Error opening the stream.  Now set to done.");
      done = true;
      return;
    }

    try {
      dac.startStream();
    }
    catch ( RtAudioError &error ) {
      error.printMessage();
      printf("Error starting the stream.");
    }

    std::cout << "Playing sound:" << std::endl;
    done = false;
}

void Player::close()
{
    // Block waiting until callback signals done.
    //while ( !done )
    //  stk::Stk::sleep( 100 );

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