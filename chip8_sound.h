#ifndef CHIP8_SOUND_H_INCLUDED
  #define CHIP8_SOUND_H_INCLUDED

#include "chip8_global.h"

int iChip8_Sound_Init_g(TagPlaySound *pSoundData);
void vChip8_Sound_Close_g(TagPlaySound *pSoundData);
void vChip8_Sound_Start_g(TagPlaySound *pSoundData);
void vChip8_Sound_Stop_g(TagPlaySound *pSoundData);

#endif /* CHIP8_SOUND_H_INCLUDED */
