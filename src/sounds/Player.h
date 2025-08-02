#ifndef PLAYER_H
#define PLAYER_H

#include "stk/FileWvIn.h"
#include "stk/RtAudio.h"
#include "SoundSource.h"

class Player : public SoundSource {
private:
    
public:
    int channels = 2; // Default to stereo
    stk::FileWvIn file;
    stk::StkFloat amplitude = 1.0;

    // Done is the flag that indicates that the sound processing pipeline has been finished.
    // Interrupt is used to stop the sound processing pipeline (that will trigger the done flag).
    bool done = true;
    bool interrupt = false;

    void init(char fl[256]) ;

    stk::StkFloat tick() override ;

    bool isDone() const override ;
};

#endif // PLAYER_H