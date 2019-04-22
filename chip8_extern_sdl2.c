#include "chip8_extern.h"

enum
{
  EMU_SET_OPTION_OK,
  EMU_SET_OPTION_CHANGED,
  EMU_SET_OPTION_ERROR
};

#define MEM_BIT_CHECK(mem,bit)     ((mem)&(0x80>>((bit)%8)))
#define SCREEN_RECTS_MAX_DRAW      0x80
#define SCREEN_RECTS_BLACK_OFFSET  SCREEN_RECTS_MAX_DRAW-1

#define EMU_CALL_USER_EVENT(emu,evt) (((emu->tagThrdEmu.usrCBFunc) && (emu->tagThrdEmu.usrCBEvents&evt))?emu->tagThrdEmu.usrCBFunc(evt,emu->tCurrOPCode):0)


/**
 * Runthread related Functions
 */
INLINE_PROT int iChip8_RunThread_Init_m(TagEmulator *pEmulator);
INLINE_PROT void vChip8_RunThread_Close_m(TagEmulator *pEmulator);

INLINE_PROT int iThreadEmu_UpdateSettings_m(TagEmulator *pSettings,
                                            Uint64 *pExecDelayPerfCounts);

int iThread_EmuProcess_m(void* data);

/**
 * Screen related Functions
 */
INLINE_PROT int iChip8_Screen_Init_m(TagWindow *pWinData);
INLINE_PROT void vChip8_Screen_Close_m(TagWindow *pWinData);

INLINE_PROT void vChip8_Screen_SetTitle_m(TagWindow *pWinData);
INLINE_PROT void vChip8_Screen_Show_m(TagWindow *pWinData,
                                      int iShow);
INLINE_PROT int iChip8_Screen_SetScale_m(TagWindow *pWinData);
INLINE_PROT void vChip8_Screen_SetWinPosition_m(TagWindow *pWinData);
INLINE_PROT int iChip8_Screen_ExMode_Enable(TagWindow *pWinData);
INLINE_PROT void vChip8_Screen_ExMode_Disable(TagWindow *pWinData);
INLINE_PROT void vChip8_Screen_Clear_m(TagWindow *pWinData);
INLINE_PROT void vChip8_Screen_Draw_m(TagWindow *pWinData);

/**
 * Sound related Functions
 */
INLINE_PROT int iChip8_Sound_Init_m(TagPlaySound *pSoundData);
INLINE_PROT void vChip8_Sound_Close_m(TagPlaySound *pSoundData);
INLINE_PROT void vChip8_Sound_Start_m(TagPlaySound *pSoundData);
INLINE_PROT void vChip8_Sound_Stop_m(TagPlaySound *pSoundData);

/**
 * Key related Functions
 */
INLINE_PROT int iChip8_Keys_Init_m(TagKeyboard *ptagKeyboard);
INLINE_PROT void vChip8_Keys_Close_m(TagKeyboard *ptagKeyboard);

INLINE_PROT int iChip8_Keys_SetKeymap_m(TagKeyboard *ptagKeyBoard);
INLINE_PROT void vChip8_Keys_UpdateState_m(TagKeyboard *ptagKeyboard);

INLINE_PROT void vChip8_Screen_DrawRects_m(TagWindow *pWinData,
                                           SDL_Rect *ptagRects,
                                           unsigned int uiFGCount,
                                           unsigned int uiBGCount);


int iChip8_External_Init_g(TagEmulator *pEmulator)
{
  TRACE_DBG_INFO("Initialise SDL Main...");
  if(SDL_Init(0))
  {
    TRACE_DBG_ERROR_VARG("SDL_Init() failed: %s",SDL_GetError());
    return(-1);
  }
  if(iChip8_RunThread_Init_m(pEmulator))
  {
    TRACE_DBG_ERROR("iChip8_RunThread_Init_g() failed");
    SDL_Quit();
    return(-1);
  }
  return(0);
}

void vChip8_External_Close_g(TagEmulator *pEmulator)
{
  vChip8_RunThread_Close_m(pEmulator);
  SDL_Quit();
}

