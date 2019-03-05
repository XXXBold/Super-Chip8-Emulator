#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "chip8_Emulator.h"
#include "chip8_global.h"
#include "chip8_extern.h"

//#define CHIP8_TEST_VALUES /* Enable this for testing purposes */

/* CMDs starting with 0x00 */
#define CHIP8_CMD_CLS              0x00E0
#define CHIP8_CMD_RET              0x00EE

/* CMDs starting with 0x8x */
#define CHIP8_CMD_MATH_MOV         0x0
#define CHIP8_CMD_MATH_OR          0x1
#define CHIP8_CMD_MATH_AND         0x2
#define CHIP8_CMD_MATH_XOR         0x3
#define CHIP8_CMD_MATH_ADD         0x4
#define CHIP8_CMD_MATH_SUB         0x5
#define CHIP8_CMD_MATH_SHR         0x6
#define CHIP8_CMD_MATH_SUBN        0x7
#define CHIP8_CMD_MATH_SHL         0xE

/* CMDs starting with 0xEx */
#define CHIP8_CMD_KEY_PRESSED      0x009E
#define CHIP8_CMD_KEY_RELEASED     0x00A1

/* CMDS Starting with 0xFx */
#define CHIP8_CMD_DELAY_TIMER_GET  0x0007
#define CHIP8_CMD_WAIT_FOR_KEY     0x000A
#define CHIP8_CMD_DELAY_TIMER_SET  0x0015
#define CHIP8_CMD_SOUND_TIMER_SET  0x0018
#define CHIP8_CMD_I_ADD_VX         0x001E
#define CHIP8_CMD_I_SPRITE_GET     0x0029
#define CHIP8_CMD_VX_GET_BCD       0x0033
#define CHIP8_CMD_REGISTERS_STORE  0x0055
#define CHIP8_CMD_REGISTERS_READ   0x0065

#define TXT_INSTRUCTION_GENERAL "[0x%.4X] 0x%.4X"

/* Text for each OP Code */
#define TXT_OPC_ASM_0x00E0 "CLS"
#define TXT_OPC_ASM_0x00EE "RET"
#define TXT_OPC_ASM_0x1NNN "JMP @0x%.2X"
#define TXT_OPC_ASM_0x2NNN "CALL @0x%.2X"
#define TXT_OPC_ASM_0x3XKK "SKEQ V%X 0x%.2X"
#define TXT_OPC_ASM_0x4XKK "SKNE V%X 0x%.2X"
#define TXT_OPC_ASM_0x5XY0 "SKEQ V%X V%X"
#define TXT_OPC_ASM_0x6XKK "LD V%X 0x%.2X"
#define TXT_OPC_ASM_0x7XKK "ADD V%X 0x%.2X"
#define TXT_OPC_ASM_0x8XY0 "MOV V%X V%X"
#define TXT_OPC_ASM_0x8XY1 "OR V%X V%X"
#define TXT_OPC_ASM_0x8XY2 "AND V%X V%X"
#define TXT_OPC_ASM_0x8XY3 "XOR V%X V%X"
#define TXT_OPC_ASM_0x8XY4 "ADD V%X V%X"
#define TXT_OPC_ASM_0x8XY5 "SUB V%X V%X"
#define TXT_OPC_ASM_0x8X06 "SHR V%X"
#define TXT_OPC_ASM_0x8XY7 "RSB V%X V%X"
#define TXT_OPC_ASM_0x8X0E "SHL V%X"
#define TXT_OPC_ASM_0x9XY0 "SKNE V%X V%X"
#define TXT_OPC_ASM_0xANNN "MVI 0x%.2X"
#define TXT_OPC_ASM_0xBNNN "JMI 0x%.2X"
#define TXT_OPC_ASM_0xCXKK "RAND V%X 0x%.2X"
#define TXT_OPC_ASM_0xDXYN "SPRITE V%X V%X 0x%.2X"
#define TXT_OPC_ASM_0xER9E "SKPR 0x%.2x"
#define TXT_OPC_ASM_0xERA1 "SKUP 0x%.2X"
#define TXT_OPC_ASM_0xFR07 "GDELAY V%X"
#define TXT_OPC_ASM_0xFR0A "KEY V%X"
#define TXT_OPC_ASM_0xFR15 "SDELAY V%X"
#define TXT_OPC_ASM_0xFR18 "SSOUND V%X"
#define TXT_OPC_ASM_0xFR1E "ADI V%X"
#define TXT_OPC_ASM_0xFR29 "FONT V%X"
#define TXT_OPC_ASM_0xFR33 "BCD V%X"
#define TXT_OPC_ASM_0xFR55 "STR V%X"
#define TXT_OPC_ASM_0xFR65 "LDR V%X"

#define TXT_OPC_DESC_0x00E0 "Clear Screen"
#define TXT_OPC_DESC_0x00EE "Return from subroutine"
#define TXT_OPC_DESC_0x1NNN "Jump to Address 0x%.2X"
#define TXT_OPC_DESC_0x2NNN "Jump to Subroutine at Address 0x%.2X"
#define TXT_OPC_DESC_0x3XKK "Skip Next Instruction if V%X(=0x%.2X) == 0x%.2X"
#define TXT_OPC_DESC_0x4XKK "Skip Next Instruction if V%X(=0x%.2X) != 0x%.2X"
#define TXT_OPC_DESC_0x5XY0 "Skip Next Instruction if V%X==V%X"
#define TXT_OPC_DESC_0x6XKK "Set val 0x%.2X -> V%X"
#define TXT_OPC_DESC_0x7XKK "Add val 0x%.2X -> V%X"
#define TXT_OPC_DESC_0x8XY0 "Move V%X(=0x%.2X) -> V%X"
#define TXT_OPC_DESC_0x8XY1 "Or V%X(=0x%.2X) -> V%X(=0x%.2X)"
#define TXT_OPC_DESC_0x8XY2 "And V%X(=0x%.2X) -> V%X(=0x%.2X)"
#define TXT_OPC_DESC_0x8XY3 "Xor V%X(=0x%.2X) -> V%X(=0x%.2X)"
#define TXT_OPC_DESC_0x8XY4 "Add V%X(=0x%.2X) -> V%X(=0x%.2X)"
#define TXT_OPC_DESC_0x8XY5 "Subtract V%X(=0x%.2X) -> V%X(=0x%.2X)"
#define TXT_OPC_DESC_0x8X06 "Shift right V%X(=0x%.2X)"
#define TXT_OPC_DESC_0x8XY7 "Subtract Reverseorder V%X(=0x%.2X) - V%X(=0x%.2X)"
#define TXT_OPC_DESC_0x8X0E "Shift left V%X(=0x%.2X)"
#define TXT_OPC_DESC_0x9XY0 "Skip Next Instruction if V%X(=0x%.2X)!=V%X(=0x%.2X)"
#define TXT_OPC_DESC_0xANNN "Set Reg I =0x%.2X"
#define TXT_OPC_DESC_0xBNNN "Jump to V0(=0x%.2X) + nnn(=0x%.2X)"
#define TXT_OPC_DESC_0xCXKK "Random Number AND 0x%.2X -> V%X"
#define TXT_OPC_DESC_0xDXYN "Draw 0x%.2X bytes @ V%X(=0x%.2X)/V%X(=0x%.2X)"
#define TXT_OPC_DESC_0xER9E "Skip if Key 0x%x pressed"
#define TXT_OPC_DESC_0xERA1 "SKip if key 0x%X not pressed"
#define TXT_OPC_DESC_0xFR07 "Get delay timer value(=0x%.2X) -> V%X"
#define TXT_OPC_DESC_0xFR0A "Wait for Key and store -> V%X"
#define TXT_OPC_DESC_0xFR15 "Set Delay Timer value <- V%X(=0x%.2X)"
#define TXT_OPC_DESC_0xFR18 "Set Sound Timer <- V%X(=0x%.2X)"
#define TXT_OPC_DESC_0xFR1E "Increment Reg I(=0x%.3X) by V%X(=0x%.2X)"
#define TXT_OPC_DESC_0xFR29 "Set Reg I to Sprite in V%X(No. 0x%.2X)"
#define TXT_OPC_DESC_0xFR33 "Store BCD of V%X(=0x%.2X) -> Reg I"
#define TXT_OPC_DESC_0xFR55 "Store registers V0-V%X on Memory pointed by Reg I"
#define TXT_OPC_DESC_0xFR65 "Read from Memory pointed by Reg I to Registers V0-V%X"

