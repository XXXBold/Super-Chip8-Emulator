#include <stdio.h>
#include <stdlib.h>

#include "chip8_screen.h"
#include "chip8_global.h"

#define MEM_BIT_CHECK(mem,bit)     ((mem)&(0x80>>((bit)%8)))
#define SCREEN_RECTS_MAX_DRAW      0x80
#define SCREEN_RECTS_BLACK_OFFSET  SCREEN_RECTS_MAX_DRAW-1

INLINE_PROT void vChip8_Screen_DrawRects_m(TagWindow *pWinData,
                                           SDL_Rect *ptagRects,
                                           unsigned int uiFGCount,
                                           unsigned int uiBGCount);

int iChip8_Screen_Init_g(TagWindow *pWinData)
{
  SDL_DisplayMode tagDPM;
  TRACE_DBG_INFO("Initialise Screen...");

  pWinData->data.exModeOn=0;
  pWinData->data.visible=0;

  if(SDL_InitSubSystem(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK))   // TODO: version check for joystick?
  {
    TRACE_DBG_ERROR_VARG("SDL_Init Error: %s",SDL_GetError());
    return(-1);
  }
  if(SDL_GetDesktopDisplayMode(0,&tagDPM))
  {
    TRACE_DBG_ERROR_VARG("SDL_GetDesktopDisplayMode() failed: %s",SDL_GetError());
    SDL_QuitSubSystem(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
    return(-1);
  }
  pWinData->iMonitorWidth  =tagDPM.w;
  pWinData->iMonitorHeigth =tagDPM.h;
  if(!tagDPM.refresh_rate)
  {
    TRACE_DBG_ERROR("Monitor refresh frequency not defined, falling back to 60Hz...");
    pWinData->iMonitorHz=60;
  }
  else
    pWinData->iMonitorHz=tagDPM.refresh_rate;

  TRACE_DBG_INFO_VARG("Display width: %dpx, height: %dpx, @%dHz",
                      pWinData->iMonitorWidth,
                      pWinData->iMonitorHeigth,
                      pWinData->iMonitorHz);

  if(!(pWinData->pWindow=SDL_CreateWindow("Chip-8 Emulator",
                                          pWinData->iPosX,
                                          pWinData->iPosY,
                                          CHIP8_SCREEN_WIDTH*EMU_SCREEN_DEFAULT_SCALE,
                                          CHIP8_SCREEN_HEIGHT*EMU_SCREEN_DEFAULT_SCALE,
                                          SDL_WINDOW_HIDDEN)))
  {
    TRACE_DBG_ERROR_VARG("SDL_CreateWindow() failed: %s",SDL_GetError());
    return(-1);
  }
  if(!(pWinData->pRenderer=SDL_CreateRenderer(pWinData->pWindow,-1,SDL_RENDERER_ACCELERATED)))
  {
    SDL_DestroyWindow(pWinData->pWindow);
    TRACE_DBG_ERROR_VARG("SDL_CreateRenderer() failed: %s",SDL_GetError());
    return(-1);
  }
  if(iChip8_Screen_SetScale_g(pWinData))
  {
    SDL_DestroyRenderer(pWinData->pRenderer);
    SDL_DestroyWindow(pWinData->pWindow);
    return(-1);
  }
  pWinData->data.visible=0;
  return(0);
}

void vChip8_Screen_Close_g(TagWindow *pWinData)
{
  TRACE_DBG_INFO("Close Screen...");
  SDL_DestroyRenderer(pWinData->pRenderer);
  SDL_DestroyWindow(pWinData->pWindow);
  SDL_QuitSubSystem(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
}

void vChip8_Screen_Show_g(TagWindow *pWinData,
                          int iShow)
{
  if(iShow)
  {
    if(pWinData->data.visible) /* If already visible, return */
      return;
    SDL_ShowWindow(pWinData->pWindow);
    SDL_RaiseWindow(pWinData->pWindow);
    tagEmulator_g.tagWindow.data.visible=1;
  }
  else
  {
    if(!pWinData->data.visible) /* If already hidden, return */
      return;
    /* needed to handle correct key (don't keep esc pressed virtually) */
    SDL_MinimizeWindow(pWinData->pWindow);
    SDL_RestoreWindow(pWinData->pWindow);
    SDL_HideWindow(pWinData->pWindow);
    pWinData->data.visible=0;
  }
}

int iChip8_Screen_SetScale_g(TagWindow *pWinData)
{
  int iRc=0;
  TRACE_DBG_INFO_VARG("Set Window scale to Factor %d",pWinData->data.scale);

  /* Check window still fits on current display */
  if((CHIP8_SCREEN_WIDTH*pWinData->data.scale>(unsigned)pWinData->iMonitorWidth) ||
     (CHIP8_SCREEN_HEIGHT*pWinData->data.scale>(unsigned)pWinData->iMonitorHeigth))
  {
    TRACE_DBG_ERROR_VARG("Can't Set new Scale, too big, falling back to default(=%u)",EMU_SCREEN_DEFAULT_SCALE);
    pWinData->data.scale=EMU_SCREEN_DEFAULT_SCALE;
    iRc=1;
  }
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
  if(pWinData->data.scale<2)
  {
    TRACE_DBG_ERROR_VARG("Can't Set scale lower than 2 (is: %u) if Superchip is enabled, setting to Minimum (=2)",pWinData->data.scale);
    pWinData->data.scale=2;
    iRc=2;
  }
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */
  SDL_SetWindowSize(pWinData->pWindow,
                    CHIP8_SCREEN_WIDTH*pWinData->data.scale,
                    CHIP8_SCREEN_HEIGHT*pWinData->data.scale);
  SDL_RenderSetScale(pWinData->pRenderer,(float)pWinData->data.scale/2.0,(float)pWinData->data.scale/2.0);
  return(iRc);
}

void vChip8_Screen_SetWinPosition_g(TagWindow *pWinData)
{
  SDL_SetWindowPosition(pWinData->pWindow,pWinData->iPosX,pWinData->iPosY);
}

void vChip8_Screen_Clear_g(TagWindow *pWinData)
{
  /* clear window contents, redraw... (window should be visible while calling this) */
  memset(pWinData->taLastScreen,0,sizeof(pWinData->taLastScreen));
  memset(pWinData->taScreenBuffer,0,sizeof(pWinData->taScreenBuffer));
  SDL_SetRenderDrawColor(pWinData->pRenderer,0,0,0,SDL_ALPHA_OPAQUE);
  SDL_RenderClear(pWinData->pRenderer);
  SDL_RenderPresent(pWinData->pRenderer);
}

int iChip8_Screen_ExMode_Enable(TagWindow *pWinData)
{
  /* Check if already in extended mode */
  if(pWinData->data.exModeOn)
  {
    TRACE_DBG_INFO("iChip8_Screen_ExMode_Enable() Already in Extended mode");
    return(0);
  }
  if(pWinData->data.scale<2)
  {
    TRACE_DBG_ERROR_VARG("iChip8_Screen_ExMode_Enable(): Can't set Extended mode, scale is too small (%u), should at least be 2.",
                         pWinData->data.scale);
    return(-1);
  }
  pWinData->data.exModeOn=1;
  return(0);
}

void vChip8_Screen_ExMode_Disable(TagWindow *pWinData)
{
  pWinData->data.exModeOn=0;
}

void vChip8_Screen_Draw_g(TagWindow *pWinData)
{
  SDL_Rect tagRects[SCREEN_RECTS_MAX_DRAW]; /* Max possible */
  unsigned int uiIndex;
  unsigned int uiIndexB;
  unsigned int uiFGRectsCount=0;
  unsigned int uiBGCountRects=SCREEN_RECTS_BLACK_OFFSET;
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
  const unsigned int uiScreenWidth=(pWinData->data.exModeOn)?SUCHIP_SCREEN_WIDTH:CHIP8_SCREEN_WIDTH;
  const unsigned char ucScale=(pWinData->data.exModeOn)?1:2;
#else
  const unsigned int uiScreenWidth=CHIP8_SCREEN_WIDTH;
  const unsigned char ucScale=2;
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */

  for(uiIndex=0;uiIndex<((pWinData->data.exModeOn)?SCREEN_SIZE_EXMODE:SCREEN_SIZE_NORMAL);++uiIndex)
  {
    /* If current byte is equal -> leave as is, continue */
    if(pWinData->taScreenBuffer[uiIndex]==pWinData->taLastScreen[uiIndex])
      continue;

    for(uiIndexB=0;uiIndexB<CHAR_BIT;++uiIndexB)
    {
      /* Check single pixel (bit) if it is equal - leave as is, continue */
      if(MEM_BIT_CHECK(pWinData->taScreenBuffer[uiIndex],uiIndexB)==MEM_BIT_CHECK(pWinData->taLastScreen[uiIndex],uiIndexB)) /* pixel not changed */
        continue;

      if(MEM_BIT_CHECK(pWinData->taScreenBuffer[uiIndex],uiIndexB))
      {
        tagRects[uiFGRectsCount].y=(uiIndex*8+uiIndexB)/uiScreenWidth*ucScale;
        tagRects[uiFGRectsCount].x=(uiIndex*8+uiIndexB)%uiScreenWidth*ucScale;
        tagRects[uiFGRectsCount].h=ucScale;
        tagRects[uiFGRectsCount].w=ucScale;
        ++uiFGRectsCount;
      }
      else
      {
        tagRects[uiBGCountRects].y=(uiIndex*8+uiIndexB)/uiScreenWidth*ucScale;
        tagRects[uiBGCountRects].x=(uiIndex*8+uiIndexB)%uiScreenWidth*ucScale;
        tagRects[uiBGCountRects].h=ucScale;
        tagRects[uiBGCountRects].w=ucScale;
        --uiBGCountRects;
      }
    }
    pWinData->taLastScreen[uiIndex]=pWinData->taScreenBuffer[uiIndex];
    if(uiBGCountRects-uiFGRectsCount>8) /* check still space in tagRects buffer */
      continue;
    /* No more space (means less than 8), in buffer, draw contents and reset buffer... */
    vChip8_Screen_DrawRects_m(pWinData,tagRects,uiFGRectsCount,uiBGCountRects);
    uiFGRectsCount=0;
    uiBGCountRects=SCREEN_RECTS_BLACK_OFFSET;
  }
  vChip8_Screen_DrawRects_m(pWinData,tagRects,uiFGRectsCount,uiBGCountRects);
  SDL_RenderPresent(pWinData->pRenderer);
}

INLINE_FCT void vChip8_Screen_DrawRects_m(TagWindow *pWinData,
                                          SDL_Rect *ptagRects,
                                          unsigned int uiFGCount,
                                          unsigned int uiBGCount)
{
  if(uiFGCount)
  {
    if(SDL_SetRenderDrawColor(pWinData->pRenderer,255,255,255,SDL_ALPHA_OPAQUE)) /* White */
    {
      TRACE_DBG_ERROR_VARG("SDL_SetRenderDrawColor() failed: %s",SDL_GetError());
      return;
    }

    SDL_RenderFillRects(pWinData->pRenderer,ptagRects,uiFGCount);
  }
  if(uiBGCount!=SCREEN_RECTS_BLACK_OFFSET)
  {
    if(SDL_SetRenderDrawColor(pWinData->pRenderer,0,0,0,SDL_ALPHA_OPAQUE)) /* Black */
    {
      TRACE_DBG_ERROR_VARG("SDL_SetRenderDrawColor() failed: %s",SDL_GetError());
      return;
    }

    SDL_RenderFillRects(pWinData->pRenderer,&ptagRects[uiBGCount+1],SCREEN_RECTS_BLACK_OFFSET-uiBGCount);
  }
}
