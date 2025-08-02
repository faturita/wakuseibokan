#include <cmath> // For sin, cos, and M_PI
#include "stk/Stk.h"
#include "stk/SineWave.h"
#include "stk/Noise.h"
#include "stk/BiQuad.h"
#include "stk/RtAudio.h" 

#include "MantaSoundTexture.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Mapping function for the turbine whine frequency
float getFrequencyForSpeed(float speed) {
    const float speed_start = 90.0f;
    const float speed_end = 500.0f;
    const float freq_start = 200.0f;
    const float freq_end = 1200.0f;

    if (speed < speed_start) speed = speed_start;
    if (speed > speed_end) speed = speed_end;

    float ratio = (speed - speed_start) / (speed_end - speed_start);
    return freq_start + ratio * (freq_end - freq_start);
}

// Separate mapping for the filter to create a more realistic "roar"
float mapSpeedToFilterCutoff(float speed) {
    const float speed_start = 90.0f;
    const float speed_end = 500.0f;
    const float cutoff_start = 400.0f;  // Dull roar at low speed
    const float cutoff_end = 6000.0f; // Bright, hissy roar at high speed

    if (speed < speed_start) speed = speed_start;
    if (speed > speed_end) speed = speed_end;

    float ratio = (speed - speed_start) / (speed_end - speed_start);
    return cutoff_start + ratio * (cutoff_end - cutoff_start);
}

// The audio callback function that STK will use to get audio samples.
int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                  double streamTime, RtAudioStreamStatus status, void *userData)
{
    // Cast the generic user data pointer back to our specific class type
    MantaSoundTexture *engine = (MantaSoundTexture *)userData;
    stk::StkFloat *samples = (stk::StkFloat *)outputBuffer;

    // Fill the audio buffer with samples from our engine's tick() method
    for (unsigned int i = 0; i < nBufferFrames; i++) {
        *samples++ = engine->tick();
    }
    return 0; // Return 0 for success
}




// Corrected constructor name
MantaSoundTexture::MantaSoundTexture() {
    // Set the global sample rate for all STK objects
    stk::Stk::setSampleRate(44100.0);
}

void MantaSoundTexture::calculateLowPassCoefficients(stk::StkFloat cutoff_hz, stk::StkFloat Q) {
    stk::StkFloat sampleRate = stk::Stk::sampleRate();
    if (cutoff_hz >= sampleRate / 2.0) {
        cutoff_hz = (sampleRate / 2.0) - 1.0;
    }
    
    stk::StkFloat w0 = 2.0 * M_PI * cutoff_hz / sampleRate;
    stk::StkFloat alpha = sin(w0) / (2.0 * Q);
    stk::StkFloat cos_w0 = cos(w0);

    stk::StkFloat b0 = (1.0 - cos_w0) / 2.0;
    stk::StkFloat b1 = 1.0 - cos_w0;
    stk::StkFloat b2 = (1.0 - cos_w0) / 2.0;
    stk::StkFloat a0 = 1.0 + alpha;
    stk::StkFloat a1 = -2.0 * cos_w0;
    stk::StkFloat a2 = 1.0 - alpha;

    filter.setCoefficients(b0/a0, b1/a0, b2/a0, a1/a0, a2/a0);
}

void MantaSoundTexture::setSpeed(float speed) {
    whine.setFrequency(getFrequencyForSpeed(speed)); 
    // Use the new mapping for a better sound
    calculateLowPassCoefficients(mapSpeedToFilterCutoff(speed));
}

stk::StkFloat MantaSoundTexture::tick() {
    stk::StkFloat noise_sample = filter.tick(noise.tick());
    stk::StkFloat whine_sample = whine.tick();
    return (noise_sample * noise_gain) + (whine_sample * whine_gain);
}

// The engine sound is never "done"
bool MantaSoundTexture::isDone() const  
{ 
    return false; 
}


// Prepares the audio stream. The filename is ignored for procedural audio.
void MantaSoundTexture::init(char filename[256]) 
{
    if (dac.isStreamOpen()) return;

    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = 2; // Mono
    unsigned int bufferFrames = stk::RT_BUFFER_SIZE;

    try {
        dac.openStream(&parameters, NULL, RTAUDIO_FLOAT64,
                        stk::Stk::sampleRate(), &bufferFrames,
                        &audioCallback, (void *)this); // Pass a pointer to this object
    } catch (stk::StkError &e) {
        e.printMessage();
    }
}

// Starts the audio stream playback.
void MantaSoundTexture::play() 
{
    try {
        if (dac.isStreamOpen() && !dac.isStreamRunning()) {
            dac.startStream();
        }
    } catch (stk::StkError &e) {
        e.printMessage();
    }
}

// Stops and closes the audio stream.
void MantaSoundTexture::close() 
{
    try {
        if (dac.isStreamOpen()) {
            dac.stopStream();
            dac.closeStream();
        }
    } catch (stk::StkError &e) {
        e.printMessage();
    }
    
    // Call the base class method if it has cleanup tasks
    SoundTexture::close();
}