#define TXT_INSTRUCTION_0x00E0 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x00E0 ": " TXT_OPC_DESC_0x00E0
#define TXT_INSTRUCTION_0x00EE TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x00EE ": " TXT_OPC_DESC_0x00EE
#define TXT_INSTRUCTION_0x1NNN TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x1NNN ": " TXT_OPC_DESC_0x1NNN
#define TXT_INSTRUCTION_0x2NNN TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x2NNN ": " TXT_OPC_DESC_0x2NNN
#define TXT_INSTRUCTION_0x3XKK TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x3XKK ": " TXT_OPC_DESC_0x3XKK
#define TXT_INSTRUCTION_0x4XKK TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x4XKK ": " TXT_OPC_DESC_0x4XKK
#define TXT_INSTRUCTION_0x5XY0 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x5XY0 ": " TXT_OPC_DESC_0x5XY0
#define TXT_INSTRUCTION_0x6XKK TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x6XKK ": " TXT_OPC_DESC_0x6XKK
#define TXT_INSTRUCTION_0x7XKK TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x7XKK ": " TXT_OPC_DESC_0x7XKK
#define TXT_INSTRUCTION_0x8XY0 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x8XY0 ": " TXT_OPC_DESC_0x8XY0
#define TXT_INSTRUCTION_0x8XY1 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x8XY1 ": " TXT_OPC_DESC_0x8XY1
#define TXT_INSTRUCTION_0x8XY2 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x8XY2 ": " TXT_OPC_DESC_0x8XY2
#define TXT_INSTRUCTION_0x8XY3 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x8XY3 ": " TXT_OPC_DESC_0x8XY3
#define TXT_INSTRUCTION_0x8XY4 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x8XY4 ": " TXT_OPC_DESC_0x8XY4
#define TXT_INSTRUCTION_0x8XY5 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x8XY5 ": " TXT_OPC_DESC_0x8XY5
#define TXT_INSTRUCTION_0x8X06 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x8X06 ": " TXT_OPC_DESC_0x8X06
#define TXT_INSTRUCTION_0x8XY7 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x8XY7 ": " TXT_OPC_DESC_0x8XY7
#define TXT_INSTRUCTION_0x8X0E TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x8X0E ": " TXT_OPC_DESC_0x8X0E
#define TXT_INSTRUCTION_0x9XY0 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x9XY0 ": " TXT_OPC_DESC_0x9XY0
#define TXT_INSTRUCTION_0xANNN TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xANNN ": " TXT_OPC_DESC_0xANNN
#define TXT_INSTRUCTION_0xBNNN TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xBNNN ": " TXT_OPC_DESC_0xBNNN
#define TXT_INSTRUCTION_0xCXKK TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xCXKK ": " TXT_OPC_DESC_0xCXKK
#define TXT_INSTRUCTION_0xDXYN TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xDXYN ": " TXT_OPC_DESC_0xDXYN
#define TXT_INSTRUCTION_0xER9E TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xER9E ": " TXT_OPC_DESC_0xER9E
#define TXT_INSTRUCTION_0xERA1 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xERA1 ": " TXT_OPC_DESC_0xERA1
#define TXT_INSTRUCTION_0xFR07 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFR07 ": " TXT_OPC_DESC_0xFR07
#define TXT_INSTRUCTION_0xFR0A TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFR0A ": " TXT_OPC_DESC_0xFR0A
#define TXT_INSTRUCTION_0xFR15 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFR15 ": " TXT_OPC_DESC_0xFR15
#define TXT_INSTRUCTION_0xFR18 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFR18 ": " TXT_OPC_DESC_0xFR18
#define TXT_INSTRUCTION_0xFR1E TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFR1E ": " TXT_OPC_DESC_0xFR1E
#define TXT_INSTRUCTION_0xFR29 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFR29 ": " TXT_OPC_DESC_0xFR29
#define TXT_INSTRUCTION_0xFR33 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFR33 ": " TXT_OPC_DESC_0xFR33
#define TXT_INSTRUCTION_0xFR55 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFR55 ": " TXT_OPC_DESC_0xFR55
#define TXT_INSTRUCTION_0xFR65 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFR65 ": " TXT_OPC_DESC_0xFR65

#define MEM_JMP_ADDR_VALID(addr) (((addr<OFF_ADDR_USER_START) || (addr>OFF_ADDR_USER_END))?0:1)
#define SCREEN_CHECK_PIXEL_EREASED(curr,last) ((((curr)&(last))<(last))?1:0)
#define MEM_ADDRESS_AS_WORD(index) *(word*)(&tagEmulator_g.taChipMemory[index])

#define REG_STACKPTR_1UP(stptr) (stptr)-=sizeof(word)
#define REG_STACKPTR_1DOWN(stptr) (stptr)+=sizeof(word)
#define REG_PROGRAMCOUNTER_INCREMENT() REG_PC+=sizeof(word)

//#define MEM_SCREEN_BIT_SET(x,y)   tagEmulator_g.taChipMemory[OFF_ADDR_SCREEN_START+SCREEN_CALC_INDEX(x,y)]|=(0x80>>((x)%8))
//#define MEM_SCREEN_BIT_UNSET(x,y) tagEmulator_g.taChipMemory[OFF_ADDR_SCREEN_START+SCREEN_CALC_INDEX(x,y)]&=~(0x80>>((x)%8))
#define MEM_SCREEN_BIT_CHECK(x,y) (tagEmulator_g.taChipMemory[OFF_ADDR_SCREEN_START+SCREEN_CALC_INDEX(x,y)]&(0x80>>((x)%8)))
#define MEM_SCREEN_BYTE(x,y)      tagEmulator_g.taChipMemory[OFF_ADDR_SCREEN_START+SCREEN_CALC_INDEX(x,y)]

INLINE_PROT void chip8_MemoryInit_m(byte *pMem,
                                    word WStartAddress);

INLINE_PROT int iInstruction_GetPressedKey_m(TagKeyboard *ptagKeyboard,
                                             byte *pReg);

INLINE_PROT int iInstruction_Sprite(byte *pMem,
                                    byte tRegX,
                                    byte tRegY,
                                    byte tBytes); /* 0xDxyn */

int chip8_Process(TagEmulator *pEmulator);

static const byte tChip8_font_m[]=
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
  0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
  0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
  0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
  0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
  0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
  0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
  0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
  0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
  0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
  0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
  0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
  0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
  0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
  0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
  0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

