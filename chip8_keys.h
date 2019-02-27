#ifndef CHIP8_KEYS_H_INCLUDED
  #define CHIP8_KEYS_H_INCLUDED

#include "chip8_global.h"

int iChip8_Keys_Init_g(TagKeyboard *ptagKeyboard);
void vChip8_Keys_Close_g(TagKeyboard *ptagKeyboard);

int iChip8_Keys_SetKeymap_g(TagKeyboard *ptagKeyBoard);

void vChip8_Keys_UpdateState_g(TagKeyboard *ptagKeyboard);

#endif /* CHIP8_KEYS_H_INCLUDED */
