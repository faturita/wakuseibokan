#ifndef SOUNDSOURCE_H
#define SOUNDSOURCE_H

#include "stk/Stk.h"

// A generic base class for anything that can produce sound.
class SoundSource {
public:
    stk::StkFloat amplitude = 1.0;
    // Done is the flag that indicates that the sound processing pipeline has been finished.
    // Interrupt is used to stop the sound processing pipeline (that will trigger the done flag).
    bool done = true;
    bool interrupt = false;
    
    virtual ~SoundSource() = default;

    // The main function to get the next audio sample.
    virtual stk::StkFloat tick() = 0;

    // A way to check if the sound has finished playing (for one-shot sounds).
    virtual bool isDone() const {
        return false;
    }
};

#endif // SOUNDSOURCE_H