const char *pcChip8_GetReturnCodeText_m(int iRc)
{
  switch(iRc)
  {
    case RET_CHIP8_OPCODE_OK:
      return("Chip-8 Instruction executed");
    case RET_SUCHIP_OPCODE_OK:
      return("SuperChip Instruction executed");
    case RET_PGM_EXIT:
      return("ROM requested exit");
    case RET_ERR_INVALID_JUMP_ADDRESS:
      return("Address invalid, not in Userspace");
    case RET_ERR_MEM_WOULD_OVERFLOW:
      return("Can't perform operation, memory would overflow");
    case RET_ERR_FONT_OUT_OF_INDEX:
      return("Font index out of Range, use 0-F");
    case RET_ERR_DRAW_OUT_OF_SCREEN:
      return("Draw Coordinates out of Screen");
    case RET_ERR_SCREEN_DRAW:
      return("Draw operation failed");
    case RET_ERR_STACK_MAX_CALLS:
      return("Max Stack depth reached, can't step one more");
    case RET_ERR_STACK_ON_TOP:
      return("Stack is on top, can't jump up");
    case RET_ERR_KEYCODE_INVALID:
      return("Keycode not valid");
    case RET_ERR_INSTRUCTION_UNKNOWN:
      return("Instruction code unknown");
    default:
      return("Unknown Error");
  }
}

int iChip8_RunThread_Init_m(TagEmulator *pEmulator)
{
  TRACE_DBG_INFO("Initialise RunThread...");
  pEmulator->tagThrdEmu.eEmuState=EMU_STATE_INACTIVE;
  pEmulator->tagThrdEmu.eSpeed=EMU_SPEED_1_0X;

  if(!(pEmulator->tagThrdEmu.pThread=SDL_CreateThread(iThread_EmuProcess_m,"EmulatorRunThread",pEmulator)))
  {
    TRACE_DBG_ERROR_VARG("SDL_CreateThread() failed: %s",SDL_GetError());
    return(-1);
  }
  /* Wait for EmulatorRunThread to start successful */
  while(pEmulator->tagThrdEmu.eEmuState!=EMU_STATE_PAUSE)
  {
    if(pEmulator->tagThrdEmu.eEmuState==EMU_STATE_QUIT) /* Thread init failed */
    {
      TRACE_DBG_ERROR("Failed to init threads, waiting for them to end...");
      SDL_WaitThread(pEmulator->tagThrdEmu.pThread,NULL);
      return(-1);
    }
    SDL_Delay(10);
  }
  return(0);
}

void vChip8_RunThread_Close_m(TagEmulator *pEmulator)
{
  if((pEmulator->tagThrdEmu.eEmuState==EMU_STATE_INACTIVE))
    return;

  TRACE_DBG_INFO("Request Runthread to Terminate");
  pEmulator->updateSettings.emu_Quit=1;
  TRACE_DBG_INFO("Waiting for threads to end...");
  SDL_WaitThread(pEmulator->tagThrdEmu.pThread,NULL);
}

