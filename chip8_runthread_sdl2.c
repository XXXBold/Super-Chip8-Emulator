#include <stdio.h>
#include <stdlib.h>

#include "chip8_runthread.h"
#include "chip8_global.h"
#include "chip8_screen.h"
#include "chip8_sound.h"
#include "chip8_keys.h"

enum
{
  EMU_SET_OPTION_OK,
  EMU_SET_OPTION_CHANGED,
  EMU_SET_OPTION_ERROR
};

#define EMU_CALL_USER_EVENT(emu,evt) (((emu->tagThrdEmu.usrCBFunc) && (emu->tagThrdEmu.usrCBEvents&evt))?emu->tagThrdEmu.usrCBFunc(evt,emu->tCurrOPCode):0)

int iThread_EmuProcess_m(void* data);
int iThread_SDLUpdate_m(void* data);
void vChip8_RunThread_LockScreen_m(TagEmulator *pEmulator,
                                   int iLock);
INLINE_PROT int iThreadEmu_UpdateSettings_m(TagEmulator *pSettings,
                                            Uint64 *pExecDelayPerfCounts);


void vChip8_SleepMicroSecs_m(long lSleepTime)
{
#ifdef _WIN32
  //TODO implement short wait for windows
#else
  struct timespec tagDelay;
  tagDelay.tv_sec=0;
  tagDelay.tv_nsec=lSleepTime*1000;
  nanosleep(&tagDelay,NULL);
#endif /* _Win32 */
}

