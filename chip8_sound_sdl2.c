#include <stdio.h>
#ifndef SDL_MAIN_HANDLED
  #define SDL_MAIN_HANDLED
#endif /* !SDL_MAIN_HANDLED */
#include <SDL2/SDL.h>

#include "chip8_sound.h"
#include "chip8_global.h"

int iChip8_Sound_Init_g(TagPlaySound *pSoundData)
{
  unsigned int uiIndex;
  unsigned int uiWaveGen;
  TRACE_DBG_INFO("Initialise Sound...");
  pSoundData->iSoundPlaying=0;

  if(SDL_InitSubSystem(SDL_INIT_AUDIO))
  {
    TRACE_DBG_ERROR_VARG("SDL_InitSubSystem() failed: %s",SDL_GetError());
    return(-1);
  }
  SDL_AudioSpec desiredSpec;
  SDL_AudioSpec obtainedSpec;
  SDL_zero(desiredSpec);

  desiredSpec.freq = EMU_PLAYSOUND_SAMPLE_RATE_HZ;
  desiredSpec.format = AUDIO_S8;
  desiredSpec.channels = EMU_PLAYSOUND_CHANNELS;
  desiredSpec.samples = EMU_PLAYSOUND_SAMPLES_BUFFER;
  desiredSpec.callback = NULL;
  desiredSpec.userdata = NULL;

  if(!(pSoundData->tDevID=SDL_OpenAudioDevice(NULL,
                                              0,
                                              &desiredSpec,
                                              &obtainedSpec,
                                              0)))
  {
    TRACE_DBG_ERROR_VARG("SDL_OpenAudioDevice() failed: %s",SDL_GetError());
    return(-1);
  }
  if (obtainedSpec.freq != desiredSpec.freq || obtainedSpec.format != desiredSpec.format
    || obtainedSpec.channels != desiredSpec.channels || obtainedSpec.samples != desiredSpec.samples)
  {
    SDL_CloseAudioDevice(pSoundData->tDevID);
    TRACE_DBG_ERROR("Failed to initialize desired SDL_OpenAudio!");
    return(-1);
  }
  SDL_PauseAudioDevice(pSoundData->tDevID,1); /* Pause device */
  /* Create square wave for frequency */
  uiWaveGen=EMU_PLAYSOUND_SAMPLE_RATE_HZ/pSoundData->uiFreqSoundHz+0.5;
  for(uiIndex=0;uiIndex<sizeof(pSoundData->caSoundBuffer);++uiIndex)   // TODO: probably improve tone generation, but for now it's okay
  {
    if((sizeof(pSoundData->caSoundBuffer)-uiIndex<uiWaveGen) && (!(uiIndex%uiWaveGen))) /* Make buffer concatenateable */
      break;
    pSoundData->caSoundBuffer[uiIndex]=(uiIndex%(uiWaveGen)<(uiWaveGen)/2)?EMU_PLAYSOUND_AMPLITUDE:-EMU_PLAYSOUND_AMPLITUDE;
  }
  pSoundData->uiUsedSoundSize=uiIndex;
  return(0);
}

void vChip8_Sound_Close_g(TagPlaySound *pSoundData)
{
  TRACE_DBG_INFO("Close Sound...");
  SDL_ClearQueuedAudio(pSoundData->tDevID);
  SDL_CloseAudioDevice(pSoundData->tDevID);
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void vChip8_Sound_Start_g(TagPlaySound *pSoundData)
{
  if(pSoundData->iSoundPlaying)
    return;
  while(SDL_GetQueuedAudioSize(pSoundData->tDevID)<EMU_PLAYSOUND_SAMPLE_RATE_HZ*5) /* Prepare data for 5s continously playing */
    SDL_QueueAudio(pSoundData->tDevID,pSoundData->caSoundBuffer,pSoundData->uiUsedSoundSize);
  SDL_PauseAudioDevice(pSoundData->tDevID,0); /* Start Playback */
  pSoundData->iSoundPlaying=1;
}

void vChip8_Sound_Stop_g(TagPlaySound *pSoundData)
{
  if(!pSoundData->iSoundPlaying)
    return;
  SDL_PauseAudioDevice(pSoundData->tDevID,1); /* pause Playback */
  pSoundData->iSoundPlaying=0;
}
