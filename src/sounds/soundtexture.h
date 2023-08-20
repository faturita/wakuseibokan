#ifndef SOUNDTEXTURE_H
#define SOUNDTEXTURE_H

#include <FileWvIn.h>
#include <RtAudio.h>

#include <signal.h>
#include <iostream>
#include <cstdlib>

using namespace stk;

class SoundTexture
{
public:
    bool done;
    StkFrames frames;
    FileWvIn input;
    RtAudio dac;
    char filename[256];


public:
    SoundTexture();
    ~SoundTexture();

    void init(char filename[256]);
    void play();
    void close();
};

#endif // SOUNDTEXTURE_H
