#include "SoundGenerator.h"


// Initialize the sound mixer with default settings
SoundGenerator::SoundGenerator() {

}

// Set the sound source to be played
void SoundGenerator::setPlayerSource(char* filename) {
    try {
        if (file.isOpen()) {
            file.closeFile();
        }
        file.openFile(filename);
    } catch (stk::StkError &e) {
        e.printMessage();
    }

    double rate = 1.0;
    rate = file.getFileRate() / stk::Stk::sampleRate();
    rate *= 1.0;
    file.setRate(rate);
    file.ignoreSampleRateChange();
}

void SoundGenerator::setVehicleSpeed(float speed) {
    mantaSound.setSpeed(speed); // Example speed setting
}

stk::StkFloat SoundGenerator::tick()  {
    stk::StkFloat playerSample = 0.0;

    if (!file.isFinished()) {
        playerSample = file.tick();
    }

    if (mantaSound.isEnabled()) {
        playerSample += mantaSound.tick() * 0.1;
    }

    return playerSample;
}

bool SoundGenerator::isDone() const  {
    return false;
}

void SoundGenerator::enableBackground(bool enable) {
    std::cout << "SoundGenerator: enableBackground called with " << (enable ? "true" : "false") << std::endl;
    mantaSound.setEnabled(enable);
}
