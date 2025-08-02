#ifndef SOUNDRENDER_H
#define SOUNDRENDER_H

#include "stk/FileWvIn.h"
#include "stk/RtAudio.h"
#include "SoundSource.h"
#include "Player.h"

void play(Player* p);
void close(Player *p);

#endif // SOUNDRENDER_H