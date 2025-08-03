
#ifndef SOUNDGENERATOR_H
#define SOUNDGENERATOR_H


#include "SoundSource.h"
#include "Player.h"
#include "MantaSoundTexture.h"

class SoundGenerator : public SoundSource {
private:

    stk::FileWvIn file;
    MantaSoundTexture mantaSound;


public:
    // Initialize the sound mixer with default settings
    SoundGenerator() ;
    // Set the sound source to be played
    void setPlayerSource(char* filename);
    void setVehicleSpeed(float speed);
    stk::StkFloat tick() override ;

    bool isDone() const override ;
    void enableBackground(bool enable);

};

#endif // SOUNDGENERATOR_H