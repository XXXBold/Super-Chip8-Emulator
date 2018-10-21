#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "Chip8_Emulator.h"

#define CHIP8_TESTPGM_PATH "Pong_1P.ch8"

int main(void)
{
  int iIndex;
  iChip8_Init_g();
  if(iChip8_LoadFile_g(CHIP8_TESTPGM_PATH,0x200))
  {
    puts("Loading file failed, exiting");
    return(EXIT_FAILURE);
  }
  for(iIndex=0;iIndex<300;++iIndex)
//while(1)
  {
    Sleep(2);
    vChip8_Process_g();
  }
  puts("Emulation done");
  getchar();
  vChip8_Close_g();

  return(EXIT_SUCCESS);
}
