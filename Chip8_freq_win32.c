#include <stdio.h>
#include <process.h>
#include <windows.h>

#include "Chip8_freq.h"

typedef enum
{
  eChip8Freq_Thread_Inactive,
  eChip8Freq_Thread_Active
}EChip8Freq_ThreadStatus;

unsigned int __stdcall uiThreadFreqFunc_m(void *pvArg);

static uintptr_t tThreadHandle_m;
static EChip8Freq_ThreadStatus eThreadStatus_m;
static int iThread_Stop_m;

int iChip8_Frequency_Start_g(FreqFunc func)
{
  if(eThreadStatus_m!=eChip8Freq_Thread_Inactive)
  {
    fprintf(stderr,"Can't start Thread, already running\n");
    return(-1);
  }
  iThread_Stop_m=0;
  tThreadHandle_m=_beginthreadex(NULL,
                                 0,
                                 uiThreadFreqFunc_m,
                                 func,
                                 0,
                                 NULL);

  if((!tThreadHandle_m) || (tThreadHandle_m==1L))
  {
    fprintf(stderr,"_beginthreadex() failed\n");
    return(-2);
  }
  return(0);
}

void vChip8_Frequency_Stop_g(void)
{
  if(eThreadStatus_m!=eChip8Freq_Thread_Active)
  {
    fprintf(stderr,"Can't stop Thread, not running");
    return;
  }
  iThread_Stop_m=1;
  WaitForSingleObject((HANDLE)tThreadHandle_m,0);
}

unsigned int __stdcall uiThreadFreqFunc_m(void *pvArg)
{
  FreqFunc *pfFreq=pvArg;

  eThreadStatus_m=eChip8Freq_Thread_Active;
  while(!iThread_Stop_m)
  {
    pfFreq();
    Sleep(16);  //TODO: More accurate?
  }
  eThreadStatus_m=eChip8Freq_Thread_Inactive;
  return(0);
}


