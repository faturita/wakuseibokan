
#ifndef SOUNDMIXER_H
#define SOUNDMIXER_H


#include "SoundSource.h"
#include "Player.h"
#include "MantaSoundTexture.h"

class SoundMixer : public SoundSource {
private:

    Player player;
    MantaSoundTexture mantaSound;


public:
    // Initialize the sound mixer with default settings
    SoundMixer() ;
    // Set the sound source to be played
    void setPlayerSource(char* filename);
    void setVehicleSpeed() ;
    stk::StkFloat tick() override ;

    bool isDone() const override ;
    void setEnabled(bool enable) ;

    void finish() override;

    void started() override;

    void interrupt() override;

    bool isFinished() override;

    bool isInterrupted() override; 

};

#endif // SOUNDMIXER_H