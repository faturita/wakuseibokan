#ifndef SOUNDSOURCE_H
#define SOUNDSOURCE_H

#include "stk/Stk.h"

// A generic base class for anything that can produce sound.
class SoundSource {
public:
    virtual ~SoundSource() = default;

    // The main function to get the next audio sample.
    virtual stk::StkFloat tick() = 0;

    // A way to check if the sound has finished playing (for one-shot sounds).
    virtual bool isDone() const {
        return false;
    }
};

#endif // SOUNDSOURCE_H
