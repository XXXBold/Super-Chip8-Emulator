#ifndef CHIP8_SCREEN_H_INCLUDED
  #define CHIP8_SCREEN_H_INCLUDED

#include "Chip8_Emulator.h"

#define CHIP8_SCREEN_WIDTH        64
#define CHIP8_SCREEN_HEIGHT       32

#define CHIP8_WINDOW_STRETCH_FACTOR 10

int iChip8_Screen_Init_g(void);
void vChip8_Screen_Close_g(void);
int iChip8_Screen_Update_g(byte taScreenBuffer[CHIP8_SCREEN_WIDTH/8][CHIP8_SCREEN_HEIGHT]);

#endif /* CHIP8_SCREEN_H_INCLUDED */