const char *pcChip8_GetErrorText(int iErrorcode)
{
  switch(iErrorcode)
  {
    case RET_CHIP8_OPCODE_OK:
      return("No Error");
    case RET_ERR_INVALID_JUMP_ADDRESS:
      return("Address invalid, not in Userspace");
    case RET_ERR_MEM_WOULD_OVERFLOW:
      return("Can't perform operation, memory would overflow");
    case RET_ERR_FONT_OUT_OF_INDEX:
      return("Font index out of Range, use 0-F");
    case RET_ERR_SCREEN_DRAWPOS_INVALID:
      return("Draw X Coordinate overflow");
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

int iChip8_RunThread_Init_g(TagEmulator *pEmulator)
{
  TRACE_DBG_INFO("Initialise RunThread...");
  pEmulator->tagThrdEmu.eEmuState=EMU_STATE_INACTIVE;
  pEmulator->tagThrdEmu.eSpeed=EMU_SPEED_1_0X;

  pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_INACTIVE;
  pEmulator->tagWindow.lockScreen=vChip8_RunThread_LockScreen_m;

  if(!(pEmulator->pMutex=SDL_CreateMutex()))
  {
    TRACE_DBG_ERROR_VARG("SDL_CreateMutex() failed: %s",SDL_GetError());
    return(-1);
  }
  if(!(pEmulator->tagThrdEmu.pThread=SDL_CreateThread(iThread_EmuProcess_m,"EmulatorRunThread",pEmulator)))
  {
    TRACE_DBG_ERROR_VARG("SDL_CreateThread() failed: %s",SDL_GetError());
    SDL_DestroyMutex(pEmulator->pMutex);
    return(-1);
  }
  if(!(pEmulator->tagThrdRenderer.pThread=SDL_CreateThread(iThread_SDLUpdate_m,"SDLUpdateThread",pEmulator)))
  {
    TRACE_DBG_ERROR_VARG("SDL_CreateThread() failed: %s",SDL_GetError());
    SDL_DestroyMutex(pEmulator->pMutex);
    return(-1);
  }
  /* Wait for EmulatorRunThread to start successful */
  while(pEmulator->tagThrdEmu.eEmuState!=EMU_STATE_PAUSE)
  {
    if(pEmulator->tagThrdEmu.eEmuState==EMU_STATE_QUIT) /* Thread init failed */
    {
      TRACE_DBG_ERROR("Failed to init threads, waiting for them to end...");
      SDL_WaitThread(pEmulator->tagThrdEmu.pThread,NULL);
      SDL_WaitThread(pEmulator->tagThrdRenderer.pThread,NULL);
      SDL_DestroyMutex(pEmulator->pMutex);
      return(-1);
    }
    SDL_Delay(10);
  }
  return(0);
}

void vChip8_RunThread_Close_g(TagEmulator *pEmulator)
{
  if((pEmulator->tagThrdEmu.eEmuState==EMU_STATE_INACTIVE))
    return;

  TRACE_DBG_INFO("Request Runthread to Terminate");
  pEmulator->updateSettings.threadEmu_Quit=1;
  TRACE_DBG_INFO("Waiting for threads to end...");
  SDL_WaitThread(pEmulator->tagThrdEmu.pThread,NULL);
  SDL_WaitThread(pEmulator->tagThrdRenderer.pThread,NULL);
  SDL_DestroyMutex(pEmulator->pMutex);
}

void vChip8_RunThread_LockScreen_m(TagEmulator *pEmulator,
                                   int iLock)
{
  if(iLock)
    SDL_LockMutex(pEmulator->pMutex);
  else
    SDL_UnlockMutex(pEmulator->pMutex);
}

INLINE_FCT int iThreadEmu_UpdateSettings_m(TagEmulator *pEmulator,
                                           Uint64 *pExecDelayPerfCounts)
{
  int iRc=EMU_SET_OPTION_OK;
  SDL_LockMutex(pEmulator->pMutex);
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

  if(pEmulator->updateSettings.threadEmu_Run)
  {
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_RUN;
    /* Flag update thread to run */
    pEmulator->updateSettings.threadUpdate_Run=1;
    pEmulator->updateSettings.threadEmu_Run=0;
  }
  if(pEmulator->updateSettings.threadEmu_Pause)
  {
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_PAUSE;
    /* Flag update thread to pause */
    pEmulator->updateSettings.threadUpdate_Pause=1;
    pEmulator->updateSettings.threadEmu_Pause=0;
  }
  if(pEmulator->updateSettings.threadEmu_Quit)
  {
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_QUIT;
    /* Flag update thread to quit */
    pEmulator->updateSettings.threadUpdate_Quit=1;
    pEmulator->updateSettings.threadEmu_Quit=0;
  }
  SDL_UnlockMutex(pEmulator->pMutex);
  return(iRc);
}

int iThread_EmuProcess_m(void* data)
{
  int iRc;
  TagEmulator *pEmulator=data;
  Uint32 uiExecutionsCount;
  Uint64 uiLastExecTime=0;
  Uint64 uiPFCCurrent;
  Uint64 uiPFCProcessDelay;

  TRACE_DBG_INFO("iThread_EmuProcess_m(): started, waitng for Updatethread to init...");
  uiExecutionsCount=0;
  pEmulator->updateSettings.emu_ExecSpeed=1;
  iThreadEmu_UpdateSettings_m(pEmulator,&uiPFCProcessDelay);
  for(iRc=0;iRc<10;++iRc) /* Wait for SDL_Updatethread to init successful, else quit */
  {
    if(pEmulator->tagThrdRenderer.eThreadState==THREAD_UPDATE_STATE_PAUSED)
      break;
    SDL_Delay(100);
  }
  if(pEmulator->tagThrdRenderer.eThreadState!=THREAD_UPDATE_STATE_PAUSED)
  {
    TRACE_DBG_ERROR("iThread_EmuProcess_m(): timeout waiting for Updatethread to init, aborting...");
    pEmulator->updateSettings.threadUpdate_Quit=1;
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_QUIT;
    return(-1);
  }
  TRACE_DBG_INFO("iThread_EmuProcess_m(): Initialise done");
  /* Pause this thread at startup */
  pEmulator->tagThrdEmu.eEmuState=EMU_STATE_PAUSE;
  while(1)
  {
    switch(pEmulator->tagThrdEmu.eEmuState)
    {
      case EMU_STATE_RUN:
        uiPFCCurrent=SDL_GetPerformanceCounter();
        if(uiPFCCurrent-uiLastExecTime<uiPFCProcessDelay)
        {
          break; /* No processing needed yet */
        }
        /* Check if timer decrement is needed */
        if(uiExecutionsCount%(CHIP8_FREQ_RUN_HZ/CHIP8_FREQ_TIMER_DELAY_HZ)==0)
        {
          if(REG_TMRDEL)
            --REG_TMRDEL;
          if(REG_TMRSND)
            --REG_TMRSND;
        }
        iRc=pEmulator->tagThrdEmu.procFunc(pEmulator);
/*      printf("DELAY WAS: %I64u\n",uiPFCCurrent-uiLastExecTime); */ //TODO: Sleep/delay somehow without loosing precision
        uiLastExecTime=uiPFCCurrent;
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
            pEmulator->updateSettings.threadEmu_Pause=1;
            EMU_CALL_USER_EVENT(pEmulator,EMU_EVT_PROGRAM_EXIT);
            break;
          default: /* Some error occured, stop execution */
            TRACE_DBG_ERROR_VARG("Execute Instruction (0x%.4X) Error: %d: %s\n",pEmulator->tCurrOPCode,iRc,pcChip8_GetErrorText(iRc));
            pEmulator->tagThrdEmu.eEmuState=EMU_STATE_ERROR_INSTRUCTION;
            /* Also signal the updatethread to pause */
            pEmulator->updateSettings.threadUpdate_Pause=1;
            if(iRc==RET_ERR_INSTRUCTION_UNKNOWN)
            {
              EMU_CALL_USER_EVENT(pEmulator,EMU_EVT_INSTRUCTION_UNKNOWN);
            }
            else
            {
              EMU_CALL_USER_EVENT(pEmulator,EMU_EVT_INSTRUCTION_ERROR);
            }
        }
        break;
      case EMU_STATE_PAUSE:
      case EMU_STATE_ERROR_INSTRUCTION:
        /* Wait while thread is paused or an error is indicated */
        SDL_Delay(5);
        break;
      case EMU_STATE_QUIT: /* Handled outside switch */
        break;
      default:
        TRACE_DBG_ERROR_VARG("iThreadFunction_m() invalid Thread Ctrl enum (%d), pausing...",pEmulator->tagThrdEmu.eEmuState);
        pEmulator->tagThrdEmu.eEmuState=EMU_STATE_PAUSE;
        pEmulator->updateSettings.threadUpdate_Pause=1;

    }
    /* Quit Loop here */
    if(pEmulator->tagThrdEmu.eEmuState==EMU_STATE_QUIT)
    {
      TRACE_DBG_INFO("iThreadFunction_m() Quit thread requested...");
      break;
    }
    /* Change settings if requested */
    iThreadEmu_UpdateSettings_m(pEmulator,&uiPFCProcessDelay);
  }
  return(0);
}

int iThreadSDLUpdate_UpdateSettings_m(TagEmulator *pEmulator)
{
  int iRc=EMU_SET_OPTION_OK;
  SDL_LockMutex(pEmulator->pMutex);

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
    iChip8_Keys_SetKeymap_g(&pEmulator->tagKeyboard);
    pEmulator->updateSettings.keyboard_Keymap=0;
  }

  /**
   * Check for Thread status updates
   */
  /* Set run */
  if(pEmulator->updateSettings.threadUpdate_Run)
  {
    pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_ACTIVE;
    pEmulator->updateSettings.threadUpdate_Run=0;
  }
  /* Set pause */
  if(pEmulator->updateSettings.threadUpdate_Pause)
  {
    vChip8_Sound_Stop_g(&pEmulator->tagPlaySound);
    pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_PAUSED;
    pEmulator->updateSettings.threadUpdate_Pause=0;
  }
  /* quit thread */
  if(pEmulator->updateSettings.threadUpdate_Quit)
  {
    pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_QUIT;
    pEmulator->updateSettings.threadUpdate_Quit=0;
  }

  /**
   * check for Screen updates
   */
  if(pEmulator->updateSettings.screen_Scale)
  {
    if(iChip8_Screen_SetScale_g(&pEmulator->tagWindow))
    {
      TRACE_DBG_ERROR("iChip8_Screen_SetScale_g() failed");
      iRc=EMU_SET_OPTION_ERROR;
    }
    else /* Clear screen on resize */
      pEmulator->updateSettings.screen_Clear=1;
    pEmulator->updateSettings.screen_Scale=0;
  }
  if(pEmulator->updateSettings.screen_Pos)
  {
    vChip8_Screen_SetWinPosition_g(&pEmulator->tagWindow);
    pEmulator->updateSettings.screen_Pos=0;
  }
  if(pEmulator->updateSettings.screen_Show)
  {
    vChip8_Screen_Show_g(&pEmulator->tagWindow,1);
    pEmulator->updateSettings.screen_Show=0;
  }
  if(pEmulator->updateSettings.screen_Hide)
  {
    vChip8_Screen_Show_g(&pEmulator->tagWindow,0);
    pEmulator->updateSettings.screen_Hide=0;
  }
  if((pEmulator->tagWindow.data.visible) && (pEmulator->updateSettings.screen_Clear))
  {
    vChip8_Screen_Clear_g(&pEmulator->tagWindow);
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
  SDL_UnlockMutex(pEmulator->pMutex);
  return(iRc);
}

int iThread_SDLUpdate_m(void* data)
{
  TagEmulator *pEmulator=data;
  Uint64 uiLastExecTime=0;
  Uint64 uiPFCDelay;
  Uint64 uiPFCCurrent;

  TRACE_DBG_INFO("iThread_SDLUpdate_m started, initialising further SDL Libraries...");
  if(iChip8_Screen_Init_g(&pEmulator->tagWindow)) /* Init Screen */
  {
    TRACE_DBG_ERROR("iChip8_Screen_Init_g() failed, exit thread...");
    pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_ERROR;
    return(-1);
  }
  if(iChip8_Sound_Init_g(&pEmulator->tagPlaySound)) /* Init Sound */
  {
    TRACE_DBG_ERROR("iChip8_Sound_Init_g() failed, exit thread...");
    vChip8_Screen_Close_g(&pEmulator->tagWindow);
    pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_ERROR;
    return(-1);
  }
  if(iChip8_Keys_Init_g(&pEmulator->tagKeyboard)) /* Init keys */
  {
    TRACE_DBG_ERROR("iChip8_Keys_Init_g() failed, exit thread...");
    vChip8_Sound_Close_g(&pEmulator->tagPlaySound);
    vChip8_Screen_Close_g(&pEmulator->tagWindow);
    pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_ERROR;
    return(-1);
  }
  if(iChip8_Keys_SetKeymap_g(&pEmulator->tagKeyboard))
  {
    TRACE_DBG_ERROR("Some Errors occured in iChip8_Keys_SetKeymap_g()");
  }
  /* Calculate Delay with Monitor frequency, to make it work like VSync */
  uiPFCDelay=(SDL_GetPerformanceFrequency()/pEmulator->tagWindow.iMonitorHz)+0.5;

  pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_PAUSED;
  TRACE_DBG_INFO("iThread_SDLUpdate_m: Init successful");
  while(1)
  {
    switch(pEmulator->tagThrdRenderer.eThreadState)
    {
      case THREAD_UPDATE_STATE_ACTIVE:
        uiPFCCurrent=SDL_GetPerformanceCounter();
        if(uiPFCCurrent-uiLastExecTime<uiPFCDelay)
        {
          if(REG_TMRSND)
            vChip8_Sound_Start_g(&pEmulator->tagPlaySound);
          else
            vChip8_Sound_Stop_g(&pEmulator->tagPlaySound);
          SDL_Delay(2); /* Delay for CPU usage reduction */
        }
        uiLastExecTime=uiPFCCurrent;
        SDL_LockMutex(pEmulator->pMutex);
        vChip8_Screen_Draw_g(&pEmulator->tagWindow);
        SDL_UnlockMutex(pEmulator->pMutex);
        break;
      case THREAD_UPDATE_STATE_PAUSED:
        SDL_Delay(20); /* Relax while paused */
        break;
      case THREAD_UPDATE_STATE_ERROR:
        TRACE_DBG_ERROR("iThread_SDLUpdate_m(): Error, Pausing...");
        pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_PAUSED;
        break;
      case THREAD_UPDATE_STATE_QUIT: /* Handle at bottom of loop */
        break;
      default:
        TRACE_DBG_ERROR("iThread_SDLUpdate_m(): Unknown Thread State, Pausing...");
        pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_PAUSED;
    }
    if(pEmulator->tagThrdRenderer.eThreadState==THREAD_UPDATE_STATE_QUIT)
    {
      TRACE_DBG_INFO("iThread_SDLUpdate_m(): requested exit, quit...");
      break;
    }
    /* Change settings if requested */
    vChip8_Keys_UpdateState_g(&pEmulator->tagKeyboard); /* Still update keyboard events */
    iThreadSDLUpdate_UpdateSettings_m(pEmulator);
  }
  vChip8_Keys_Close_g(&pEmulator->tagKeyboard);
  vChip8_Sound_Close_g(&pEmulator->tagPlaySound);
  vChip8_Screen_Close_g(&pEmulator->tagWindow);
  pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_INACTIVE;
  return(0);
}
