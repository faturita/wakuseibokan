#ifndef SOUNDRENDER_H
#define SOUNDRENDER_H

#include "stk/FileWvIn.h"
#include "stk/RtAudio.h"
#include "SoundSource.h"

void play(SoundSource* p);
void close(SoundSource *p);

#endif // SOUNDRENDER_H