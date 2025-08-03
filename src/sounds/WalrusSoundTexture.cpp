#include "WalrusSoundTexture.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Mapping Functions for Speedboat Sound ---

// Maps speed to the LFO frequency, creating the "chug-chug" effect.
// Higher speed = faster pulsation.
float mapSpeedboatSpeedToLfoFreq(float speed) {
    const float speed_start = 0.0f;
    const float speed_end = 100.0f; // Speed in knots or similar
    const float freq_start = 4.0f;  // Slow "chug" at idle
    const float freq_end = 30.0f;   // Fast pulsation at max speed
    
    if (speed < speed_start) speed = speed_start;
    if (speed > speed_end) speed = speed_end;

    float ratio = (speed - speed_start) / (speed_end - speed_start);
    return freq_start + ratio * (freq_end - freq_start);
}

// Maps speed to the core engine hum.
float mapSpeedboatSpeedToEngineHum(float speed) {
    const float speed_start = 0.0f;
    const float speed_end = 100.0f;
    const float freq_start = 80.0f;  // Low hum at idle
    const float freq_end = 450.0f; // High RPM hum
    
    if (speed < speed_start) speed = speed_start;
    if (speed > speed_end) speed = speed_end;

    float ratio = (speed - speed_start) / (speed_end - speed_start);
    return freq_start + ratio * (freq_end - freq_start);
}

// Maps speed to the center frequency of the band-pass filter.
// This makes the engine roar sound more focused and intense at high speeds.
float mapSpeedboatSpeedToFilterFreq(float speed) {
    const float speed_start = 0.0f;
    const float speed_end = 100.0f;
    const float freq_start = 300.0f;
    const float freq_end = 1200.0f;
    
    if (speed < speed_start) speed = speed_start;
    if (speed > speed_end) speed = speed_end;

    float ratio = (speed - speed_start) / (speed_end - speed_start);
    return freq_start + ratio * (freq_end - freq_start);
}


// --- Class Implementation ---

WalrusSoundTexture::WalrusSoundTexture() {
    // This should ideally be set once globally, not per instance.
    stk::Stk::setSampleRate(44100.0);
}

void WalrusSoundTexture::calculateBandPassCoefficients(stk::StkFloat center_hz, stk::StkFloat Q) {
    stk::StkFloat sampleRate = stk::Stk::sampleRate();
    if (center_hz >= sampleRate / 2.0) {
        center_hz = (sampleRate / 2.0) - 1.0;
    }
    
    stk::StkFloat w0 = 2.0 * M_PI * center_hz / sampleRate;
    stk::StkFloat alpha = sin(w0) / (2.0 * Q);
    stk::StkFloat cos_w0 = cos(w0);

    // Band-pass filter coefficients
    stk::StkFloat b0 = alpha;
    stk::StkFloat b1 = 0;
    stk::StkFloat b2 = -alpha;
    stk::StkFloat a0 = 1.0 + alpha;
    stk::StkFloat a1 = -2.0 * cos_w0;
    stk::StkFloat a2 = 1.0 - alpha;

    filter.setCoefficients(b0/a0, b1/a0, b2/a0, a1/a0, a2/a0);
}

void WalrusSoundTexture::setSpeed(float speed) {
    // Update all sound parameters based on the new speed
    lfo.setFrequency(mapSpeedboatSpeedToLfoFreq(speed));
    engineHum.setFrequency(mapSpeedboatSpeedToEngineHum(speed));
    calculateBandPassCoefficients(mapSpeedboatSpeedToFilterFreq(speed));

    // Reset the time-to-live to keep the sound active
    ttl = 10000;
    enabled = true;
}

stk::StkFloat WalrusSoundTexture::tick() {
    if (!enabled) {
        return 0.0;
    }

    // --- Sound Synthesis ---
    // 1. Get the LFO value. It swings from -1 to 1.
    stk::StkFloat lfo_val = lfo.tick();
    // Scale it to be a modulator (e.g., from 0.5 to 1.0)
    stk::StkFloat modulator = 0.75 + (lfo_val * 0.25);
    
    // 2. Generate noise, filter it, and apply the LFO modulation.
    stk::StkFloat noise_sample = noise.tick();
    stk::StkFloat filtered_noise = filter.tick(noise_sample);
    stk::StkFloat modulated_noise = filtered_noise * modulator;

    // 3. Get the core engine hum.
    stk::StkFloat hum_sample = engineHum.tick();

    // 4. Mix the two parts together.
    stk::StkFloat final_sample = (modulated_noise * noise_gain) + (hum_sample * hum_gain);
    
    // --- State Management ---
    ttl--;
    if (ttl <= 0) {
        enabled = false; // Disable after ttl expires
    }

    return final_sample;
}

bool WalrusSoundTexture::isDone() const {
    // The sound is "done" when it's no longer enabled.
    return !enabled;
}

void WalrusSoundTexture::setEnabled(bool enable) {
    enabled = enable;
    if (enabled) {
        ttl = 10000; // Reset ttl if re-enabled manually
    }
}

bool WalrusSoundTexture::isEnabled() const {
    return enabled;
}