TagEmulator tagEmulator_g;

int chip8_Init(int winPosX,
               int winPosY,
               unsigned int winScaleFactor,
               const unsigned char keyMap[EMU_KEY_COUNT],
               pCallbackFunc cbFunc,
               unsigned int callbackEvents)
{
  TRACE_DBG_INFO("Init Chip-8 Emulator...");
  tagEmulator_g.tagWindow.iPosX=winPosX;
  tagEmulator_g.tagWindow.iPosY=winPosY;
  tagEmulator_g.tagWindow.uiScale=(winScaleFactor)?winScaleFactor:CHIP8_SCREEN_DEFAULT_SCALE;

  tagEmulator_g.tagThrdEmu.eSpeed=EMU_SPEED_1_0X;
  tagEmulator_g.tagThrdEmu.eEmuState=EMU_STATE_INACTIVE;
  tagEmulator_g.tagThrdEmu.usrCBFunc=cbFunc;
  tagEmulator_g.tagThrdEmu.usrCBEvents=callbackEvents;
  tagEmulator_g.tagThrdEmu.procFunc=chip8_Process;

  tagEmulator_g.tagThrdRenderer.eThreadState=THREAD_UPDATE_STATE_INACTIVE;

  tagEmulator_g.tagPlaySound.uiFreqSoundHz=CHIP8_SOUND_FREQ;

  tagEmulator_g.uiSettingsChanged=0;
  tagEmulator_g.tCurrOPCode=0;

  if(keyMap)
  {
    TRACE_DBG_INFO("Keymap: using custom");
    memcpy(tagEmulator_g.tagKeyboard.ucaUsrKeyMap,
           keyMap,
           sizeof(tagEmulator_g.tagKeyboard.ucaUsrKeyMap));
  }
  else
  {
    TRACE_DBG_INFO("Keymap: using default");
    memset(tagEmulator_g.tagKeyboard.ucaUsrKeyMap,
           EMU_KEY_DEFAULT,
           sizeof(tagEmulator_g.tagKeyboard.ucaUsrKeyMap));
  }
  /*Check if first call of this function, initialise rng then */
  srand((unsigned int)time(NULL));
  chip8_MemoryInit_m(tagEmulator_g.taChipMemory,OFF_ADDR_USER_START);

  if(iChip8_External_Init_g(&tagEmulator_g)) /* Init external libraries */
  {
    TRACE_DBG_ERROR("iChip8_External_Init_g() failed");
    return(-1);
  }
  return(0);
}

void chip8_SetKeyState_m(TagEmulator *pEmulator,
                         unsigned char *pucKeyMap);

void chip8_Close(void)
{
  TRACE_DBG_INFO("Shutting down Chip-8 Emulator...");
  vChip8_External_Close_g(&tagEmulator_g);
}

EEmulatorState chip8_GetEmulatorState(void)
{
  return(tagEmulator_g.tagThrdEmu.eEmuState);
}

void chip8_SetPause(int pause)
{
  /* Return if emulator is not initialised yet */
  if(tagEmulator_g.tagThrdEmu.eEmuState==EMU_STATE_INACTIVE)
    return;
  if(pause)
  {
    EMU_UPD_SET_FLAG(&tagEmulator_g,EMU_UPDATE_SETTING_RUNTHREAD_PAUSE);
  }
  else
  {
    EMU_UPD_SET_FLAG(&tagEmulator_g,EMU_UPDATE_SETTING_SCREEN_SHOW);
    EMU_UPD_SET_FLAG(&tagEmulator_g,EMU_UPDATE_SETTING_RUNTHREAD_RUN);
  }

  /* Wait for thread to update setting */
  while(tagEmulator_g.uiSettingsChanged)
  {
    ;
  }
}

void chip8_SetWindowScale(unsigned int windowScale)
{
  /* Return if emulator is not initialised yet */
  if(tagEmulator_g.tagThrdEmu.eEmuState==EMU_STATE_INACTIVE)
    return;

  if(tagEmulator_g.tagWindow.uiScale==windowScale)
    return;

  tagEmulator_g.tagWindow.uiScale=windowScale;
  EMU_UPD_SET_FLAG(&tagEmulator_g,EMU_UPDATE_SETTING_SCREEN_SCALE);

  /* Wait for thread to set window size */
  while(tagEmulator_g.uiSettingsChanged)
  {
    ;
  }
}

void chip8_SetWindowPosition(int iPosX,
                             int iPosY)
{
  /* Return if emulator is not initialised yet */
  if(tagEmulator_g.tagThrdEmu.eEmuState==EMU_STATE_INACTIVE)
    return;
  if((tagEmulator_g.tagWindow.iPosX==iPosX) &&
     (tagEmulator_g.tagWindow.iPosY==iPosY))
  {
    return;
  }
  tagEmulator_g.tagWindow.iPosX=iPosX;
  tagEmulator_g.tagWindow.iPosY=iPosY;
  EMU_UPD_SET_FLAG(&tagEmulator_g,EMU_UPDATE_SETTING_SCREEN_POS);
  /* Wait for thread to set window pos */
  while(tagEmulator_g.uiSettingsChanged)
  {
    ;
  }
}

void chip8_SetWindowVisible(int visible)
{
  /* Return if emulator is not initialised yet */
  if(tagEmulator_g.tagThrdEmu.eEmuState==EMU_STATE_INACTIVE)
    return;

  if(visible)
    EMU_UPD_SET_FLAG(&tagEmulator_g,EMU_UPDATE_SETTING_SCREEN_SHOW);
  else
    EMU_UPD_SET_FLAG(&tagEmulator_g,EMU_UPDATE_SETTING_SCREEN_HIDE);

  /* Wait for thread to set window visibility */
  while(tagEmulator_g.uiSettingsChanged)
  {
    ;
  }
}

void chip8_SetEmulationSpeed(EEmulationSpeed eSpeed)
{
  /* Return if emulator is not initialised yet */
  if(tagEmulator_g.tagThrdEmu.eEmuState==EMU_STATE_INACTIVE)
    return;
  TRACE_DBG_INFO_VARG("Setting Emulation speed=%u...",eSpeed);
  if(eSpeed==tagEmulator_g.tagThrdEmu.eSpeed)
    return;
  tagEmulator_g.tagThrdEmu.eSpeed=eSpeed;
  EMU_UPD_SET_FLAG(&tagEmulator_g,EMU_UPDATE_SETTING_EMULATION_SPEED);

  /* Wait for speed to get updated */
  while(tagEmulator_g.uiSettingsChanged)
  {
    ;
  }
}

void chip8_SetKeymap(unsigned char *newKeymap)
{
  memcpy(tagEmulator_g.tagKeyboard.ucaUsrKeyMap,newKeymap,sizeof(tagEmulator_g.tagKeyboard.ucaUsrKeyMap)),

  EMU_UPD_SET_FLAG(&tagEmulator_g,EMU_UPDATE_SETTING_KEYBOARD_KEYMAP);

  while(tagEmulator_g.uiSettingsChanged)
  {
    ;
  }
}

