#ifndef WALRUSSOUNDTEXTURE_H
#define WALRUSSOUNDTEXTURE_H

#include "stk/Stk.h"
#include "stk/SineWave.h"
#include "stk/Noise.h"
#include "stk/BiQuad.h"

#include "SoundSource.h"

// Sound class for a speedboat engine.
// Uses a filtered noise source modulated by an LFO for the "chugging"
// sound, combined with a sine wave for the core engine hum.
class WalrusSoundTexture : public SoundSource {
private:
    bool enabled = false; // Flag to enable/disable the sound texture
    int ttl = 0;

public:
    // STK Sound Generators
    stk::Noise noise;
    stk::BiQuad filter;
    stk::SineWave engineHum; // The main tonal part of the engine
    stk::SineWave lfo;       // Low-Frequency Oscillator for the pulsating effect

    // Gain controls for mixing
    stk::StkFloat noise_gain = 0.7;
    stk::StkFloat hum_gain = 0.3;

    // --- Core Methods ---
    WalrusSoundTexture();

    void setSpeed(float speed);
    stk::StkFloat tick() override;
    bool isDone() const override;

    // --- State Management ---
    void setEnabled(bool enable);
    bool isEnabled() const;

private:
    // Helper function to set the filter coefficients
    void calculateBandPassCoefficients(stk::StkFloat center_hz, stk::StkFloat Q = 1.0);
};

#endif // WALRUSSOUNDTEXTURE_H