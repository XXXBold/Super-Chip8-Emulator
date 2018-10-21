#include <stdio.h>
#include <stdlib.h>
#ifndef SDL_MAIN_HANDLED
  #define SDL_MAIN_HANDLED
#endif /* !SDL_MAIN_HANDLED */
#include <SDL2/SDL.h>

#include "Chip8_screen.h"

static SDL_Window *ptagChip8Window_m;
static SDL_Renderer *ptagChip8Renderer_m;

int iChip8_Screen_Init_g(void)
{
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
  {
    fprintf(stderr,"SDL_Init Error: %s\n",SDL_GetError());
    return(1);
  }
  if(!(ptagChip8Window_m=SDL_CreateWindow("Chip-8 Emulator",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          CHIP8_SCREEN_WIDTH*CHIP8_WINDOW_STRETCH_FACTOR,
                                          CHIP8_SCREEN_HEIGHT*CHIP8_WINDOW_STRETCH_FACTOR,
                                          0)))
  {
    fprintf(stderr,"SDL_CreateWindow() failed: %s\n",SDL_GetError());
    return(1);
  }
  if(!(ptagChip8Renderer_m=SDL_CreateRenderer(ptagChip8Window_m,-1,SDL_RENDERER_ACCELERATED)))
  {
    SDL_DestroyWindow(ptagChip8Window_m);
    fprintf(stderr,"SDL_CreateRenderer() failed: %s\n",SDL_GetError());
    return(1);
  }
  if(SDL_SetRenderDrawColor(ptagChip8Renderer_m,0,0,0,SDL_ALPHA_OPAQUE))
    fprintf(stderr,"SDL_SetRenderDrawColor() failed: %s\n",SDL_GetError());

  SDL_RenderClear(ptagChip8Renderer_m);
  return(0);
}

void vChip8_Screen_Close_g(void)
{
  SDL_DestroyRenderer(ptagChip8Renderer_m);
  SDL_DestroyWindow(ptagChip8Window_m);
  SDL_Quit();
}

int iChip8_Screen_Update_g(byte taScreenBuffer[CHIP8_SCREEN_WIDTH/8][CHIP8_SCREEN_HEIGHT])
{
  int iIndex;
  int iIndexB;
  int iCount;
  SDL_Rect tagRects[CHIP8_SCREEN_WIDTH];
  static byte taLastScreen[CHIP8_SCREEN_WIDTH/8][CHIP8_SCREEN_HEIGHT]={{1}};

  if(memcmp(taLastScreen,taScreenBuffer,sizeof(taLastScreen))==0)
  {
    return(0);
  }
  if(SDL_SetRenderDrawColor(ptagChip8Renderer_m,0,0,0,SDL_ALPHA_OPAQUE))
    fprintf(stderr,"SDL_SetRenderDrawColor() failed: %s\n",SDL_GetError());

  if(SDL_RenderClear(ptagChip8Renderer_m))
    fprintf(stderr,"SDL_RenderClear() failed: %s\n",SDL_GetError());

  if(SDL_SetRenderDrawColor(ptagChip8Renderer_m,255,255,255,SDL_ALPHA_OPAQUE))
    fprintf(stderr,"SDL_SetRenderDrawColor() failed: %s\n",SDL_GetError());

  for(iIndex=0;iIndex<CHIP8_SCREEN_HEIGHT;++iIndex)
  {
    iCount=0;
    for(iIndexB=0;iIndexB<CHIP8_SCREEN_WIDTH;++iIndexB)
    {
//    printf("screenbuffer[%u][%u]=0x%.2X checking bit %u\n",
//           (iIndexB/8),
//           iIndex,
//           taScreenBuffer[(iIndexB/8)][iIndex],
//           iIndexB%8);
      if(taScreenBuffer[(iIndexB/8)][iIndex]&(1<<(7-iIndexB%8)))
      {
        tagRects[iCount].y=(iIndex)*CHIP8_WINDOW_STRETCH_FACTOR;
        tagRects[iCount].x=(iIndexB)*CHIP8_WINDOW_STRETCH_FACTOR;
        tagRects[iCount].h=CHIP8_WINDOW_STRETCH_FACTOR;
        tagRects[iCount].w=CHIP8_WINDOW_STRETCH_FACTOR;
//      printf("screenbuffer[%u][%u] Drawing Rect @%u/%u\n",
//             (iIndexB/8),
//             iIndex,
//             tagRects[iCount].x,
//             tagRects[iCount].y);
        ++iCount;
      }
    }
    SDL_RenderFillRects(ptagChip8Renderer_m,tagRects,iCount);
  }
  SDL_RenderPresent(ptagChip8Renderer_m);
  memcpy(taLastScreen,taScreenBuffer,sizeof(taLastScreen));
  return(0);
}