int chip8_LoadFile(const char *pcFilePath,
                   word tStartAddress)
{
  FILE *fp;
  byte taMemTemp[OFF_ADDR_USER_END-OFF_ADDR_USER_START+1];

  chip8_SetPause(1);

  TRACE_DBG_INFO_VARG("Loading File \"%s\" @0x%X",pcFilePath,tStartAddress);
  if(!MEM_JMP_ADDR_VALID(tStartAddress))
  {
    TRACE_DBG_ERROR_VARG("chip8_LoadFile() failed: invalid Startaddress: 0x%X",tStartAddress);
    return(-1);
  }
  errno=0;
  if(!(fp=fopen(pcFilePath,"rb")))
  {
    TRACE_DBG_ERROR_VARG("chip8_LoadFile() failed to open File \"%s\": (%d) %s",pcFilePath,errno,strerror(errno));
    return(-1);
  }
  if(fread(taMemTemp,1,sizeof(taMemTemp)-tStartAddress,fp)<1)
  {
    if(ferror(fp))
    {
      TRACE_DBG_ERROR_VARG("chip8_LoadFile(): fread() failed on file \"%s\"",pcFilePath);
    }
    else
      TRACE_DBG_ERROR_VARG("chip8_LoadFile(): File \"%s\" is empty",pcFilePath);

    fclose(fp);
    return(-1);
  }
  if(!feof(fp))
  {
    TRACE_DBG_ERROR_VARG("chip8_LoadFile(): file \"%s\" is too big (> 0x%X-0x%X)",pcFilePath,tStartAddress,OFF_ADDR_USER_END);
    fclose(fp);
    return(-1);
  }
  fclose(fp);
  chip8_MemoryInit_m(tagEmulator_g.taChipMemory,tStartAddress);
  memcpy(&tagEmulator_g.taChipMemory[tStartAddress],taMemTemp,sizeof(taMemTemp)-tStartAddress);
  EMU_UPD_SET_FLAG(&tagEmulator_g,EMU_UPDATE_SETTING_SCREEN_CLEAR);
  return(0);
}

void chip8_DumpScreen(void)
{
  int iIndex, iIndexB;
  char caTextBuffer[CHIP8_SCREEN_WIDTH+3];

  caTextBuffer[0]='|';
  caTextBuffer[CHIP8_SCREEN_WIDTH+1]='|';
  caTextBuffer[CHIP8_SCREEN_WIDTH+2]='\0';
  puts("Display (1=white, 0=black):"
       "------------------------------------------------------------------");
  for(iIndex=0;iIndex<CHIP8_SCREEN_HEIGHT;++iIndex)
  {
    for(iIndexB=0;iIndexB<CHIP8_SCREEN_WIDTH;++iIndexB)
    {
      caTextBuffer[iIndexB+1]=MEM_SCREEN_BIT_CHECK(iIndexB,iIndex)?'1':'0';
    }
    puts(caTextBuffer);
  }
  puts("------------------------------------------------------------------");
}