INLINE_FCT int iThreadEmu_UpdateSettings_m(TagEmulator *pEmulator,
                                           Uint64 *pExecDelayPerfCounts)
{
  int iRc=EMU_SET_OPTION_OK;
  if(pEmulator->updateSettings.emu_ExecSpeed)
  {
    switch(pEmulator->tagThrdEmu.eSpeed)
    {
      case EMU_SPEED_0_5X:
        *pExecDelayPerfCounts=SDL_GetPerformanceFrequency() / (CHIP8_FREQ_RUN_HZ/2) + 0.5;
        break;
      case EMU_SPEED_1_0X: /* 540 Hz */
        *pExecDelayPerfCounts=SDL_GetPerformanceFrequency() / CHIP8_FREQ_RUN_HZ + 0.5;
        break;
      case EMU_SPEED_1_5X:
        *pExecDelayPerfCounts=SDL_GetPerformanceFrequency() / (CHIP8_FREQ_RUN_HZ*1.5) + 0.5;
        break;
      case EMU_SPEED_2_0X:
        *pExecDelayPerfCounts=SDL_GetPerformanceFrequency() / (CHIP8_FREQ_RUN_HZ*2) + 0.5;
        break;
      default:
        TRACE_DBG_ERROR("Invalid Speed factor selected, changing to default (=1.0)");
        pEmulator->tagThrdEmu.eSpeed=EMU_SPEED_1_0X;
        *pExecDelayPerfCounts=SDL_GetPerformanceFrequency() / CHIP8_FREQ_RUN_HZ + 0.5;
        iRc=EMU_SET_OPTION_CHANGED;
    }
    pEmulator->updateSettings.emu_ExecSpeed=0;
  }
  if(pEmulator->updateSettings.emu_Run)
  {
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_RUN;
    pEmulator->updateSettings.emu_Run=0;
  }
  if(pEmulator->updateSettings.emu_Pause)
  {
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_PAUSE;
    pEmulator->updateSettings.emu_Pause=0;
  }
  if(pEmulator->updateSettings.emu_Quit)
  {
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_QUIT;
    pEmulator->updateSettings.emu_Quit=0;
  }
  /* Check for ESC-Key pressed (Only call once per activation) */
  if((pEmulator->tagKeyboard.ulKeyStates&EMU_KEY_MASK_ESC) &&
     (!(pEmulator->tagKeyboard.ulKeyStatesLast&EMU_KEY_MASK_ESC)))
  {
    /* Reset key indicator here */
    EMU_CALL_USER_EVENT(pEmulator,EMU_EVT_KEYPRESS_ESCAPE);
  }
  /* Update of keymap needed */
  if(pEmulator->updateSettings.keyboard_Keymap)
  {
    iChip8_Keys_SetKeymap_m(&pEmulator->tagKeyboard);
    pEmulator->updateSettings.keyboard_Keymap=0;
  }
  /**
   * check for Screen updates
   */
  if(pEmulator->updateSettings.screen_Scale)
  {
    if(iChip8_Screen_SetScale_m(&pEmulator->tagWindow))
    {
      TRACE_DBG_ERROR("iChip8_Screen_SetScale_m() failed");
      iRc=EMU_SET_OPTION_ERROR;
    }
    else /* Clear screen on resize */
      pEmulator->updateSettings.screen_Clear=1;
    pEmulator->updateSettings.screen_Scale=0;
  }
  if(pEmulator->updateSettings.screen_Pos)
  {
    vChip8_Screen_SetWinPosition_m(&pEmulator->tagWindow);
    pEmulator->updateSettings.screen_Pos=0;
  }
  if(pEmulator->updateSettings.screen_Show)
  {
    vChip8_Screen_Show_m(&pEmulator->tagWindow,1);
    pEmulator->updateSettings.screen_Show=0;
  }
  if(pEmulator->updateSettings.screen_Hide)
  {
    vChip8_Screen_Show_m(&pEmulator->tagWindow,0);
    pEmulator->updateSettings.screen_Hide=0;
  }
  if(pEmulator->updateSettings.screen_Title)
  {
    vChip8_Screen_SetTitle_m(&pEmulator->tagWindow);
    pEmulator->updateSettings.screen_Title=0;
  }
  if((pEmulator->tagWindow.info.visible) && (pEmulator->updateSettings.screen_Clear))
  {
    vChip8_Screen_Clear_m(&pEmulator->tagWindow);
    pEmulator->updateSettings.screen_Clear=0;
  }
  if(pEmulator->updateSettings.screen_ExMode_En)
  {
    if(iChip8_Screen_ExMode_Enable(&pEmulator->tagWindow))
      iRc=EMU_SET_OPTION_ERROR;
    pEmulator->updateSettings.screen_ExMode_En=0;
  }
  if(pEmulator->updateSettings.screen_ExMode_Dis)
  {
    vChip8_Screen_ExMode_Disable(&pEmulator->tagWindow);
    pEmulator->updateSettings.screen_ExMode_Dis=0;
  }
  return(iRc);
}

