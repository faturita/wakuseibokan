#ifndef SOUNDSOURCE_H
#define SOUNDSOURCE_H

#include "stk/Stk.h"

// A generic base class for anything that can produce sound.
class SoundSource {
    // Done is the flag that indicates that the sound processing pipeline has been finished.
    // Interrupt is used to stop the sound processing pipeline (that will trigger the done flag).
    bool finished = true;
    bool interruption = false;

public:
    stk::StkFloat amplitude = 1.0;

    virtual ~SoundSource() = default;

    // The main function to get the next audio sample.
    virtual stk::StkFloat tick() = 0;

    // A way to check if the sound has finished playing (for one-shot sounds).
    virtual bool isDone() const {
        return false;
    }

    virtual void finish() {
        finished = true;
    }

    virtual void started() {
        finished = false;
        interruption = false;
    }

    virtual void interrupt() {
        interruption = true;
    }

    virtual bool isFinished() {
        return finished;
    }

    virtual bool isInterrupted() {
        return interruption;
    }

};

#endif // SOUNDSOURCE_H
