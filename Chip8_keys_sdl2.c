#include <stdio.h>
#include <stdlib.h>
#ifndef SDL_MAIN_HANDLED
  #define SDL_MAIN_HANDLED
#endif /* !SDL_MAIN_HANDLED */
#include <SDL2/SDL.h>

#include "Chip8_keys.h"

void vChip8_Keys_GetState_g(unsigned short *pusKeyMap)
{
  const Uint8 *puiTmp;

  SDL_PumpEvents();
  puiTmp=SDL_GetKeyboardState(NULL);

  *pusKeyMap=((puiTmp[SDL_SCANCODE_0])?0x1:0|
              (puiTmp[SDL_SCANCODE_1])?0x2:0|
              (puiTmp[SDL_SCANCODE_2])?0x4:0|
              (puiTmp[SDL_SCANCODE_3])?0x8:0|
              (puiTmp[SDL_SCANCODE_4])?0x10:0|
              (puiTmp[SDL_SCANCODE_5])?0x20:0|
              (puiTmp[SDL_SCANCODE_6])?0x40:0|
              (puiTmp[SDL_SCANCODE_7])?0x80:0|
              (puiTmp[SDL_SCANCODE_8])?0x100:0|
              (puiTmp[SDL_SCANCODE_9])?0x200:0|
              (puiTmp[SDL_SCANCODE_A])?0x400:0|
              (puiTmp[SDL_SCANCODE_B])?0x800:0|
              (puiTmp[SDL_SCANCODE_C])?0x1000:0|
              (puiTmp[SDL_SCANCODE_D])?0x2000:0|
              (puiTmp[SDL_SCANCODE_E])?0x4000:0|
              (puiTmp[SDL_SCANCODE_F])?0x8000:0);
}

unsigned char ucChip8_Keys_WaitForNext_g(void)
{
  SDL_Event tagEvent;

  while(1)
  {
    SDL_PollEvent(&tagEvent);
    if(tagEvent.type==SDL_KEYDOWN)
    {
      switch(tagEvent.key.keysym.scancode)
      {
        case SDL_SCANCODE_0:
          return(0x0);
        case SDL_SCANCODE_1:
          return(0x1);
        case SDL_SCANCODE_2:
          return(0x2);
        case SDL_SCANCODE_3:
          return(0x3);
        case SDL_SCANCODE_4:
          return(0x4);
        case SDL_SCANCODE_5:
          return(0x5);
        case SDL_SCANCODE_6:
          return(0x6);
        case SDL_SCANCODE_7:
          return(0x7);
        case SDL_SCANCODE_8:
          return(0x8);
        case SDL_SCANCODE_9:
          return(0x9);
        case SDL_SCANCODE_A:
          return(0xA);
        case SDL_SCANCODE_B:
          return(0xB);
        case SDL_SCANCODE_C:
          return(0xC);
        case SDL_SCANCODE_D:
          return(0xD);
        case SDL_SCANCODE_E:
          return(0xE);
        case SDL_SCANCODE_F:
          return(0xF);
        default:
          continue;
      }
    }
  }
}