int iThread_EmuProcess_m(void* data)
{
  int iRc;
  TagEmulator *pEmulator=data;
  Uint32 uiExecutionsCount;
  Uint64 uiPFCLast=0;
  Uint64 uiPFCCurrent;
  Uint64 uiPFCProcessDelay;

  TRACE_DBG_INFO("iThread_EmuProcess_m started, initialising further SDL Libraries...");
  if(iChip8_Screen_Init_m(&pEmulator->tagWindow)) /* Init Screen */
  {
    TRACE_DBG_ERROR("iChip8_Screen_Init_m() failed, exit thread...");
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_QUIT;
    return(-1);
  }
  if(iChip8_Sound_Init_m(&pEmulator->tagPlaySound)) /* Init Sound */
  {
    TRACE_DBG_ERROR("iChip8_Sound_Init_m() failed, exit thread...");
    vChip8_Screen_Close_m(&pEmulator->tagWindow);
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_QUIT;
    return(-1);
  }
  if(iChip8_Keys_Init_m(&pEmulator->tagKeyboard)) /* Init keys */
  {
    TRACE_DBG_ERROR("iChip8_Keys_Init_m() failed, exit thread...");
    vChip8_Sound_Close_m(&pEmulator->tagPlaySound);
    vChip8_Screen_Close_m(&pEmulator->tagWindow);
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_QUIT;
    return(-1);
  }
  if(iChip8_Keys_SetKeymap_m(&pEmulator->tagKeyboard))
  {
    TRACE_DBG_ERROR("Some Errors occured in iChip8_Keys_SetKeymap_m(), mapped to default");
  }
  TRACE_DBG_INFO("iThread_EmuProcess_m(): Initialise done");
  /* Pause this thread at startup */
  pEmulator->updateSettings.emu_ExecSpeed=1;
  pEmulator->updateSettings.screen_Clear=1;
  uiExecutionsCount=0;
  pEmulator->tagThrdEmu.eEmuState=EMU_STATE_PAUSE;
  while(pEmulator->tagThrdEmu.eEmuState != EMU_STATE_QUIT)
  {
    switch(pEmulator->tagThrdEmu.eEmuState)
    {
      case EMU_STATE_RUN:
        uiPFCCurrent=SDL_GetPerformanceCounter();

        if(!uiPFCLast)
          uiPFCLast=uiPFCCurrent;
        /* Check if timer decrement is needed */
        if(uiExecutionsCount%(CHIP8_FREQ_RUN_HZ/CHIP8_FREQ_TIMER_DELAY_HZ) == 0)
        {
          if(REG_TMRDEL)
            --REG_TMRDEL;
          if(REG_TMRSND)
            --REG_TMRSND;
        }
        /* Execute Next OPCode */
        iRc=pEmulator->tagThrdEmu.procFunc(pEmulator);
//      fprintf(stderr,"Executed, uiPFCLast %zu, current: %zu, procdel: %zu\n",uiPFCLast,uiPFCCurrent,uiPFCProcessDelay);   // TODO: remove this
        switch(iRc)
        {
          case RET_CHIP8_OPCODE_OK: /* No error, continue... */
            ++uiExecutionsCount;
            EMU_CALL_USER_EVENT(pEmulator,EMU_EVT_CHIP8_INSTRUCTION_EXECUTED);
            break;
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
          case RET_SUCHIP_OPCODE_OK:
            ++uiExecutionsCount;
            EMU_CALL_USER_EVENT(pEmulator,EMU_EVT_SUCHIP_INSTRUCTION_EXECUTED);
            break;
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */
          case RET_PGM_EXIT:
            TRACE_DBG_INFO("Exit requested by rom, pausing thread...");
            pEmulator->tagThrdEmu.eEmuState=EMU_STATE_PAUSE;
            EMU_CALL_USER_EVENT(pEmulator,EMU_EVT_PROGRAM_EXIT);
            break;
          default: /* Some error occured, stop execution */
            TRACE_DBG_ERROR_VARG("Execute Instruction (0x%.4X) Error: %d: %s\n",pEmulator->tCurrOPCode,iRc,pcChip8_GetReturnCodeText_m(iRc));
            pEmulator->tagThrdEmu.eEmuState=EMU_STATE_ERROR_INSTRUCTION;
            /* Also signal the updatethread to pause */
            if(iRc==RET_ERR_INSTRUCTION_UNKNOWN)
            {
              EMU_CALL_USER_EVENT(pEmulator,EMU_EVT_INSTRUCTION_UNKNOWN);
            }
            else
            {
              EMU_CALL_USER_EVENT(pEmulator,EMU_EVT_INSTRUCTION_ERROR);
            }
        }
        uiPFCLast+=uiPFCProcessDelay;
        if(uiPFCLast > (uiPFCCurrent-uiPFCProcessDelay))
        {
          if(REG_TMRSND)
            vChip8_Sound_Start_m(&pEmulator->tagPlaySound);
          else
            vChip8_Sound_Stop_m(&pEmulator->tagPlaySound);
          vChip8_Screen_Draw_m(&pEmulator->tagWindow);
          SDL_Delay(10);
          uiPFCLast=uiPFCCurrent;
        }
        break;
      case EMU_STATE_PAUSE:
      case EMU_STATE_ERROR_INSTRUCTION:
        /* Wait while thread is paused or an error is indicated */
        uiPFCLast=0;
        SDL_Delay(20);
        break;
      case EMU_STATE_QUIT: /* Handled outside switch */
        break;
      default:
        TRACE_DBG_ERROR_VARG("iThreadFunction_m() invalid Thread Ctrl enum (%d), pausing...",pEmulator->tagThrdEmu.eEmuState);
        pEmulator->tagThrdEmu.eEmuState=EMU_STATE_PAUSE;
    }
    /* Change settings if requested */
    iThreadEmu_UpdateSettings_m(pEmulator,&uiPFCProcessDelay);
    vChip8_Keys_UpdateState_m(&pEmulator->tagKeyboard); /* Update keyboard events */
  }
  TRACE_DBG_INFO("iThreadFunction_m() Quit thread requested...");
  vChip8_Keys_Close_m(&pEmulator->tagKeyboard);
  vChip8_Sound_Close_m(&pEmulator->tagPlaySound);
  vChip8_Screen_Close_m(&pEmulator->tagWindow);
  return(0);
}

