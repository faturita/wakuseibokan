#include "SoundMixer.h"


// Initialize the sound mixer with default settings
SoundMixer::SoundMixer() {
    player.amplitude = 1.0;
    mantaSound.amplitude = 1.0;
}

// Set the sound source to be played
void SoundMixer::setPlayerSource(char* filename) {
    player.init(filename);
}

void SoundMixer::setVehicleSpeed() {
    mantaSound.setSpeed(300.0); // Example speed setting
}

stk::StkFloat SoundMixer::tick()  {
    stk::StkFloat playerSample = player.tick();

    if (!mantaSound.isDone()) {
        playerSample += mantaSound.tick();
    }

    return playerSample;
}

bool SoundMixer::isDone() const  {
    return player.isDone() && mantaSound.isDone();
}

void SoundMixer::setEnabled(bool enable) {
    mantaSound.setEnabled(enable);
}

void SoundMixer::finish() {
    player.finish();
}

void SoundMixer::started() {
    player.started();
}

void SoundMixer::interrupt() {
    player.interrupt();
}

bool SoundMixer::isFinished() {
    return player.isFinished();
}

bool SoundMixer::isInterrupted() {
    return player.isInterrupted();
}