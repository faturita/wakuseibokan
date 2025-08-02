#include <stk/FileWvIn.h>
#include <stk/RtAudio.h>

#include <signal.h>
#include <iostream>
#include <cstdlib>

#include "Player.h"

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

    // Find out how many input channels we have.
    channels = file.channelsOut();

    interrupt = false;  // Reset the interrupt flag
    done = false;       // Reset the done flag
}

stk::StkFloat Player::tick()  {
    return file.tick();
}

bool Player::isDone() const {
    return file.isFinished();
}