INLINE_FCT int iChip8_Screen_Init_m(TagWindow *pWinData)
{
  SDL_DisplayMode tagDPM;
  TRACE_DBG_INFO("Initialise Screen...");

  pWinData->info.exModeOn=0;
  pWinData->info.visible=0;

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
  pWinData->info.iMonitorWidth  =tagDPM.w;
  pWinData->info.iMonitorHeigth =tagDPM.h;
  if(!tagDPM.refresh_rate)
  {
    TRACE_DBG_ERROR("Monitor refresh frequency not defined, falling back to 60Hz...");
    pWinData->info.iMonitorHz=60;
  }
  else
    pWinData->info.iMonitorHz=tagDPM.refresh_rate;

  TRACE_DBG_INFO_VARG("Display width: %dpx, height: %dpx, @%dHz",
                      pWinData->info.iMonitorWidth,
                      pWinData->info.iMonitorHeigth,
                      pWinData->info.iMonitorHz);

  if(!(pWinData->pWindow=SDL_CreateWindow("Chip-8 Emulator",
                                          pWinData->info.iPosX,
                                          pWinData->info.iPosY,
                                          CHIP8_SCREEN_WIDTH*EMU_SCREEN_DEFAULT_SCALE,
                                          CHIP8_SCREEN_HEIGHT*EMU_SCREEN_DEFAULT_SCALE,
                                          SDL_WINDOW_HIDDEN)))
  {
    TRACE_DBG_ERROR_VARG("SDL_CreateWindow() failed: %s",SDL_GetError());
    return(-1);
  }
  if(!(pWinData->pRenderer=SDL_CreateRenderer(pWinData->pWindow,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)))
  {
    TRACE_DBG_ERROR_VARG("SDL_CreateRenderer() failed: %s",SDL_GetError());
    SDL_DestroyWindow(pWinData->pWindow);
    return(-1);
  }
  if(SDL_SetRenderDrawBlendMode(pWinData->pRenderer,SDL_BLENDMODE_NONE))
  {
    TRACE_DBG_ERROR_VARG("SDL_SetRenderDrawBlendMode() failed: %s",SDL_GetError());
    SDL_DestroyRenderer(pWinData->pRenderer);
    SDL_DestroyWindow(pWinData->pWindow);
    return(-1);
  }
  if(!(pWinData->pTexture=SDL_CreateTexture(pWinData->pRenderer,
                                            SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_TARGET,
                                            CHIP8_SCREEN_WIDTH*2,
                                            CHIP8_SCREEN_HEIGHT*2)))
  {
    TRACE_DBG_ERROR_VARG("SDL_CreateTexture() failed: %s",SDL_GetError());
    SDL_DestroyRenderer(pWinData->pRenderer);
    SDL_DestroyWindow(pWinData->pWindow);
    return(-1);
  }
  if(iChip8_Screen_SetScale_m(pWinData))
  {
    SDL_DestroyTexture(pWinData->pTexture);
    SDL_DestroyRenderer(pWinData->pRenderer);
    SDL_DestroyWindow(pWinData->pWindow);
    return(-1);
  }
  return(0);
}

