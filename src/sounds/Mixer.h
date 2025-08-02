#include <vector>
#include <mutex>
#include <algorithm> // For std::remove_if

#include "SoundSource.h"

class Mixer {
private:
    std::vector<SoundSource*> sources;
    std::mutex sourceMutex; // To protect the vector from race conditions

public:
    // Add a new sound to be played.
    void addSource(SoundSource* source) ;

    // The main tick function that the audio callback will call.
    stk::StkFloat tick() ;

    bool isEmpty() ;
};