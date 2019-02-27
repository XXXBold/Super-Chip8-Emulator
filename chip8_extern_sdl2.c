#ifndef SDL_MAIN_HANDLED
  #define SDL_MAIN_HANDLED
#endif /* !SDL_MAIN_HANDLED */
#include <SDL2/SDL.h>

#include <stdio.h>

#include "chip8_extern.h"
#include "chip8_runthread.h"

int iChip8_External_Init_g(TagEmulator *pEmulator)
{
  TRACE_DBG_INFO("Initialise SDL Main...");
  if(SDL_Init(0))
  {
    TRACE_DBG_ERROR_VARG("SDL_Init() failed: %s",SDL_GetError());
    return(-1);
  }
  if(iChip8_RunThread_Init_g(pEmulator))
  {
    TRACE_DBG_ERROR("iChip8_RunThread_Init_g() failed");
    SDL_Quit();
    return(-1);
  }
  return(0);
}

void vChip8_External_Close_g(TagEmulator *pEmulator)
{
  vChip8_RunThread_Close_g(pEmulator);
  SDL_Quit();
}
