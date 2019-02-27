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
    case ERR_NONE:
      return("No Error");
    case ERR_INVALID_JUMP_ADDRESS:
      return("Address invalid, not in Userspace");
    case ERR_MEM_WOULD_OVERFLOW:
      return("Can't perform operation, memory would overflow");
    case ERR_FONT_OUT_OF_INDEX:
      return("Font index out of Range, use 0-F");
    case ERR_SCREEN_X_OVERFLOW:
      return("Draw X Coordinate overflow");
    case ERR_SCREEN_DRAW:
      return("Draw operation failed");
    case ERR_STACK_MAX_CALLS:
      return("Max Stack depth reached, can't step one more");
    case ERR_STACK_ON_TOP:
      return("Stack is on top, can't jump up");
    case ERR_KEYCODE_INVALID:
      return("Keycode not valid");
    case ERR_INSTRUCTION_UNKNOWN:
      return("Instruction code unknown");
    default:
      return("Unknown Error");
  }
}

int iChip8_RunThread_Init_g(TagEmulator *pEmulator)
{
  TRACE_DBG_INFO("Initialise RunThread...");
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
  EMU_UPD_SET_FLAG(pEmulator,EMU_UPDATE_SETTING_RUNTHREAD_QUIT);
  TRACE_DBG_INFO("Waiting for threads to end...");
  SDL_WaitThread(pEmulator->tagThrdEmu.pThread,NULL);
  SDL_WaitThread(pEmulator->tagThrdRenderer.pThread,NULL);
  SDL_DestroyMutex(pEmulator->pMutex);
}