int chip8_Process(TagEmulator *pEmulator)
{
  word tOPCode;
  byte tRegX;
  byte tRegY;

  tOPCode=tagEmulator_g.taChipMemory[REG_PC];
  tOPCode=(tOPCode<<8)|tagEmulator_g.taChipMemory[REG_PC+1];
  pEmulator->tCurrOPCode=tOPCode;
  TRACE_DBG_INFO_VARG("Current OPCode: 0x%.4X @0x%.4X",tOPCode,REG_PC);
  switch(tOPCode&0xF000)
  {
    case 0x0000: /* 0x0xxx, 0x00E0=CLS, 0x00EE=Return from subroutine */
      switch((tOPCode&0x00FF))
      {
        case CHIP8_CMD_CLS:
          TRACE_INSTR(TXT_INSTRUCTION_0x00E0,REG_PC,pEmulator->tCurrOPCode);
          EMU_UPD_SET_FLAG(&tagEmulator_g,EMU_UPDATE_SETTING_SCREEN_CLEAR);
          break;
        case CHIP8_CMD_RET:
          printf("Stackptr: 0x%X, startstack: 0x%X, Endstack: 0x%X\n",REG_PTR_STACK,OFF_ADDR_STACK_START,OFF_ADDR_STACK_END);
          if(REG_PTR_STACK>OFF_ADDR_STACK_START)
          {
            TRACE_INSTR(TXT_INSTRUCTION_0x00EE,REG_PC,pEmulator->tCurrOPCode);
            REG_STACKPTR_1UP(REG_PTR_STACK); /* Move stackpointer 1 level up */
            REG_PC=MEM_ADDRESS_AS_WORD(REG_PTR_STACK);
            TRACE_DBG_INFO_VARG("Returning from subroutine (->@0x%.4X), deepness: <--%u/%u",
                                REG_PC,
                                (word)((REG_PTR_STACK-OFF_ADDR_STACK_START)/sizeof(word)),
                                (word)((OFF_ADDR_STACK_END-OFF_ADDR_STACK_START+1)/sizeof(word)));
            break; /* Break okay, will increment stored address below and continue at previous point */
          }
          TRACE_DBG_ERROR("RET failed, already on top, end pgm here?");
          return(ERR_STACK_ON_TOP);
        default:
          return(ERR_INSTRUCTION_UNKNOWN);
      }
      break;
    case 0x1000: /* 0x1nnn, Jump to address */
      tOPCode&=0xFFF;
      TRACE_INSTR(TXT_INSTRUCTION_0x1NNN,REG_PC,pEmulator->tCurrOPCode,tOPCode,tOPCode);
      if(MEM_JMP_ADDR_VALID(tOPCode))
      {
        REG_PC=tOPCode;
        return(ERR_NONE); /* Return immediately, no increment of PC needed */
      }
      TRACE_DBG_ERROR_VARG("JMP failed, tried to jump to addr 0x%.4X (0x%.4X-0x%.4X allowed)",tOPCode,OFF_ADDR_USER_START,OFF_ADDR_USER_END);
      return(ERR_INVALID_JUMP_ADDRESS);
    case 0x2000: /* 0x2nnn, Jump to addr, as a subroutine */
      tOPCode&=0xFFF;
      TRACE_INSTR(TXT_INSTRUCTION_0x2NNN,REG_PC,pEmulator->tCurrOPCode,tOPCode,tOPCode);
      if(MEM_JMP_ADDR_VALID(tOPCode))
      {
        if(REG_PTR_STACK<OFF_ADDR_STACK_END)
        {
          MEM_ADDRESS_AS_WORD(REG_PTR_STACK)=REG_PC; /* Store curr PC in Stack */
          REG_PC=tOPCode;
          TRACE_DBG_INFO_VARG("Storing Current Programmcounter (0x%.4X) @Stack: 0x%.4X",REG_PC,REG_PTR_STACK);
          REG_STACKPTR_1DOWN(REG_PTR_STACK); /* Move stackpointer 1 level down */
          return(ERR_NONE); /* Return immediately, no increment of PC needed */
        }
        TRACE_DBG_ERROR_VARG("CALL failed, max emulator nested calls reached (%u allowed)",OFF_ADDR_STACK_END-OFF_ADDR_STACK_START+1);
        return(ERR_STACK_MAX_CALLS);
      }
      TRACE_DBG_ERROR_VARG("CALL failed, tried to jump to addr 0x%.4X (0x%.4X-0x%.4X allowed)",tOPCode,OFF_ADDR_USER_START,OFF_ADDR_USER_END);
      return(ERR_INVALID_JUMP_ADDRESS);
    case 0x3000: /* 0x3xkk, Compare Reg x with value kk, if equal -> Skip next instruction */
      tRegX=(tOPCode&0x0F00)>>8;
      tOPCode&=0x00FF;
      TRACE_INSTR(TXT_INSTRUCTION_0x3XKK,REG_PC,pEmulator->tCurrOPCode,tRegX,tOPCode,tRegX,REG_VX(tRegX),tOPCode);
      if(REG_VX(tRegX)==tOPCode)
      {
        TRACE_DBG_INFO_VARG("Skipping next instruction, Reg_V%X==0x%.2X",tRegX,tOPCode);
        REG_PROGRAMCOUNTER_INCREMENT(); /* Increment here, and at the end of switch, results in skipping */
        break;
      }
      TRACE_DBG_INFO_VARG("Not skipping next instruction, Reg_V%X!=0x%.2X",tRegX,tOPCode);
      break;
    case 0x4000: /* 0x4xkk, Compare Reg x with value kk, if not equal -> Skip next instruction */
      tRegX=(tOPCode&0x0F00)>>8;
      tOPCode&=0x00FF;
      TRACE_INSTR(TXT_INSTRUCTION_0x4XKK,REG_PC,pEmulator->tCurrOPCode,tRegX,tOPCode,tRegX,REG_VX(tRegX),tOPCode);
      if(REG_VX(tRegX)!=tOPCode)
      {
        TRACE_DBG_INFO_VARG("Skipping next instruction, Reg_V%X(=0x%.2X)!=0x%.2X",tRegX,REG_VX(tRegX),tOPCode);
        REG_PROGRAMCOUNTER_INCREMENT(); /* Increment here, and at the end of switch, results in skipping */
        break;
      }
      TRACE_DBG_INFO_VARG("Not skipping next instruction, Reg_V%X==0x%.2X",tRegX,tOPCode);
      break;
    case 0x5000: /* 0x5xy0, Compare Reg x with Reg y, if values are equal -> Skip next instruction */
      if(tOPCode&0x000F) /* Nibble must be 0 */
        return(ERR_INSTRUCTION_UNKNOWN);
      tRegX=(tOPCode&0x0F00)>>8;
      tRegY=(tOPCode&0x00F0)>>4;
      TRACE_INSTR(TXT_INSTRUCTION_0x5XY0,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegX,tRegY);
      if(REG_VX(tRegX)==REG_VX(tRegY))
      {
        TRACE_DBG_INFO_VARG("Skipping next instruction, Reg_V%X and Reg_V%X ==0x%.2X",
                            tRegX,
                            tRegY,
                            REG_VX(tRegY));
        REG_PROGRAMCOUNTER_INCREMENT(); /* Increment here, and at the end of switch, results in skipping */
        break;
      }
      TRACE_DBG_INFO_VARG("Not skipping next instruction, Reg_V%X=0x%.2X, Reg_V%X=0x%.2X",
                          tRegX,
                          REG_VX(tRegX),
                          tRegY,
                          REG_VX(tRegY));
      break;
    case 0x6000: /* 0x6xkk, Set Register x to value kk */
      tRegX=(tOPCode&0x0F00)>>8;
      tOPCode&=0x00FF;
      TRACE_INSTR(TXT_INSTRUCTION_0x6XKK,REG_PC,pEmulator->tCurrOPCode,tRegX,tOPCode,tOPCode,tRegX);
      REG_VX(tRegX)=tOPCode;
      break;
    case 0x7000: /* 0x7xkk, Add Value kk to Register x */
      tRegX=(tOPCode&0x0F00)>>8;
      tOPCode&=0x00FF;
      TRACE_INSTR(TXT_INSTRUCTION_0x7XKK,REG_PC,pEmulator->tCurrOPCode,tRegX,tOPCode,tOPCode,tRegX);
      REG_VX(tRegX)+=tOPCode;
      break;
    case 0x8000: /* Mathematical Operations */
      tRegX=(tOPCode&0x0F00)>>8;
      tRegY=(tOPCode&0x00F0)>>4;
      switch(tOPCode&0x000F)
      {
        case CHIP8_CMD_MATH_MOV: /* MOV */
          TRACE_INSTR(TXT_INSTRUCTION_0x8XY0,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegY,REG_VX(tRegY),tRegX);
          REG_VX(tRegX)=REG_VX(tRegY);
          break;
        case CHIP8_CMD_MATH_OR: /* OR */
          TRACE_INSTR(TXT_INSTRUCTION_0x8XY1,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegY,REG_VX(tRegY),tRegX,REG_VX(tRegX));
          REG_VX(tRegX)|=REG_VX(tRegY);
          break;
        case CHIP8_CMD_MATH_AND: /* AND */
          TRACE_INSTR(TXT_INSTRUCTION_0x8XY2,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegY,REG_VX(tRegY),tRegX,REG_VX(tRegX));
          REG_VX(tRegX)&=REG_VX(tRegY);
          break;
        case CHIP8_CMD_MATH_XOR: /* XOR */
          TRACE_INSTR(TXT_INSTRUCTION_0x8XY3,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegY,REG_VX(tRegY),tRegX,REG_VX(tRegX));
          REG_VX(tRegX)^=REG_VX(tRegY);
          break;
        case CHIP8_CMD_MATH_ADD: /* ADD */
          TRACE_INSTR(TXT_INSTRUCTION_0x8XY4,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegY,REG_VX(tRegY),tRegX,REG_VX(tRegX));
          if((tOPCode=REG_VX(tRegX)+REG_VX(tRegY))>0xFF)
            REG_VF=1; /* Set overflow indication on VF */
          REG_VX(tRegX)=tOPCode&0xFF;
          break;
        case CHIP8_CMD_MATH_SUB: /* SUB */
          TRACE_INSTR(TXT_INSTRUCTION_0x8XY5,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegY,REG_VX(tRegY),tRegX,REG_VX(tRegX));
          REG_VF=(REG_VX(tRegX)>REG_VX(tRegY))?1:0; /* Set underflow indication on VF */
          REG_VX(tRegX)-=REG_VX(tRegY);
          break;
        case CHIP8_CMD_MATH_SHR: /* SHR */
          TRACE_INSTR(TXT_INSTRUCTION_0x8X06,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          REG_VF=(REG_VX(tRegX)&0x1)?1:0; /* If LSB (0000 0001) is set, set VF=1, otherways VF=0 */
          REG_VX(tRegX)>>=1;
          break;
        case CHIP8_CMD_MATH_SUBN: /* SUBN */
          TRACE_INSTR(TXT_INSTRUCTION_0x8XY7,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegX,REG_VX(tRegX),tRegY,REG_VX(tRegY));
          REG_VF=(REG_VX(tRegX)<REG_VX(tRegY))?1:0; /* Set underflow indication on VF */
          REG_VX(tRegX)=REG_VX(tRegY)-REG_VX(tRegX);
          break;
        case CHIP8_CMD_MATH_SHL: /* SHL */
          TRACE_INSTR(TXT_INSTRUCTION_0x8X0E,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          REG_VF=(REG_VX(tRegX)&0x80)?1:0; /* If MSB (1000 0000) is set, set VF=1, otherways VF=0 */
          REG_VX(tRegX)<<=1;
          break;
        default:
          return(ERR_INSTRUCTION_UNKNOWN);
      }
      break;
    case 0x9000: /* 0x9xy0, Compare Reg x with Reg y, if values are not equal -> Skip next instruction */
      if(tOPCode&0x000F) /* Nibble must be 0 */
        return(ERR_INSTRUCTION_UNKNOWN);
      tRegX=(tOPCode&0x0F00)>>8;
      tRegY=(tOPCode&0x00F0)>>4;
      TRACE_INSTR(TXT_INSTRUCTION_0x9XY0,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegX,REG_VX(tRegX),tRegY,REG_VX(tRegY));
      if(REG_VX(tRegX)!=REG_VX(tRegY))
      {
        TRACE_DBG_INFO_VARG("Skipping next instruction, Reg_V%X and Reg_V%X==0x%.2X",
                            tRegX,
                            tRegY,
                            REG_VX(tRegX));
        REG_PROGRAMCOUNTER_INCREMENT(); /* Increment here, and at the end of switch, results in skipping */
        break;
      }
      TRACE_DBG_INFO_VARG("Not skipping next instruction, Reg_V%X=0x%.2X and Reg_V%X==0x%.2X",
                          tRegX,
                          REG_VX(tRegX),
                          tRegY,
                          REG_VX(tRegY));
      break;
    case 0xA000: /* 0xAnnn, Set Register I = nnn */
      REG_I=(tOPCode&0x0FFF);
      TRACE_INSTR(TXT_INSTRUCTION_0xANNN,REG_PC,pEmulator->tCurrOPCode,REG_I,REG_I);
      break;
    case 0xB000: /* 0xBnnn, Jump to Reg I + V0 */
      tOPCode&=0x0FFF;
      TRACE_INSTR(TXT_INSTRUCTION_0xBNNN,REG_PC,pEmulator->tCurrOPCode,tOPCode,REG_V0,tOPCode);
      tOPCode+=REG_V0;
      if(MEM_JMP_ADDR_VALID(tOPCode))
      {
        REG_PC=tOPCode;
        return(ERR_NONE); /* Return immediately, no increment of PC needed */
      }
      TRACE_DBG_ERROR_VARG("JMP failed, addr 0x%.4X not valid",tOPCode);
      return(ERR_INVALID_JUMP_ADDRESS);
    case 0xC000: /* 0xCxkk, Generate random number, AND with val kk and store in Reg_Vx */
      tRegX=(tOPCode&0x0F00)>>8;
      tOPCode&=0x00FF;
      TRACE_INSTR(TXT_INSTRUCTION_0xCXKK,REG_PC,pEmulator->tCurrOPCode,tRegX,tOPCode,tOPCode,tRegX);
      REG_VX(tRegX)=(rand()%0x100)&tOPCode;
      break;
    case 0xD000: /* 0xDxyn, Draw n Sprites @Pos Reg_Vx/REG_Vy */
      tRegX=(tOPCode&0x0F00)>>8;
      tRegY=(tOPCode&0x00F0)>>4;
      tOPCode&=0x000F;
      TRACE_INSTR(TXT_INSTRUCTION_0xDXYN,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tOPCode,tOPCode,tRegX,REG_VX(tRegX),tRegY,REG_VX(tRegY));
      if((tOPCode=iInstruction_Sprite(tagEmulator_g.taChipMemory,REG_VX(tRegX),REG_VX(tRegY),tOPCode))!=ERR_NONE)
      {
        TRACE_DBG_ERROR("iInstruction_Sprite() failed to draw Screen");
        return(tOPCode);
      }
      break;
    case 0xE000: /* Keypress stuff */
      tRegX=(tOPCode&0x0F00)>>8;
      if(REG_VX(tRegX)>0xF) /* Check for invalid key */
      {
        TRACE_DBG_ERROR_VARG("Invalid Key in REG_V%X: %X, allowed: 0-F",tRegX,REG_VX(tRegX));
        return(ERR_KEYCODE_INVALID);
      }
      switch((tOPCode&0x00FF))
      {
        case CHIP8_CMD_KEY_PRESSED: /* 0xEx9E, Skip next instruction if REG_Vx==pressed key */
          TRACE_INSTR(TXT_INSTRUCTION_0xER9E,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX);   // TODO: check if key works
          if((pEmulator->tagKeyboard.ulKeyStates&(1<<REG_VX(tRegX))))
          {
            TRACE_DBG_INFO("Key is pressed, skipping next instruction");
            REG_PROGRAMCOUNTER_INCREMENT(); /* Increment here, and at the end of switch, results in skipping */
            break;
          }
          TRACE_DBG_INFO("Key is not pressed");
          break;
        case CHIP8_CMD_KEY_RELEASED: /* 0xExA1, Skip next instruction if REG_Vx==released key */
          TRACE_INSTR(TXT_INSTRUCTION_0xERA1,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX);   // TODO: check if key works
          if(!(pEmulator->tagKeyboard.ulKeyStates&(1<<REG_VX(tRegX))))
          {
            TRACE_DBG_INFO("Key is released, skipping next instruction");
            REG_PROGRAMCOUNTER_INCREMENT(); /* Increment here, and at the end of switch, results in skipping */
            break;
          }
          TRACE_DBG_INFO("Key is not released");
          break;
        default:
          return(ERR_INSTRUCTION_UNKNOWN);
      }
      break;
    case 0xF000: /* Misc stuff */
      tRegX=(tOPCode&0x0F00)>>8;
      switch((tOPCode&0x00FF))
      {
        case CHIP8_CMD_DELAY_TIMER_GET: /* 0xFx07, Set REG_Vx = Delay timer value */
          REG_VX(tRegX)=REG_TMRDEL;
          TRACE_INSTR(TXT_INSTRUCTION_0xFR07,REG_PC,pEmulator->tCurrOPCode,tRegX,REG_VX(tRegX),tRegX);
          break;
        case CHIP8_CMD_WAIT_FOR_KEY: /* 0xFx0A, wait until a Key is pressed, store it in REG_Vx */
          TRACE_INSTR(TXT_INSTRUCTION_0xFR0A,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX);
          if(iInstruction_GetPressedKey_m(&pEmulator->tagKeyboard,
                                          &REG_VX(tRegX)))
            return(ERR_NONE); /* If no key is pressed, return and call this again */
          break;
        case CHIP8_CMD_DELAY_TIMER_SET: /* 0xFx15, Set DelayTimer= REG_Vx */
          TRACE_INSTR(TXT_INSTRUCTION_0xFR15,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          REG_TMRDEL=REG_VX(tRegX);
          break;
        case CHIP8_CMD_SOUND_TIMER_SET: /* 0xFx18, Set SoundTimer = REG_Vx */
          TRACE_INSTR(TXT_INSTRUCTION_0xFR18,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          REG_TMRSND=REG_VX(tRegX); /* Other thread will start sound playback */
          break;
        case CHIP8_CMD_I_ADD_VX: /* 0xFx1E, Increment I by the val of REG_Vx */
          TRACE_INSTR(TXT_INSTRUCTION_0xFR1E,REG_PC,pEmulator->tCurrOPCode,tRegX,REG_I,tRegX,REG_VX(tRegX));
          REG_I+=REG_VX(tRegX);
          break;
        case CHIP8_CMD_I_SPRITE_GET:  /* 0xFx29, Set I to the address from sprite of val REG_Vx */
          if((tOPCode=OFF_ADDR_FONT_START+(5*REG_VX(tRegX)))>OFF_ADDR_FONT_END)
          {
            TRACE_DBG_ERROR_VARG("Can't set REG_I to Sprite, Number 0x%.2X out of bounds (0-F allowed)",REG_VX(tRegX));
            return(ERR_FONT_OUT_OF_INDEX);
          }
          TRACE_INSTR(TXT_INSTRUCTION_0xFR29,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          TRACE_DBG_INFO_VARG("Setting I on Pos for Sprite '%X'@0x%.4X",REG_VX(tRegX),tOPCode);
          REG_I=tOPCode;
          break;
        case CHIP8_CMD_VX_GET_BCD: /* 0xFx33, Store BCD val of REG_Vx in REG_I-REG_I+2 */
          TRACE_INSTR(TXT_INSTRUCTION_0xFR33,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          TRACE_DBG_INFO_VARG("BCD Reg_V%X -> I",tRegX);
          if(!REG_VX(tRegX))
          {
            TRACE_DBG_INFO("Register=0, set mem[I]-mem[I+2] to 0");
            tagEmulator_g.taChipMemory[REG_I]=0;
            tagEmulator_g.taChipMemory[REG_I+1]=0;
            tagEmulator_g.taChipMemory[REG_I+2]=0;
          }
          else
          {
            tagEmulator_g.taChipMemory[REG_I]=REG_VX(tRegX)/100;
            tagEmulator_g.taChipMemory[REG_I+1]=(REG_VX(tRegX)%100)/10;
            tagEmulator_g.taChipMemory[REG_I+2]=(REG_VX(tRegX)%100)%10;
            TRACE_DBG_INFO_VARG("Set mem[I]= 0x%.2X 0x%.2X 0x%.2X",
                                tagEmulator_g.taChipMemory[REG_I],
                                tagEmulator_g.taChipMemory[REG_I+1],
                                tagEmulator_g.taChipMemory[REG_I+2]);
          }
          break;
        case CHIP8_CMD_REGISTERS_STORE: /* 0xFx55, Store register V0-Vx in memory pointed by REG_I */
          TRACE_INSTR(TXT_INSTRUCTION_0xFR55,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX);
          TRACE_DBG_INFO_VARG("Storing Register V0-V%X in memory @0x%.4X-0x%.4X",
                              tRegX,
                              REG_I,
                              REG_I+tRegX);

          if((!MEM_JMP_ADDR_VALID(REG_I)) ||
             (REG_I+tRegX>OFF_ADDR_USER_END))
          {
            TRACE_DBG_ERROR("Can't store Registers, memory would overflow");
            return(ERR_MEM_WOULD_OVERFLOW);
          }
          for(tOPCode=0;tOPCode<=tRegX;++tOPCode)
          {
            tagEmulator_g.taChipMemory[REG_I+tOPCode]=REG_VX(tOPCode);
          }
          break;
        case CHIP8_CMD_REGISTERS_READ: /* 0xFx65, Read Values for Registers V0-Vx from memory pointed by REG_I */
          TRACE_INSTR(TXT_INSTRUCTION_0xFR65,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX);
          TRACE_DBG_INFO_VARG("Reading Register V0-V%X from memory @0x%.4X-0x%.4X",
                              tRegX,
                              REG_I,
                              REG_I+tRegX);

          if((!MEM_JMP_ADDR_VALID(REG_I)) ||
             (REG_I+tRegX>OFF_ADDR_USER_END))
          {
            TRACE_DBG_ERROR("Can't read memory, invalid address");
            return(ERR_MEM_WOULD_OVERFLOW);
          }
          for(tOPCode=0;tOPCode<=tRegX;++tOPCode)
          {
            REG_VX(tOPCode)=tagEmulator_g.taChipMemory[REG_I+tOPCode];
          }
          break;
        default:
          return(ERR_INSTRUCTION_UNKNOWN);
      }
      break;
    default:
      return(ERR_INSTRUCTION_UNKNOWN);
  }
  REG_PROGRAMCOUNTER_INCREMENT();
  return(ERR_NONE);
}

#if 0

INLINE_FCT int iInstruction_Sprite(byte *pMem,
                                   byte tRegX,
                                   byte tRegY,
                                   byte tBytes) /* 0xDxyn */   // TODO: simplify this function
{
  unsigned int uiIndex;
  byte tScreenTmp;
  byte tPosX=REG_VX(tRegX);
  byte tPosY=REG_VX(tRegY);
  TRACE_DBG_INFO_VARG("Drawing %u bytes from 0x%.4X @Pos(%u/%u)",tBytes,REG_I,tPosX,tPosY);
  if(REG_I+tBytes>OFF_ADDR_USER_END)
  {
    TRACE_DBG_ERROR("Drawing cancelled, memory would overflown");
    return(ERR_MEM_WOULD_OVERFLOW);
  }
  if(tPosX>CHIP8_SCREEN_WIDTH-1)
  {
    TRACE_DBG_ERROR_VARG("Screen overflow in X-Direction (%u allowed, is: %u)",CHIP8_SCREEN_WIDTH-1,tPosX);
    return(ERR_SCREEN_X_OVERFLOW);
  }
  REG_VF=0;
  for(uiIndex=0;uiIndex<tBytes;++uiIndex)
  {
    if(tPosY+uiIndex>CHIP8_SCREEN_HEIGHT-1)
    {
      TRACE_DBG_INFO_VARG("Ignoring Y-Overflow Drawing(Allowed: %u, is: %u)",CHIP8_SCREEN_HEIGHT-1,tPosY+uiIndex);
      break;
    }
    tScreenTmp=MEM_SCREEN_BYTE(tPosX,tPosY+uiIndex); /* Save last byte */
    MEM_SCREEN_BYTE(tPosX,tPosY+uiIndex)^=(pMem[REG_I+uiIndex]>>((tPosX%8)));

    if((!REG_VF) &&
       (SCREEN_CHECK_PIXEL_EREASED(MEM_SCREEN_BYTE(tPosX,tPosY+uiIndex),tScreenTmp)))
    {
//    printf("VF set to 1!\n");// TODO: set VF=1 if XOR causes pixel to be ereased, else set VF=0
//    getchar();
      REG_VF=1;
    }
    /* Check for Screen overflow */
    if((tPosX+uiIndex>CHIP8_SCREEN_WIDTH-9))
    {
      if(tPosY+tBytes+uiIndex>=CHIP8_SCREEN_HEIGHT)
        TRACE_DBG_INFO_VARG("(Wraparound) Ignoring Y-Overflow Drawing: [0][%u (%u allowed)]",tPosY+tBytes+uiIndex,CHIP8_SCREEN_HEIGHT-1);
      else
      {
        MEM_SCREEN_BYTE(0,tPosY+tBytes+uiIndex)^=(pMem[REG_I+uiIndex])<<(8-(tPosX%8));
        if((!REG_VF) &&
           (SCREEN_CHECK_PIXEL_EREASED(MEM_SCREEN_BYTE(tPosX,tPosY+uiIndex),tScreenTmp)))
        {
//        printf("VF set to 1!\n");// TODO: set VF=1 if XOR causes pixel to be ereased, else set VF=0
//        getchar();
          REG_VF=1;
        }
        TRACE_DBG_INFO_VARG("(Wraparound) MEM_SCREEN[%u][%u]=0x%.2X / [0][%u]=0x%.2X @0x%.2X(=0x%.2X)",
                            tPosX/8,
                            tPosY+uiIndex,
                            MEM_SCREEN_BYTE(tPosX,tPosY+uiIndex),
                            tPosY+tBytes+uiIndex,
                            MEM_SCREEN_BYTE(0,tPosY+tBytes+uiIndex),
                            REG_I+uiIndex,
                            pMem[REG_I+uiIndex]);
      }
    }
    else
    {
      MEM_SCREEN_BYTE(tPosX+8,tPosY+uiIndex)^=(pMem[REG_I+uiIndex])<<(8-(tPosX%8));
      if((!REG_VF) &&
         (SCREEN_CHECK_PIXEL_EREASED(MEM_SCREEN_BYTE(tPosX,tPosY+uiIndex),tScreenTmp)))
      {
//      printf("VF set to 1!\n");// TODO: set VF=1 if XOR causes pixel to be ereased, else set VF=0
//      getchar();
        REG_VF=1;
      }
      TRACE_DBG_INFO_VARG("MEM_SCREEN[%u][%u]=0x%.2X / [%u][%u]=0x%.2X @0x%.2X(=0x%.2X)",
                          tPosX/8,
                          tPosY+uiIndex,
                          MEM_SCREEN_BYTE(tPosX,tPosY+uiIndex),
                          tPosX/8+1,
                          tPosY+uiIndex,
                          MEM_SCREEN_BYTE(tPosX+8,tPosY+uiIndex),
                          REG_I+uiIndex,
                          pMem[REG_I+uiIndex]);
    }

  }
  return(ERR_NONE);
}

#else

INLINE_FCT int iInstruction_Sprite(byte *pMem,
                                   byte tPosX,
                                   byte tPosY,
                                   byte tBytes) /* 0xDxyn */
{
  unsigned int uiIndex;
  word tLastVal;
  word tCurrVal;
  TRACE_DBG_INFO_VARG("Drawing %u bytes @[%u][%u] (from mem 0x%.4X)",tBytes,tPosX,tPosY,REG_I);

  /* Check memory for overflow */
  if(REG_I+tBytes>OFF_ADDR_USER_END)
  {
    TRACE_DBG_ERROR_VARG("REG_I(0x%.4X)+tBytes(0x%.2X)=0x%.4X > OFF_ADDR_USER_END(0x%.4X), cancel drawing, Memory would overflow",
                         REG_I,
                         tBytes,
                         REG_I+tBytes,
                         OFF_ADDR_USER_END);
    return(ERR_MEM_WOULD_OVERFLOW);
  }
  /* Check if dest coordinates are okay/ on screen  */
  if((tPosX>CHIP8_SCREEN_WIDTH-1) || (tPosY>CHIP8_SCREEN_HEIGHT-1))
  {
    TRACE_DBG_ERROR_VARG("Drawing Position(%u-%u/%u-%u) out of Screen(%u/%u), cancel drawing",
                         tPosX,
                         tPosX+8,
                         tPosY,
                         tPosY+tBytes,
                         CHIP8_SCREEN_WIDTH-1,
                         CHIP8_SCREEN_HEIGHT-1);
    return(ERR_SCREEN_X_OVERFLOW);
  }
  REG_VF=0;
  for(uiIndex=0;uiIndex<tBytes;++uiIndex)
  {
    tLastVal=0;
    tCurrVal=0;
    if(tPosY+uiIndex>CHIP8_SCREEN_HEIGHT-1)
    {
      TRACE_DBG_INFO("Cancel drawing, Y-Axis Overflow");
      break;
    }
    tLastVal|=MEM_SCREEN_BYTE(tPosX,tPosY+uiIndex);
//  printf("BYTE[%u][%u](index=%u)\n",
//         tPosX,
//         tPosY+uiIndex,
//         SCREEN_CALC_INDEX(tPosX,tPosY+uiIndex));
//
//  printf("BEFORE UPDATE: BYTE VAL:0x%.2X, tLastVal FIRST BYTE=0x%.4X\n",
//         MEM_SCREEN_BYTE(tPosX,tPosY+uiIndex),
//         tLastVal);
    if((tPosX+8u<CHIP8_SCREEN_WIDTH) && (tPosX&0x7))
    {
      tLastVal|=(MEM_SCREEN_BYTE(tPosX+8,tPosY+uiIndex)<<8);
//    printf("tLastVal with 2ND BYTE: 0x%.4X\n",tLastVal);
    }
    tCurrVal|=(pMem[REG_I+uiIndex])>>(tPosX&0x7);
//  printf("pMem[0x%.4X]=0x%.2X, curr: 0x%.4X\n",
//         REG_I+uiIndex,
//         pMem[REG_I+uiIndex],
//         tCurrVal);
    MEM_SCREEN_BYTE(tPosX,tPosY+uiIndex)^=(tCurrVal);

//  printf("AFTER UPDATE: BYTE VAL:0x%.2X, tCurrVal FIRST BYTE=0x%.4X\n",
//         MEM_SCREEN_BYTE(tPosX,tPosY+uiIndex),
//         tCurrVal);

    if((tPosX+8u<CHIP8_SCREEN_WIDTH) && (tPosX&0x7))
    {
      tCurrVal|=(pMem[REG_I+uiIndex] & (0xFF>>(8-(tPosX&0x7)))) << (16-(tPosX&0x7));
      MEM_SCREEN_BYTE(tPosX+8u,tPosY+uiIndex)^=tCurrVal>>8;
//    printf("2nd(+8) BYTE VAL:0x%.2X, tCurrVal=0x%.4X\n",
//           MEM_SCREEN_BYTE(tPosX+8,tPosY+uiIndex),
//           tCurrVal);
    }
    if((!REG_VF) &&
       ((tCurrVal&tLastVal)>0))
    {
//    printf("Pixel was ereased, set REG_VF to 1\n");
      REG_VF=1;
    }
  }
  return(ERR_NONE);
}

#endif

INLINE_FCT int iInstruction_GetPressedKey_m(TagKeyboard *ptagKeyboard,
                                            byte *pReg)
{
  if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_0)
    *pReg=0x0;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_1)
    *pReg=0x1;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_2)
    *pReg=0x2;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_3)
    *pReg=0x3;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_4)
    *pReg=0x4;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_5)
    *pReg=0x5;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_6)
    *pReg=0x6;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_7)
    *pReg=0x7;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_8)
    *pReg=0x8;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_9)
    *pReg=0x9;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_A)
    *pReg=0xA;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_B)
    *pReg=0xB;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_C)
    *pReg=0xC;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_D)
    *pReg=0xD;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_E)
    *pReg=0xE;
  else if(ptagKeyboard->ulKeyStates&EMU_KEY_MASK_F)
    *pReg=0xF;
  else
    return(-1);

  return(0);
}

INLINE_FCT void chip8_MemoryInit_m(byte *pMem,
                                   word wStartAddress)
{
  /* Reset whole memory to 0 */
  memset(pMem,0,OFF_MEM_SIZE);
  /* Copy Fonts to memory */
  memcpy(&pMem[OFF_ADDR_FONT_START],tChip8_font_m,OFF_ADDR_FONT_END-OFF_ADDR_FONT_START);
  /**
   * Init Registers
   */
  /* Set Programcounter to default startaddress */
  REG_PC        = wStartAddress;
  REG_PTR_STACK = OFF_ADDR_STACK_START;
#ifdef CHIP8_TEST_VALUES
  /* Add some values for testing purposes */
  REG_V1=26;
  REG_V2=31;

  pMem[0x200]=0xD1;
  pMem[0x201]=0x21;
  pMem[0x202]=0xD1;
  pMem[0x203]=0x21;
  REG_I=0x300;
  pMem[0x300]=0x81;
  pMem[0x301]=0x0F;
  pMem[0x302]=0xF0;
  pMem[0x303]=0x00;
  pMem[0x304]=0xAA;
  pMem[0x305]=0xFF;
  pMem[0x306]=0xFF;
  pMem[0x307]=0xFF;

#endif
}
