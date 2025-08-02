#ifndef MANTASOUNDTEXTURE_H
#define MANTASOUNDTEXTURE_H

#include "stk/Stk.h"
#include "stk/SineWave.h"
#include "stk/Noise.h"
#include "stk/BiQuad.h"
#include "stk/RtAudio.h" 

#include "SoundSource.h"


class MantaSoundTexture : public SoundSource {
public:
    stk::Noise noise;
    stk::BiQuad filter;
    stk::SineWave whine;

    stk::StkFloat noise_gain = 0.6;
    stk::StkFloat whine_gain = 0.4;

    MantaSoundTexture() ;

    // NEW HELPER FUNCTION to calculate and set filter coefficients
    void calculateLowPassCoefficients(stk::StkFloat cutoff_hz, stk::StkFloat Q = 0.707) ;

    void setSpeed(float speed) ;
    // The tick function remains correct
    stk::StkFloat tick() ;

    void init(char filename[256]);
    void play();
    void close();
};

#endif // MANTASOUNDTEXTURE_H