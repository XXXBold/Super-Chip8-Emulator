#ifndef CHIP8_RUNTHREAD_H_INCLUDED
  #define CHIP8_RUNTHREAD_H_INCLUDED

#include "chip8_global.h"

int iChip8_RunThread_Init_g(TagEmulator *pEmulator);
void vChip8_RunThread_Close_g(TagEmulator *pEmulator);

#endif /* CHIP8_RUNTHREAD_H_INCLUDED */
