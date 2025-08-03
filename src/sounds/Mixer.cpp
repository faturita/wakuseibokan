#include <vector>
#include <mutex>
#include <algorithm> // For std::remove_if

#include "Mixer.h"


// Add a new sound to be played.
void Mixer::addSource(SoundSource* source) {
    std::lock_guard<std::mutex> guard(sourceMutex);
    if (source) {
        sources.push_back(source);
    }
}

bool Mixer::isEmpty() {
    std::lock_guard<std::mutex> guard(sourceMutex);
    return sources.empty();
}


// The main tick function that the audio callback will call.
stk::StkFloat Mixer::tick() {
    std::lock_guard<std::mutex> guard(sourceMutex);

    if (sources.empty()) {
        return 0.0;
    }

    stk::StkFloat mixed_sample = 0.0;

    // Tick all sources and add their output together
    for (SoundSource* src : sources) {
        if (src || src->isDone()) {
            continue; // Skip sources that are done
        }
        mixed_sample += src->tick();
    }

    // Clean up any sources that are finished playing
    sources.erase(
        std::remove_if(sources.begin(), sources.end(), [](SoundSource* s) {
            if (s->isDone()) {
                delete s; // Free the memory
                std::cout << "Removed finished sound source." << std::endl;
                return true;
            }
            return false;
        }),
        sources.end()
    );

    // Basic hard clipping to prevent distortion. More advanced limiting could be used here.
    //if (mixed_sample > 1.0) mixed_sample = 1.0;
    //if (mixed_sample < -1.0) mixed_sample = -1.0;

    return mixed_sample;
}
