#ifndef PLAYER_H
#define PLAYER_H

#include "stk/FileWvIn.h"
#include "stk/RtAudio.h"
#include "SoundSource.h"

class Player : public SoundSource {
private:
    RtAudio dac;

public:
    int channels = 2; // Default to stereo
    stk::FileWvIn file;
    stk::StkFloat amplitude = 1.0;
    bool done = false;
    bool interrupt = false;

    void init(char fl[256]) ;

    stk::StkFloat tick() override ;

    bool isDone() const override ;

    void play() ;

    void close(); 
};

#endif // PLAYER_H