INLINE_FCT int iThreadEmu_UpdateSettings_m(TagEmulator *pEmulator,
                                           Uint64 *pExecDelayPerfCounts)
{
  int iRc=EMU_SET_OPTION_OK;
  SDL_LockMutex(pEmulator->pMutex);
  if(EMU_UPD_CHK_FLAG(pEmulator,EMU_UPDATE_SETTING_EMULATION_SPEED))
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
    EMU_UPD_RST_FLAG(pEmulator,EMU_UPDATE_SETTING_EMULATION_SPEED);
  }

  if(EMU_UPD_CHK_FLAG(pEmulator,EMU_UPDATE_SETTING_RUNTHREAD_RUN))
  {
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_RUN;
    /* Flag update thread to run */
    EMU_UPD_SET_FLAG(pEmulator,EMU_UPDATE_SETTING_UPDATETHREAD_RUN);
    EMU_UPD_RST_FLAG(pEmulator,EMU_UPDATE_SETTING_RUNTHREAD_RUN);
  }
  if(EMU_UPD_CHK_FLAG(pEmulator,EMU_UPDATE_SETTING_RUNTHREAD_PAUSE))
  {
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_PAUSE;
    /* Flag update thread to pause */
    EMU_UPD_SET_FLAG(pEmulator,EMU_UPDATE_SETTING_UPDATETHREAD_PAUSE);
    EMU_UPD_RST_FLAG(pEmulator,EMU_UPDATE_SETTING_RUNTHREAD_PAUSE);
  }
  if(EMU_UPD_CHK_FLAG(pEmulator,EMU_UPDATE_SETTING_RUNTHREAD_QUIT))
  {
    pEmulator->tagThrdEmu.eEmuState=EMU_STATE_QUIT;
    /* Flag update thread to quit */
    EMU_UPD_SET_FLAG(pEmulator,EMU_UPDATE_SETTING_UPDATETHREAD_QUIT);
    EMU_UPD_RST_FLAG(pEmulator,EMU_UPDATE_SETTING_RUNTHREAD_QUIT);
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
  EMU_UPD_SET_FLAG(pEmulator,EMU_UPDATE_SETTING_EMULATION_SPEED);
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
    EMU_UPD_SET_FLAG(pEmulator,EMU_UPDATE_SETTING_UPDATETHREAD_QUIT);
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
          case ERR_NONE: /* No error, continue... */
            ++uiExecutionsCount;
            EMU_CALL_USER_EVENT(pEmulator,EMU_EVT_INSTRUCTION_EXECUTED);
            break;
          default: /* Some error occured, stop execution */
            TRACE_DBG_ERROR_VARG("Execute Instruction (0x%.4X) Error: %d: %s\n",pEmulator->tCurrOPCode,iRc,pcChip8_GetErrorText(iRc));
            pEmulator->tagThrdEmu.eEmuState=EMU_STATE_ERROR_INSTRUCTION;
            /* Also signal the updatethread to pause */
            EMU_UPD_SET_FLAG(pEmulator,EMU_UPDATE_SETTING_UPDATETHREAD_PAUSE);
            if(iRc==ERR_INSTRUCTION_UNKNOWN)
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
        TRACE_DBG_ERROR_VARG("iThreadFunction_m() unknown Thread Ctrl enum (%d), pausing...",pEmulator->tagThrdEmu.eEmuState);
        pEmulator->tagThrdEmu.eEmuState=EMU_STATE_PAUSE;
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
  if((pEmulator->tagKeyboard.ulKeyStates&EMU_KEY_MASK_ESC) &&
     (!(pEmulator->tagKeyboard.ulKeyStatesLast&EMU_KEY_MASK_ESC)))
  {
    /* Reset key indicator here */
    EMU_CALL_USER_EVENT(pEmulator,EMU_EVT_KEYPRESS_ESCAPE);
  }

  /* Set run */
  if(EMU_UPD_CHK_FLAG(pEmulator,EMU_UPDATE_SETTING_UPDATETHREAD_RUN))
  {
    pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_ACTIVE;
    EMU_UPD_RST_FLAG(pEmulator,EMU_UPDATE_SETTING_UPDATETHREAD_RUN);
  }
  /* Set pause */
  if(EMU_UPD_CHK_FLAG(pEmulator,EMU_UPDATE_SETTING_UPDATETHREAD_PAUSE))
  {
    vChip8_Sound_Stop_g(&pEmulator->tagPlaySound);
    pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_PAUSED;
    EMU_UPD_RST_FLAG(pEmulator,EMU_UPDATE_SETTING_UPDATETHREAD_PAUSE);
  }
  /* quit thread */
  if(EMU_UPD_CHK_FLAG(pEmulator,EMU_UPDATE_SETTING_UPDATETHREAD_QUIT))
  {
    pEmulator->tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_QUIT;
    EMU_UPD_RST_FLAG(pEmulator,EMU_UPDATE_SETTING_UPDATETHREAD_QUIT);
  }
  /**
   * Screen updates
   */
  if(EMU_UPD_CHK_FLAG(pEmulator,EMU_UPDATE_SETTING_SCREEN_SCALE))
  {
    if(iChip8_Screen_SetScale_g(&pEmulator->tagWindow))
    {
      TRACE_DBG_ERROR("iChip8_Screen_SetScale_g() failed");
      iRc=EMU_SET_OPTION_ERROR;
    }
    else /* Clear screen on resize */
      EMU_UPD_SET_FLAG(pEmulator,EMU_UPDATE_SETTING_SCREEN_CLEAR);
    EMU_UPD_RST_FLAG(pEmulator,EMU_UPDATE_SETTING_SCREEN_SCALE);
  }
  if(EMU_UPD_CHK_FLAG(pEmulator,EMU_UPDATE_SETTING_SCREEN_POS))
  {
    vChip8_Screen_SetWinPosition_g(&pEmulator->tagWindow);
    EMU_UPD_RST_FLAG(pEmulator,EMU_UPDATE_SETTING_SCREEN_POS);
  }
  if(EMU_UPD_CHK_FLAG(pEmulator,EMU_UPDATE_SETTING_SCREEN_SHOW))
  {
    vChip8_Screen_Show_g(&pEmulator->tagWindow,1);
    EMU_UPD_RST_FLAG(pEmulator,EMU_UPDATE_SETTING_SCREEN_SHOW);
  }
  if(EMU_UPD_CHK_FLAG(pEmulator,EMU_UPDATE_SETTING_SCREEN_HIDE))
  {
    vChip8_Screen_Show_g(&pEmulator->tagWindow,0);
    EMU_UPD_RST_FLAG(pEmulator,EMU_UPDATE_SETTING_SCREEN_HIDE);
  }
  if((pEmulator->tagWindow.iVisible) && (EMU_UPD_CHK_FLAG(pEmulator,EMU_UPDATE_SETTING_SCREEN_CLEAR)))
  {
    vChip8_Screen_Clear_g(&pEmulator->tagWindow);
    EMU_UPD_RST_FLAG(pEmulator,EMU_UPDATE_SETTING_SCREEN_CLEAR);
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
        vChip8_Screen_Draw_g(&pEmulator->tagWindow,&pEmulator->taChipMemory[OFF_ADDR_SCREEN_START]);
        break;
      case THREAD_UPDATE_STATE_PAUSED:
        SDL_Delay(20);
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
