#include "soundtexture.h"

// This tick() function handles sample computation only.  It will be
// called automatically when the system needs a new buffer of audio
// samples.
int tick_callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{

  StkFloat *samples = (StkFloat *) outputBuffer;

  SoundTexture *so = (SoundTexture*) userData;
  FileWvIn *input = &so->input;

  input->tick( so->frames );

  for ( unsigned int i=0; i<so->frames.size(); i++ ) {
    *samples++ = so->frames[i];
    if ( input->channelsOut() == 1 ) *samples++ = so->frames[i]; // play mono files in stereo
  }

  if ( input->isFinished() ) {
    so->done = true;
    so->dac.closeStream();
    return 1;
  }
  else
    return 0;
}


SoundTexture::SoundTexture()
{

}



void SoundTexture::init(char fl[256])
{
    strcpy(filename, fl);

    // Try to load the soundfile.
    try {
      input.openFile( filename);
    }
    catch ( StkError & ) {
      exit( 1 );
    }

    done = false;

    // Set input read rate based on the default STK sample rate.
    double rate = 1.0;
    rate = input.getFileRate() / Stk::sampleRate();
    rate *= atof( "1.0" );
    input.setRate( rate );

    input.ignoreSampleRateChange();

}


void SoundTexture::play()
{

    // Find out how many channels we have.
    int channels = input.channelsOut();

    //if (dac.isStreamRunning() || dac.isStreamOpen())
    //    dac.closeStream();

    // Figure out how many bytes in an StkFloat and setup the RtAudio stream.
    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = ( channels == 1 ) ? 2 : channels; //  Play mono files as stereo.
    RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
    unsigned int bufferFrames = RT_BUFFER_SIZE;
    try {
      dac.openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &tick_callback, (void *)this );
    }
    catch ( RtAudioError &error ) {
      error.printMessage();
    }

    // Resize the StkFrames object appropriately.
    frames.resize( bufferFrames, channels );

    try {
      dac.startStream();
    }
    catch ( RtAudioError &error ) {
      error.printMessage();
    }
}

SoundTexture::~SoundTexture()
{

}

void SoundTexture::close()
{
    // Block waiting until callback signals done.
    //while ( !done )
      //Stk::sleep( 100 );

    // By returning a non-zero value in the callback above, the stream
    // is automatically stopped.  But we should still close it.
    try {
      dac.closeStream();
    }
    catch ( RtAudioError &error ) {
      error.printMessage();
    }
}