INLINE_FCT void vChip8_Screen_Close_m(TagWindow *pWinData)
{
  TRACE_DBG_INFO("Close Screen...");
  SDL_DestroyTexture(pWinData->pTexture);
  SDL_DestroyRenderer(pWinData->pRenderer);
  SDL_DestroyWindow(pWinData->pWindow);
  SDL_QuitSubSystem(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
}

INLINE_FCT void vChip8_Screen_SetTitle_m(TagWindow *pWinData)
{
  TRACE_DBG_INFO_VARG("Set Emulator Window Title to \"%s\"",pWinData->info.caTitle);
  SDL_SetWindowTitle(pWinData->pWindow,pWinData->info.caTitle);
}

INLINE_FCT void vChip8_Screen_Show_m(TagWindow *pWinData,
                                     int iShow)
{
  if(iShow)
  {
    if(pWinData->info.visible) /* If already visible, return */
      return;
    SDL_ShowWindow(pWinData->pWindow);
    SDL_RaiseWindow(pWinData->pWindow);
    pWinData->info.visible=1;
  }
  else
  {
    if(!pWinData->info.visible) /* If already hidden, return */
      return;
    /* needed to handle correct key (don't keep esc pressed virtually) */
    SDL_MinimizeWindow(pWinData->pWindow);
    SDL_RestoreWindow(pWinData->pWindow);
    SDL_HideWindow(pWinData->pWindow);
    pWinData->info.visible=0;
  }
}

INLINE_FCT int iChip8_Screen_SetScale_m(TagWindow *pWinData)
{
  int iRc=0;
  TRACE_DBG_INFO_VARG("Set Window scale to Factor %d",pWinData->info.scale);

  /* Check window still fits on current display */
  if((CHIP8_SCREEN_WIDTH*pWinData->info.scale>(unsigned)pWinData->info.iMonitorWidth) ||
     (CHIP8_SCREEN_HEIGHT*pWinData->info.scale>(unsigned)pWinData->info.iMonitorHeigth))
  {
    TRACE_DBG_ERROR_VARG("Can't Set new Scale, too big, falling back to default(=%u)",EMU_SCREEN_DEFAULT_SCALE);
    pWinData->info.scale=EMU_SCREEN_DEFAULT_SCALE;
    iRc=1;
  }
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
  if(pWinData->info.scale<2)
  {
    TRACE_DBG_ERROR_VARG("Can't Set scale lower than 2 (is: %u) if Superchip is enabled, setting to Minimum (=2)",pWinData->info.scale);
    pWinData->info.scale=2;
    iRc=2;
  }
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */
  SDL_SetWindowSize(pWinData->pWindow,
                    CHIP8_SCREEN_WIDTH*pWinData->info.scale,
                    CHIP8_SCREEN_HEIGHT*pWinData->info.scale);
  SDL_RenderSetScale(pWinData->pRenderer,(float)pWinData->info.scale/2.0,(float)pWinData->info.scale/2.0);
  return(iRc);
}

INLINE_FCT void vChip8_Screen_SetWinPosition_m(TagWindow *pWinData)
{
  SDL_SetWindowPosition(pWinData->pWindow,pWinData->info.iPosX,pWinData->info.iPosY);
}

INLINE_FCT void vChip8_Screen_Clear_m(TagWindow *pWinData)
{
  /* clear window contents, redraw... (window should be visible while calling this) */
  memset(pWinData->taLastScreen,0,sizeof(pWinData->taLastScreen));
  memset(pWinData->taScreenBuffer,0,sizeof(pWinData->taScreenBuffer));
  SDL_SetRenderTarget(pWinData->pRenderer,pWinData->pTexture);
  SDL_SetRenderDrawColor(pWinData->pRenderer,0,0,0,SDL_ALPHA_OPAQUE);
  SDL_RenderClear(pWinData->pRenderer);
  SDL_SetRenderTarget(pWinData->pRenderer,NULL);
  SDL_RenderCopy(pWinData->pRenderer,pWinData->pTexture,NULL,NULL);
  SDL_RenderPresent(pWinData->pRenderer);
}

INLINE_FCT int iChip8_Screen_ExMode_Enable(TagWindow *pWinData)
{
  TRACE_DBG_INFO("Enabling Extended screen mode");
  /* Check if already in extended mode */
  if(pWinData->info.exModeOn)
  {
    TRACE_DBG_INFO("iChip8_Screen_ExMode_Enable() Already in Extended mode");
    return(0);
  }
  if(pWinData->info.scale<2)
  {
    TRACE_DBG_ERROR_VARG("iChip8_Screen_ExMode_Enable(): Can't set Extended mode, scale is too small (%u), should at least be 2.",
                         pWinData->info.scale);
    return(-1);
  }
  pWinData->info.exModeOn=1;
  return(0);
}

INLINE_FCT void vChip8_Screen_ExMode_Disable(TagWindow *pWinData)
{
  pWinData->info.exModeOn=0;
}

INLINE_FCT void vChip8_Screen_Draw_m(TagWindow *pWinData)
{
  SDL_Rect tagRects[SCREEN_RECTS_MAX_DRAW]; /* Max possible */
  unsigned int uiIndex;
  unsigned int uiIndexB;
  unsigned int uiFGRectsCount=0;
  unsigned int uiBGCountRects=SCREEN_RECTS_BLACK_OFFSET;
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
  const unsigned int uiScreenWidth=(pWinData->info.exModeOn)?SUCHIP_SCREEN_WIDTH:CHIP8_SCREEN_WIDTH;
  const unsigned char ucScale=(pWinData->info.exModeOn)?1:2;
#else
  const unsigned int uiScreenWidth=CHIP8_SCREEN_WIDTH;
  const unsigned char ucScale=2;
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */

  /* Clear Render first to bg color */
  SDL_SetRenderDrawColor(pWinData->pRenderer,0,0,0,SDL_ALPHA_OPAQUE);
  SDL_RenderClear(pWinData->pRenderer);
  /* Now, set renderer to texture, which contains the current screen buffer */
  SDL_SetRenderTarget(pWinData->pRenderer,pWinData->pTexture);

  for(uiIndex=0;uiIndex<((pWinData->info.exModeOn)?SCREEN_SIZE_EXMODE:SCREEN_SIZE_NORMAL);++uiIndex)
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
  SDL_SetRenderTarget(pWinData->pRenderer,NULL);
  SDL_RenderCopy(pWinData->pRenderer,pWinData->pTexture,NULL,NULL);
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

INLINE_FCT int iChip8_Sound_Init_m(TagPlaySound *pSoundData)
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

INLINE_FCT void vChip8_Sound_Close_m(TagPlaySound *pSoundData)
{
  TRACE_DBG_INFO("Close Sound...");
  SDL_ClearQueuedAudio(pSoundData->tDevID);
  SDL_CloseAudioDevice(pSoundData->tDevID);
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

INLINE_FCT void vChip8_Sound_Start_m(TagPlaySound *pSoundData)
{
  if(pSoundData->iSoundPlaying)
    return;
  while(SDL_GetQueuedAudioSize(pSoundData->tDevID)<EMU_PLAYSOUND_SAMPLE_RATE_HZ*5) /* Prepare data for 5s continously playing */
    SDL_QueueAudio(pSoundData->tDevID,pSoundData->caSoundBuffer,pSoundData->uiUsedSoundSize);
  SDL_PauseAudioDevice(pSoundData->tDevID,0); /* Start Playback */
  pSoundData->iSoundPlaying=1;
}

INLINE_FCT void vChip8_Sound_Stop_m(TagPlaySound *pSoundData)
{
  if(!pSoundData->iSoundPlaying)
    return;
  SDL_PauseAudioDevice(pSoundData->tDevID,1); /* pause Playback */
  pSoundData->iSoundPlaying=0;
}

INLINE_FCT int iChip8_Keys_Init_m(TagKeyboard *ptagKeyboard)
{
  TRACE_DBG_INFO("Initialise Keys...");
  /**
   *  Disable unneeded events
   */
  /* Disable Mouse events */
  SDL_EventState(SDL_MOUSEMOTION,      SDL_IGNORE);
  SDL_EventState(SDL_MOUSEBUTTONDOWN,  SDL_IGNORE);
  SDL_EventState(SDL_MOUSEBUTTONUP,    SDL_IGNORE);
  SDL_EventState(SDL_MOUSEWHEEL,       SDL_IGNORE);

  /* Disable Drag & drop stuff */
  SDL_EventState(SDL_DROPFILE,         SDL_IGNORE);
  SDL_EventState(SDL_DROPTEXT,         SDL_IGNORE);
  SDL_EventState(SDL_DROPBEGIN,        SDL_IGNORE);
  SDL_EventState(SDL_DROPCOMPLETE,     SDL_IGNORE);
  ptagKeyboard->ulKeyStates=0;
  ptagKeyboard->ulKeyStatesLast=0;
  return(0); /* further init needed for sdl2 */
}

INLINE_FCT void vChip8_Keys_Close_m(TagKeyboard *ptagKeyboard)
{
  (void)ptagKeyboard;
  TRACE_DBG_INFO("Close Keys..."); /* No close needed for sdl2 */
}

INLINE_FCT int iChip8_Keys_SetKeymap_m(TagKeyboard *ptagKeyBoard)
{
  unsigned int iRc=0;
  unsigned char ucIndex;
  for(ucIndex=0;ucIndex<sizeof(ptagKeyBoard->ausKeyMap)/sizeof(unsigned short);++ucIndex)
  {
    switch(ptagKeyBoard->ucaUsrKeyMap[ucIndex])
    {
      case EMU_KEY_0:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_0;     break;
      case EMU_KEY_1:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_1;     break;
      case EMU_KEY_2:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_2;     break;
      case EMU_KEY_3:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_3;     break;
      case EMU_KEY_4:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_4;     break;
      case EMU_KEY_5:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_5;     break;
      case EMU_KEY_6:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_6;     break;
      case EMU_KEY_7:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_7;     break;
      case EMU_KEY_8:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_8;     break;
      case EMU_KEY_9:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_9;     break;
      case EMU_KEY_A:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_A;     break;
      case EMU_KEY_B:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_B;     break;
      case EMU_KEY_C:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_C;     break;
      case EMU_KEY_D:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_D;     break;
      case EMU_KEY_E:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_E;     break;
      case EMU_KEY_F:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_F;     break;
      case EMU_KEY_G:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_G;     break;
      case EMU_KEY_H:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_H;     break;
      case EMU_KEY_I:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_I;     break;
      case EMU_KEY_J:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_J;     break;
      case EMU_KEY_K:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_K;     break;
      case EMU_KEY_L:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_L;     break;
      case EMU_KEY_M:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_M;     break;
      case EMU_KEY_N:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_N;     break;
      case EMU_KEY_O:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_O;     break;
      case EMU_KEY_P:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_P;     break;
      case EMU_KEY_Q:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_Q;     break;
      case EMU_KEY_R:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_R;     break;
      case EMU_KEY_S:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_S;     break;
      case EMU_KEY_T:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_T;     break;
      case EMU_KEY_U:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_U;     break;
      case EMU_KEY_V:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_V;     break;
      case EMU_KEY_W:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_W;     break;
      case EMU_KEY_X:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_X;     break;
      case EMU_KEY_Y:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_Y;     break;
      case EMU_KEY_Z:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_Z;     break;
      case EMU_KEY_UP:    ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_UP;    break;
      case EMU_KEY_DOWN:  ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_DOWN;  break;
      case EMU_KEY_LEFT:  ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_LEFT;  break;
      case EMU_KEY_RIGHT: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_RIGHT; break;
      default:
        TRACE_DBG_ERROR_VARG("chip8_Keys_SetKeymap_m(): Key [%X] unknwon keycode (0x%X), using default...",
                             ucIndex,
                             ptagKeyBoard->ucaUsrKeyMap[ucIndex]);
        iRc=1;
      /* Fall through */
      case EMU_KEY_DEFAULT:
        switch(ucIndex)
        {
          case 0x0: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_0; break;
          case 0x1: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_1; break;
          case 0x2: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_2; break;
          case 0x3: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_3; break;
          case 0x4: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_4; break;
          case 0x5: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_5; break;
          case 0x6: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_6; break;
          case 0x7: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_7; break;
          case 0x8: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_8; break;
          case 0x9: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_9; break;
          case 0xA: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_A; break;
          case 0xB: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_B; break;
          case 0xC: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_C; break;
          case 0xD: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_D; break;
          case 0xE: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_E; break;
          case 0xF: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_F; break;
        }
        break;
    }
  }
  return(iRc);
}

INLINE_FCT void vChip8_Keys_UpdateState_m(TagKeyboard *ptagKeyboard)
{
  const Uint8 *puiTmp;

  SDL_PumpEvents();
  puiTmp=SDL_GetKeyboardState(NULL);
  /* Save last keyboard state */
  ptagKeyboard->ulKeyStatesLast=ptagKeyboard->ulKeyStates;

  ptagKeyboard->ulKeyStates=((puiTmp[ptagKeyboard->ausKeyMap[0x0]])?EMU_KEY_MASK_0:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x1]])?EMU_KEY_MASK_1:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x2]])?EMU_KEY_MASK_2:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x3]])?EMU_KEY_MASK_3:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x4]])?EMU_KEY_MASK_4:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x5]])?EMU_KEY_MASK_5:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x6]])?EMU_KEY_MASK_6:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x7]])?EMU_KEY_MASK_7:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x8]])?EMU_KEY_MASK_8:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x9]])?EMU_KEY_MASK_9:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0xA]])?EMU_KEY_MASK_A:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0xB]])?EMU_KEY_MASK_B:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0xC]])?EMU_KEY_MASK_C:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0xD]])?EMU_KEY_MASK_D:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0xE]])?EMU_KEY_MASK_E:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0xF]])?EMU_KEY_MASK_F:0|
                             (puiTmp[SDL_SCANCODE_ESCAPE])         ?EMU_KEY_MASK_ESC:0);
}

