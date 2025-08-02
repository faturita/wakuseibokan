#ifndef PLAYER_H
#define PLAYER_H

#include "stk/FileWvIn.h"
#include "stk/RtAudio.h"
#include "SoundSource.h"

class Player : public SoundSource {
private:
    stk::FileWvIn file;
public:
    int channels = 2; // Default to stereo

    void init(char fl[256]) ;

    stk::StkFloat tick() override ;

    bool isDone() const override ;
};

#endif // PLAYER_H