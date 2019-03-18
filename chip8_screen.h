#ifndef CHIP8_SCREEN_H_INCLUDED
  #define CHIP8_SCREEN_H_INCLUDED

#include "chip8_global.h"

int iChip8_Screen_Init_g(TagWindow *pWinData);
void vChip8_Screen_Close_g(TagWindow *pWinData);

void vChip8_Screen_Show_g(TagWindow *pWinData,
                          int iShow);
int iChip8_Screen_SetScale_g(TagWindow *pWinData);
void vChip8_Screen_SetWinPosition_g(TagWindow *pWinData);

int iChip8_Screen_ExMode_Enable(TagWindow *pWinData);
void vChip8_Screen_ExMode_Disable(TagWindow *pWinData);

void vChip8_Screen_Clear_g(TagWindow *pWinData);

void vChip8_Screen_Draw_g(TagWindow *pWinData);

#endif /* CHIP8_SCREEN_H_INCLUDED */
