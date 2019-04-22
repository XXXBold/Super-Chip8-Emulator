#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "chip8_emulator.h"
#include "chip8_global.h"
#include "chip8_extern.h"

//#define CHIP8_TEST_VALUES /* Enable this for testing purposes */

enum
{
  /* CMDs starting with 0x00 */
  CHIP8_CMD_CLS              =0x00E0,
  CHIP8_CMD_RET              =0x00EE,
  SUCHIP_CMD_SCROLL_RIGHT    =0x00FB,
  SUCHIP_CMD_SCROLL_LEFT     =0x00FC,
  SUCHIP_CMD_EXIT            =0x00FD,
  SUCHIP_CMD_DIS_EX_SCREEN   =0x00FE,
  SUCHIP_CMD_EN_EX_SCREEN    =0x00FF,

  /* CMDs starting with 0x8x */
  CHIP8_CMD_MATH_MOV         =0x0,
  CHIP8_CMD_MATH_OR          =0x1,
  CHIP8_CMD_MATH_AND         =0x2,
  CHIP8_CMD_MATH_XOR         =0x3,
  CHIP8_CMD_MATH_ADD         =0x4,
  CHIP8_CMD_MATH_SUB         =0x5,
  CHIP8_CMD_MATH_SHR         =0x6,
  CHIP8_CMD_MATH_SUBN        =0x7,
  CHIP8_CMD_MATH_SHL         =0xE,

  /* CMDs starting with 0xEx */
  CHIP8_CMD_KEY_PRESSED      =0x009E,
  CHIP8_CMD_KEY_RELEASED     =0x00A1,

  /* CMDS Starting with 0xFx */
  CHIP8_CMD_DELAY_TIMER_GET  =0x0007,
  CHIP8_CMD_WAIT_FOR_KEY     =0x000A,
  CHIP8_CMD_DELAY_TIMER_SET  =0x0015,
  CHIP8_CMD_SOUND_TIMER_SET  =0x0018,
  CHIP8_CMD_I_ADD_VX         =0x001E,
  CHIP8_CMD_I_SPRITE_GET     =0x0029,
  SUCHIP_CMD_I_XSPRITE_GET   =0x0030,
  CHIP8_CMD_VX_GET_BCD       =0x0033,
  CHIP8_CMD_REGISTERS_STORE  =0x0055,
  CHIP8_CMD_REGISTERS_READ   =0x0065,
  SUCHIP_CMD_STORE_TO_RPL    =0x0075,
  SUCHIP_CMD_READ_FROM_RPL   =0x0085,
};

#define TXT_INSTRUCTION_GENERAL "[0x%.4X] 0x%.4X"

/* Text for each OP Code */
#define TXT_OPC_ASM_0x00CN "SCD 0x%.2X" /* Superchip-Instruction */
#define TXT_OPC_ASM_0x00E0 "CLS"
#define TXT_OPC_ASM_0x00EE "RET"
#define TXT_OPC_ASM_0x00FB "SCR"        /* Superchip-Instruction */
#define TXT_OPC_ASM_0x00FC "SCL"        /* Superchip-Instruction */
#define TXT_OPC_ASM_0x00FD "EXIT"       /* Superchip-Instruction */
#define TXT_OPC_ASM_0x00FE "LOW"        /* Superchip-Instruction */
#define TXT_OPC_ASM_0x00FF "HIGH"       /* Superchip-Instruction */
#define TXT_OPC_ASM_0x1NNN "JP @0x%.2X"
#define TXT_OPC_ASM_0x2NNN "CALL @0x%.2X"
#define TXT_OPC_ASM_0x3XKK "SEQ V%X,0x%.2X"
#define TXT_OPC_ASM_0x4XKK "SNEQ V%X,0x%.2X"
#define TXT_OPC_ASM_0x5XY0 "SEQ V%X,V%X"
#define TXT_OPC_ASM_0x6XKK "LD V%X,0x%.2X"
#define TXT_OPC_ASM_0x7XKK "ADD V%X,0x%.2X"
#define TXT_OPC_ASM_0x8XY0 "MOV V%X,V%X"
#define TXT_OPC_ASM_0x8XY1 "OR V%X,V%X"
#define TXT_OPC_ASM_0x8XY2 "AND V%X,V%X"
#define TXT_OPC_ASM_0x8XY3 "XOR V%X,V%X"
#define TXT_OPC_ASM_0x8XY4 "ADD V%X,V%X"
#define TXT_OPC_ASM_0x8XY5 "SUB V%X,V%X"
#define TXT_OPC_ASM_0x8XY6 "SHR V%X"
#define TXT_OPC_ASM_0x8XY7 "SUBN V%X,V%X"
#define TXT_OPC_ASM_0x8XYE "SHL V%X"
#define TXT_OPC_ASM_0x9XY0 "SNEQ V%X,V%X"
#define TXT_OPC_ASM_0xANNN "MVI 0x%.2X"
#define TXT_OPC_ASM_0xBNNN "JMI 0x%.2X"
#define TXT_OPC_ASM_0xCXKK "RAND V%X,0x%.2X"
#define TXT_OPC_ASM_0xDXY0 "XSPRITE V%X,V%X" /* Superchip-Instruction */
#define TXT_OPC_ASM_0xDXYN "SPRITE V%X,V%X,0x%.2X"
#define TXT_OPC_ASM_0xEX9E "SKPR V%x"
#define TXT_OPC_ASM_0xEXA1 "SKUP V%X"
#define TXT_OPC_ASM_0xFX07 "GDELAY V%X"
#define TXT_OPC_ASM_0xFX0A "KEY V%X"
#define TXT_OPC_ASM_0xFX15 "SDELAY V%X"
#define TXT_OPC_ASM_0xFX18 "SSOUND V%X"
#define TXT_OPC_ASM_0xFX1E "ADI V%X"
#define TXT_OPC_ASM_0xFX29 "FONT V%X"
#define TXT_OPC_ASM_0xFX30 "XFONT, V%X" /* Superchip-Instruction */
#define TXT_OPC_ASM_0xFX33 "BCD V%X"
#define TXT_OPC_ASM_0xFX55 "STR V0-V%X"
#define TXT_OPC_ASM_0xFX65 "LDR V0-V%X"
#define TXT_OPC_ASM_0xFX75 "LD R, V%X"  /* Superchip-Instruction */
#define TXT_OPC_ASM_0xFX85 "LD V%X, R"  /* Superchip-Instruction */

#define TXT_OPC_DESC_0x00CN "Scroll down 0x%.2X Lines"               /* Superchip-Instruction */
#define TXT_OPC_DESC_0x00E0 "Clear Screen"
#define TXT_OPC_DESC_0x00EE "Return from subroutine"
#define TXT_OPC_DESC_0x00FB "Scroll 4 pixels right"                  /* Superchip-Instruction */
#define TXT_OPC_DESC_0x00FC "Scroll 4 pixels left"                   /* Superchip-Instruction */
#define TXT_OPC_DESC_0x00FD "Exit Program"                           /* Superchip-Instruction */
#define TXT_OPC_DESC_0x00FE "Disable extended screen mode"           /* Superchip-Instruction */
#define TXT_OPC_DESC_0x00FF "Enable extended screen mode (128 x 64)" /* Superchip-Instruction */
#define TXT_OPC_DESC_0x1NNN "Jump to Address 0x%.2X"
#define TXT_OPC_DESC_0x2NNN "Jump to Subroutine at Address 0x%.2X (Stack %u->+1/%u)"
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
#define TXT_OPC_DESC_0x8XY6 "Shift right V%X(=0x%.2X)"
#define TXT_OPC_DESC_0x8XY7 "Subtract Reverseorder V%X(=0x%.2X) - V%X(=0x%.2X)"
#define TXT_OPC_DESC_0x8XYE "Shift left V%X(=0x%.2X)"
#define TXT_OPC_DESC_0x9XY0 "Skip Next Instruction if V%X(=0x%.2X)!=V%X(=0x%.2X)"
#define TXT_OPC_DESC_0xANNN "Set Reg I =0x%.2X"
#define TXT_OPC_DESC_0xBNNN "Jump to V0(=0x%.2X) + nnn(=0x%.2X)"
#define TXT_OPC_DESC_0xCXKK "Random Number AND 0x%.2X -> V%X"
#define TXT_OPC_DESC_0xDXY0 "Draw a 16x16 Sprite @ V%X(=0x%.2X)/V%X(=0x%.2X)" /* Superchip-Instruction */
#define TXT_OPC_DESC_0xDXYN "Draw 0x%.2X bytes @ V%X(=0x%.2X)/V%X(=0x%.2X)"
#define TXT_OPC_DESC_0xEX9E "Skip if Key in V%X(='%X') is pressed"
#define TXT_OPC_DESC_0xEXA1 "SKip if key in V%X(='%X') is not pressed"
#define TXT_OPC_DESC_0xFX07 "Get delay timer value(=0x%.2X) -> V%X"
#define TXT_OPC_DESC_0xFX0A "Wait for Key and store -> V%X"
#define TXT_OPC_DESC_0xFX15 "Set Delay Timer value <- V%X(=0x%.2X)"
#define TXT_OPC_DESC_0xFX18 "Set Sound Timer <- V%X(=0x%.2X)"
#define TXT_OPC_DESC_0xFX1E "Increment Reg I(=0x%.3X) by V%X(=0x%.2X)"
#define TXT_OPC_DESC_0xFX29 "Set Reg I to Sprite in V%X(No. 0x%.2X)"
#define TXT_OPC_DESC_0xFX30 "Set Reg I to 10-byte Sprite in V%X(No. 0x%.2X)"  /* Superchip-Instruction */
#define TXT_OPC_DESC_0xFX33 "Store BCD of V%X(=0x%.2X) -> Reg I"
#define TXT_OPC_DESC_0xFX55 "Store registers V0-V%X on Memory pointed by Reg I"
#define TXT_OPC_DESC_0xFX65 "Read from Memory pointed by Reg I to Registers V0-V%X"
#define TXT_OPC_DESC_0xFX75 "Store Register 0-%X to RPL (HP-Calculator)"       /* Superchip-Instruction */
#define TXT_OPC_DESC_0xFX85 "Read Register 0-%X from RPL (HP-Calculator)"      /* Superchip-Instruction */


#define TXT_INSTRUCTION_0x00CN TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x00CN ": " TXT_OPC_DESC_0x00CN /* Superchip-Instruction */
#define TXT_INSTRUCTION_0x00E0 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x00E0 ": " TXT_OPC_DESC_0x00E0
#define TXT_INSTRUCTION_0x00EE TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x00EE ": " TXT_OPC_DESC_0x00EE
#define TXT_INSTRUCTION_0x00FB TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x00FB ": " TXT_OPC_DESC_0x00FB /* Superchip-Instruction */
#define TXT_INSTRUCTION_0x00FC TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x00FC ": " TXT_OPC_DESC_0x00FC /* Superchip-Instruction */
#define TXT_INSTRUCTION_0x00FD TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x00FD ": " TXT_OPC_DESC_0x00FD /* Superchip-Instruction */
#define TXT_INSTRUCTION_0x00FE TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x00FE ": " TXT_OPC_DESC_0x00FE /* Superchip-Instruction */
#define TXT_INSTRUCTION_0x00FF TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x00FF ": " TXT_OPC_DESC_0x00FF /* Superchip-Instruction */
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
#define TXT_INSTRUCTION_0x8XY6 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x8XY6 ": " TXT_OPC_DESC_0x8XY6
#define TXT_INSTRUCTION_0x8XY7 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x8XY7 ": " TXT_OPC_DESC_0x8XY7
#define TXT_INSTRUCTION_0x8XYE TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x8XYE ": " TXT_OPC_DESC_0x8XYE
#define TXT_INSTRUCTION_0x9XY0 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0x9XY0 ": " TXT_OPC_DESC_0x9XY0
#define TXT_INSTRUCTION_0xANNN TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xANNN ": " TXT_OPC_DESC_0xANNN
#define TXT_INSTRUCTION_0xBNNN TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xBNNN ": " TXT_OPC_DESC_0xBNNN
#define TXT_INSTRUCTION_0xCXKK TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xCXKK ": " TXT_OPC_DESC_0xCXKK
#define TXT_INSTRUCTION_0xDXY0 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xDXY0 ": " TXT_OPC_DESC_0xDXY0 /* Superchip-Instruction */
#define TXT_INSTRUCTION_0xDXYN TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xDXYN ": " TXT_OPC_DESC_0xDXYN
#define TXT_INSTRUCTION_0xEX9E TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xEX9E ": " TXT_OPC_DESC_0xEX9E
#define TXT_INSTRUCTION_0xEXA1 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xEXA1 ": " TXT_OPC_DESC_0xEXA1
#define TXT_INSTRUCTION_0xFX07 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFX07 ": " TXT_OPC_DESC_0xFX07
#define TXT_INSTRUCTION_0xFX0A TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFX0A ": " TXT_OPC_DESC_0xFX0A
#define TXT_INSTRUCTION_0xFX15 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFX15 ": " TXT_OPC_DESC_0xFX15
#define TXT_INSTRUCTION_0xFX18 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFX18 ": " TXT_OPC_DESC_0xFX18
#define TXT_INSTRUCTION_0xFX1E TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFX1E ": " TXT_OPC_DESC_0xFX1E
#define TXT_INSTRUCTION_0xFX29 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFX29 ": " TXT_OPC_DESC_0xFX29
#define TXT_INSTRUCTION_0xFX30 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFX30 ": " TXT_OPC_DESC_0xFX30 /* Superchip-Instruction */
#define TXT_INSTRUCTION_0xFX33 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFX33 ": " TXT_OPC_DESC_0xFX33
#define TXT_INSTRUCTION_0xFX55 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFX55 ": " TXT_OPC_DESC_0xFX55
#define TXT_INSTRUCTION_0xFX65 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFX65 ": " TXT_OPC_DESC_0xFX65
#define TXT_INSTRUCTION_0xFX75 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFX75 ": " TXT_OPC_DESC_0xFX75 /* Superchip-Instruction */
#define TXT_INSTRUCTION_0xFX85 TXT_INSTRUCTION_GENERAL ": " TXT_OPC_ASM_0xFX85 ": " TXT_OPC_DESC_0xFX85 /* Superchip-Instruction */

#define MEM_JMP_ADDR_VALID(addr) (((addr<OFF_ADDR_USER_START) || (addr>OFF_ADDR_USER_END))?0:1)
#define SCREEN_CHECK_PIXEL_EREASED(curr,last) ((((curr)&(last))<(last))?1:0)
#define MEM_ADDRESS_AS_WORD(index) *(word*)(&tagEmulator_g.taChipMemory[index])

#define REG_STACKPTR_1UP(stptr) (stptr)-=sizeof(word)
#define REG_STACKPTR_1DOWN(stptr) (stptr)+=sizeof(word)
#define REG_PROGRAMCOUNTER_INCREMENT() REG_PC+=sizeof(word)

#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
  #define MEM_SCREEN_BIT_CHECK(win,x,y) ((win).taScreenBuffer[(((win).info.exModeOn)?SUCHIP_SCREEN_INDEX(x,y):CHIP8_SCREEN_INDEX(x,y))]&(0x80>>((x)%8)))
  #define MEM_SCREEN_BYTE(win,x,y)       (win).taScreenBuffer[(((win).info.exModeOn)?SUCHIP_SCREEN_INDEX(x,y):CHIP8_SCREEN_INDEX(x,y))]
#else
  #define MEM_SCREEN_BIT_CHECK(win,x,y) ((win).taScreenBuffer[CHIP8_SCREEN_INDEX(x,y)]&(0x80>>((x)%8)))
  #define MEM_SCREEN_BYTE(win,x,y)       (win).taScreenBuffer[CHIP8_SCREEN_INDEX(x,y)]
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */


INLINE_PROT byte tChip8_GetRand_m(void);

INLINE_PROT void vChip8_MemoryInit_m(byte *pMem,
                                     word WStartAddress);

INLINE_PROT int iChip8_GetPressedKey_m(TagKeyboard *ptagKeyboard,
                                       byte *pReg);

INLINE_PROT void vChip8_ShiftRight_m(byte *pData,
                                     unsigned int uiDataLength,
                                     int iShift);
INLINE_PROT void vChip8_ShiftLeft_m(byte *pData,
                                    unsigned int uiDataLength,
                                    int iShift);

INLINE_PROT int iChip8_Sprite_m(TagEmulator *pEmulator,
                                byte tRegX,
                                byte tRegY,
                                byte tBytes); /* 0xDxyn */

#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
INLINE_PROT int iSuperchip_Sprite_m(TagEmulator *pEmulator,
                                    byte tPosX,
                                    byte tPosY); /* 0xDxy0 */
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS*/

int chip8_Process(TagEmulator *pEmulator);

static const byte tChip8_Font_m[]=
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

#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
static const byte tSuperChip_Font_m[]=
{
  0x7C, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7C, 0x00, /* 0 */
  0x08, 0x18, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3C, 0x00, /* 1 */
  0x7C, 0x82, 0x02, 0x02, 0x04, 0x18, 0x20, 0x40, 0xFE, 0x00, /* 2 */
  0x7C, 0x82, 0x02, 0x02, 0x3C, 0x02, 0x02, 0x82, 0x7C, 0x00, /* 3 */
  0x84, 0x84, 0x84, 0x84, 0xFE, 0x04, 0x04, 0x04, 0x04, 0x00, /* 4 */
  0xFE, 0x80, 0x80, 0x80, 0xFC, 0x02, 0x02, 0x82, 0x7C, 0x00, /* 5 */
  0x7C, 0x82, 0x80, 0x80, 0xFC, 0x82, 0x82, 0x82, 0x7C, 0x00, /* 6 */
  0xFE, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x20, 0x20, 0x00, /* 7 */
  0x7C, 0x82, 0x82, 0x82, 0x7C, 0x82, 0x82, 0x82, 0x7C, 0x00, /* 8 */
  0x7C, 0x82, 0x82, 0x82, 0x7E, 0x02, 0x02, 0x82, 0x7C, 0x00, /* 9 */
  0x10, 0x28, 0x44, 0x82, 0x82, 0xFE, 0x82, 0x82, 0x82, 0x00, /* A */
  0xFC, 0x82, 0x82, 0x82, 0xFC, 0x82, 0x82, 0x82, 0xFC, 0x00, /* B */
  0x7C, 0x82, 0x80, 0x80, 0x80, 0x80, 0x80, 0x82, 0x7C, 0x00, /* C */
  0xFC, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0xFC, 0x00, /* D */
  0xFE, 0x80, 0x80, 0x80, 0xF8, 0x80, 0x80, 0x80, 0xFE, 0x00, /* E */
  0xFE, 0x80, 0x80, 0x80, 0xF8, 0x80, 0x80, 0x80, 0x80, 0x00, /* F */
};
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */

TagEmulator tagEmulator_g;

int chip8_Init(int winPosX,
               int winPosY,
               unsigned char winScaleFactor,
               const unsigned char keyMap[EMU_KEY_COUNT],
               pCallbackFunc cbFunc,
               unsigned int callbackEvents)
{
  TRACE_DBG_INFO("Init Chip-8 Emulator...");
  tagEmulator_g.tagWindow.info.iPosX=winPosX;
  tagEmulator_g.tagWindow.info.iPosY=winPosY;
  tagEmulator_g.tagWindow.info.scale=(winScaleFactor)?winScaleFactor:EMU_SCREEN_DEFAULT_SCALE;

  tagEmulator_g.tagThrdEmu.procFunc=chip8_Process;
  tagEmulator_g.tagThrdEmu.usrCBFunc=cbFunc;
  tagEmulator_g.tagThrdEmu.usrCBEvents=callbackEvents;

  tagEmulator_g.tagPlaySound.uiFreqSoundHz=CHIP8_SOUND_FREQ;

  /* Reset all updateSettings flags */
  tagEmulator_g.updateSettings.emu_ExecSpeed=0;
  tagEmulator_g.updateSettings.keyboard_Keymap=0;
  tagEmulator_g.updateSettings.screen_Clear=0;
  tagEmulator_g.updateSettings.screen_ExMode_Dis=0;
  tagEmulator_g.updateSettings.screen_ExMode_En=0;
  tagEmulator_g.updateSettings.screen_Hide=0;
  tagEmulator_g.updateSettings.screen_Pos=0;
  tagEmulator_g.updateSettings.screen_Scale=0;
  tagEmulator_g.updateSettings.screen_Show=0;
  tagEmulator_g.updateSettings.emu_Pause=0;
  tagEmulator_g.updateSettings.emu_Quit=0;
  tagEmulator_g.updateSettings.emu_Run=0;

  if(keyMap)
  {
    TRACE_DBG_INFO("Keymap: using custom");
    memcpy(tagEmulator_g.tagKeyboard.ucaUsrKeyMap,
           keyMap,
           EMU_KEY_COUNT);
  }
  else
  {
    TRACE_DBG_INFO("Keymap: using default");
    memset(tagEmulator_g.tagKeyboard.ucaUsrKeyMap,
           EMU_KEY_DEFAULT,
           EMU_KEY_COUNT);
  }
  /* Initialise RNG */
  srand((unsigned int)time(NULL));
  vChip8_MemoryInit_m(tagEmulator_g.taChipMemory,OFF_ADDR_USER_START);

  if(iChip8_External_Init_g(&tagEmulator_g)) /* Init external libraries */
  {
    TRACE_DBG_ERROR("iChip8_External_Init_g() failed");
    return(-1);
  }
  return(0);
}

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
    /* Check if already paused */
    if(tagEmulator_g.tagThrdEmu.eEmuState==EMU_STATE_PAUSE)
      return;
    tagEmulator_g.updateSettings.emu_Pause=1;
    while(tagEmulator_g.updateSettings.emu_Pause);
  }
  else
  {
    /* Check if already running */
    if(tagEmulator_g.tagThrdEmu.eEmuState==EMU_STATE_RUN)
      return;
    tagEmulator_g.updateSettings.screen_Show=1;
    tagEmulator_g.updateSettings.emu_Run=1;
    /* Wait for thread to update setting */
    while((tagEmulator_g.updateSettings.screen_Show) || (tagEmulator_g.updateSettings.emu_Run));
  }
}

void chip8_SetWindowScale(unsigned char windowScale)
{
  /* Return if emulator is not initialised yet */
  if(tagEmulator_g.tagThrdEmu.eEmuState==EMU_STATE_INACTIVE)
    return;

  if(tagEmulator_g.tagWindow.info.scale==windowScale)
    return;

  tagEmulator_g.tagWindow.info.scale=windowScale;
  tagEmulator_g.updateSettings.screen_Scale=1;

  /* Wait for thread to set window size */
  while(tagEmulator_g.updateSettings.screen_Scale)
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
  if((tagEmulator_g.tagWindow.info.iPosX==iPosX) &&
     (tagEmulator_g.tagWindow.info.iPosY==iPosY))
  {
    return;
  }
  tagEmulator_g.tagWindow.info.iPosX=iPosX;
  tagEmulator_g.tagWindow.info.iPosY=iPosY;
  tagEmulator_g.updateSettings.screen_Pos=1;
  /* Wait for thread to set window pos */
  while(tagEmulator_g.updateSettings.screen_Pos)
  {
    ;
  }
}

void chip8_SetWindowTitle(const char *windowTitle)
{
  strncpy(tagEmulator_g.tagWindow.info.caTitle,
          windowTitle,
          sizeof(tagEmulator_g.tagWindow.info.caTitle)-1);
  tagEmulator_g.tagWindow.info.caTitle[sizeof(tagEmulator_g.tagWindow.info.caTitle)-1]='\0';
  tagEmulator_g.updateSettings.screen_Title=1;
  while(tagEmulator_g.updateSettings.screen_Title)
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
  {
    tagEmulator_g.updateSettings.screen_Show=1;
    /* Wait for thread to set window visibility */
    while(tagEmulator_g.updateSettings.screen_Show);
  }
  else
  {
    tagEmulator_g.updateSettings.screen_Hide=1;
    /* Wait for thread to set window visibility */
    while(tagEmulator_g.updateSettings.screen_Hide);
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
  tagEmulator_g.updateSettings.emu_ExecSpeed=1;

  /* Wait for speed to get updated */
  while(tagEmulator_g.updateSettings.emu_ExecSpeed)
  {
    ;
  }
}

void chip8_SetKeymap(unsigned char newKeymap[EMU_KEY_COUNT])
{
  memcpy(tagEmulator_g.tagKeyboard.ucaUsrKeyMap,newKeymap,EMU_KEY_COUNT),

  tagEmulator_g.updateSettings.keyboard_Keymap=1;

  while(tagEmulator_g.updateSettings.keyboard_Keymap)
  {
    ;
  }
}

void chip8_EnableQuirks(unsigned int quirks)
{
  tagEmulator_g.uiQuirks=(quirks&(EMU_QUIRK_INCREMENT_I_ON_STORAGE|EMU_QUIRK_SHIFT_SOURCE_REG));
}

int chip8_LoadFile(const char *pcFilePath,
                   word tStartAddress)
{
  FILE *fp;
  byte taMemTemp[OFF_ADDR_USER_END-OFF_ADDR_USER_START+1];
  unsigned int uiReadSize;

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
  if((uiReadSize=fread(taMemTemp,1,sizeof(taMemTemp),fp))<1)
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
    TRACE_DBG_ERROR_VARG("chip8_LoadFile(): file \"%s\" is too big  (> 0x%X-0x%X)",pcFilePath,tStartAddress,OFF_ADDR_USER_END);
    fclose(fp);
    return(-1);
  }
  fclose(fp);
  vChip8_MemoryInit_m(tagEmulator_g.taChipMemory,tStartAddress);
  memcpy(&tagEmulator_g.taChipMemory[tStartAddress],taMemTemp,uiReadSize);
  tagEmulator_g.updateSettings.screen_ExMode_Dis=1;
  tagEmulator_g.updateSettings.screen_Clear=1;
  return(0);
}

void chip8_DumpScreen(void)
{
  int iIndex;
  int iIndexB;
  int iScreenWidth=(tagEmulator_g.tagWindow.info.exModeOn)?SUCHIP_SCREEN_WIDTH:CHIP8_SCREEN_WIDTH;
  int iScreenHeigth=(tagEmulator_g.tagWindow.info.exModeOn)?SUCHIP_SCREEN_HEIGHT:CHIP8_SCREEN_HEIGHT;
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
  char caTextBuffer[SUCHIP_SCREEN_WIDTH+3];
#else
  char caTextBuffer[CHIP8_SCREEN_WIDTH+3];
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */

  caTextBuffer[0]='|';
  caTextBuffer[iScreenWidth+1]='|';
  caTextBuffer[iScreenWidth+2]='\0';
  puts("Display (1=white, 0=black):\n"
       "-------------------------------------------------------------------------------");
  for(iIndex=0;iIndex<iScreenHeigth;++iIndex)
  {
    for(iIndexB=0;iIndexB<iScreenWidth;++iIndexB)
    {
      caTextBuffer[iIndexB+1]=MEM_SCREEN_BIT_CHECK(tagEmulator_g.tagWindow,iIndexB,iIndex)?'1':'0';
    }
    puts(caTextBuffer);
  }
  puts("-------------------------------------------------------------------------------");
}

int chip8_Process(TagEmulator *pEmulator)
{
  int iRc=RET_CHIP8_OPCODE_OK;
  word tOPCode;
  byte tRegX;
  byte tRegY;

  tOPCode=pEmulator->taChipMemory[REG_PC];
  tOPCode=(tOPCode<<8)|pEmulator->taChipMemory[REG_PC+1];
  pEmulator->tCurrOPCode=tOPCode;
  TRACE_DBG_INFO_VARG("Current OPCode: 0x%.4X @0x%.4X",tOPCode,REG_PC);
  switch(tOPCode&0xF000)
  {
    case 0x0000: /* 0x0xxx */
      if((tOPCode&0x0F00)) /* This byte must be 0 */
      {
        TRACE_DBG_ERROR_VARG("Invalid OPCode: 0x%.4X",tOPCode);
        return(RET_ERR_INSTRUCTION_UNKNOWN);
      }
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
      if((tOPCode&0x00F0)==0x00C0)
      {
        tOPCode&=0x000F;
        iRc=((pEmulator->tagWindow.info.exModeOn)?SUCHIP_SCREEN_WIDTH:CHIP8_SCREEN_WIDTH)/8;
        TRACE_SUCHIP_INSTR(TXT_INSTRUCTION_0x00CN,REG_PC,pEmulator->tCurrOPCode,tOPCode,tOPCode);
        TRACE_DBG_INFO_VARG("Scrolling down 0x%X lines, moving from data[0x0000]->[0x%.4X] (0x%.4X bytes), set data[0x0000]->[0x%.4X] to 0",
                            tOPCode,
                            iRc*tOPCode,
                            ((pEmulator->tagWindow.info.exModeOn)?SCREEN_SIZE_EXMODE:SCREEN_SIZE_NORMAL)-iRc*tOPCode,
                            iRc*tOPCode);
        memmove(&pEmulator->tagWindow.taScreenBuffer[iRc*tOPCode],
                &pEmulator->tagWindow.taScreenBuffer[0],
                ((pEmulator->tagWindow.info.exModeOn)?SCREEN_SIZE_EXMODE:SCREEN_SIZE_NORMAL)-iRc*tOPCode);
        memset(pEmulator->tagWindow.taScreenBuffer,0,iRc*tOPCode);
        iRc=RET_SUCHIP_OPCODE_OK;
        break;
      }
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */
      switch((tOPCode&0x00FF))
      {
        case CHIP8_CMD_CLS:
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x00E0,REG_PC,pEmulator->tCurrOPCode);
          pEmulator->updateSettings.screen_Clear=1; /* Will be processed after return */
          break;
        case CHIP8_CMD_RET:
//        printf("Stackptr: 0x%X, startstack: 0x%X, Endstack: 0x%X\n",REG_PTR_STACK,OFF_ADDR_STACK_START,OFF_ADDR_STACK_END);
          if(REG_PTR_STACK>OFF_ADDR_STACK_START)
          {
            TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x00EE,REG_PC,pEmulator->tCurrOPCode);
            REG_STACKPTR_1UP(REG_PTR_STACK); /* Move stackpointer 1 level up */
            REG_PC=MEM_ADDRESS_AS_WORD(REG_PTR_STACK);
            TRACE_DBG_INFO_VARG("Returning from subroutine (->@0x%.4X), deepness: <--%u/%u",
                                REG_PC,
                                (word)((REG_PTR_STACK-OFF_ADDR_STACK_START)/sizeof(word)),
                                (word)((OFF_ADDR_STACK_END-OFF_ADDR_STACK_START+1)/sizeof(word)));
            break; /* Break okay, will increment stored address below and continue at previous point */
          }
          TRACE_DBG_ERROR("RET failed, already on top, end pgm here?");
          return(RET_ERR_STACK_ON_TOP);
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
        case SUCHIP_CMD_SCROLL_RIGHT:
          TRACE_SUCHIP_INSTR(TXT_INSTRUCTION_0x00FB,REG_PC,pEmulator->tCurrOPCode);
          tRegY=(pEmulator->tagWindow.info.exModeOn)?SUCHIP_SCREEN_HEIGHT:CHIP8_SCREEN_HEIGHT;
          tRegX=(pEmulator->tagWindow.info.exModeOn)?SUCHIP_SCREEN_WIDTH:CHIP8_SCREEN_WIDTH;
          for(tOPCode=0;tOPCode<8;++tOPCode)
          {
            vChip8_ShiftRight_m(&MEM_SCREEN_BYTE(pEmulator->tagWindow,0,tOPCode),
                                tRegX/8,
                                4);
          }
          iRc=RET_SUCHIP_OPCODE_OK;
          break;
        case SUCHIP_CMD_SCROLL_LEFT:
          TRACE_SUCHIP_INSTR(TXT_INSTRUCTION_0x00FC,REG_PC,pEmulator->tCurrOPCode);
          tRegY=(pEmulator->tagWindow.info.exModeOn)?SUCHIP_SCREEN_HEIGHT:CHIP8_SCREEN_HEIGHT;
          tRegX=(pEmulator->tagWindow.info.exModeOn)?SUCHIP_SCREEN_WIDTH:CHIP8_SCREEN_WIDTH;
          for(tOPCode=0;tOPCode<tRegY;++tOPCode)
          {
            vChip8_ShiftLeft_m(&MEM_SCREEN_BYTE(pEmulator->tagWindow,0,tOPCode),
                               tRegX/8,
                               4);
          }
          iRc=RET_SUCHIP_OPCODE_OK;
          break;
        case SUCHIP_CMD_EXIT:
          TRACE_SUCHIP_INSTR(TXT_INSTRUCTION_0x00FD,REG_PC,pEmulator->tCurrOPCode);
          return(RET_PGM_EXIT);
        case SUCHIP_CMD_DIS_EX_SCREEN:
          TRACE_SUCHIP_INSTR(TXT_INSTRUCTION_0x00FE,REG_PC,pEmulator->tCurrOPCode);
          pEmulator->updateSettings.screen_ExMode_Dis=1; /* Will be processed after return */
          iRc=RET_SUCHIP_OPCODE_OK;
          break;
        case SUCHIP_CMD_EN_EX_SCREEN:
          TRACE_SUCHIP_INSTR(TXT_INSTRUCTION_0x00FF,REG_PC,pEmulator->tCurrOPCode);
          pEmulator->updateSettings.screen_ExMode_En=1; /* Will be processed after return */
          iRc=RET_SUCHIP_OPCODE_OK;
          break;
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */
        default:
          return(RET_ERR_INSTRUCTION_UNKNOWN);
      }
      break;
    case 0x1000: /* 0x1nnn, Jump to address */
      tOPCode&=0xFFF;
      TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x1NNN,REG_PC,pEmulator->tCurrOPCode,tOPCode,tOPCode);
      if(MEM_JMP_ADDR_VALID(tOPCode))
      {
        REG_PC=tOPCode;
        return(RET_CHIP8_OPCODE_OK); /* Return immediately, no increment of PC needed */
      }
      TRACE_DBG_ERROR_VARG("JMP failed, tried to jump to addr 0x%.4X (0x%.4X-0x%.4X allowed)",tOPCode,OFF_ADDR_USER_START,OFF_ADDR_USER_END);
      return(RET_ERR_INVALID_JUMP_ADDRESS);
    case 0x2000: /* 0x2nnn, Jump to addr, as a subroutine */
      tOPCode&=0xFFF;
      TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x2NNN,
                  REG_PC,
                  pEmulator->tCurrOPCode,
                  tOPCode,
                  tOPCode,
                  (word)((REG_PTR_STACK-OFF_ADDR_STACK_START)/sizeof(word)),
                  (word)((OFF_ADDR_STACK_END-OFF_ADDR_STACK_START)/sizeof(word)));
      if(MEM_JMP_ADDR_VALID(tOPCode))
      {
        if(REG_PTR_STACK<OFF_ADDR_STACK_END)
        {
          MEM_ADDRESS_AS_WORD(REG_PTR_STACK)=REG_PC; /* Store curr PC in Stack */
          REG_PC=tOPCode;
          TRACE_DBG_INFO_VARG("Storing Current Programmcounter (0x%.4X) @Stack: 0x%.4X",REG_PC,REG_PTR_STACK);
          REG_STACKPTR_1DOWN(REG_PTR_STACK); /* Move stackpointer 1 level down */
          return(RET_CHIP8_OPCODE_OK); /* Return immediately, no increment of PC needed */
        }
        TRACE_DBG_ERROR_VARG("CALL failed, max emulator nested calls reached (%u allowed)",OFF_ADDR_STACK_END-OFF_ADDR_STACK_START+1);
        return(RET_ERR_STACK_MAX_CALLS);
      }
      TRACE_DBG_ERROR_VARG("CALL failed, tried to jump to addr 0x%.4X (0x%.4X-0x%.4X allowed)",tOPCode,OFF_ADDR_USER_START,OFF_ADDR_USER_END);
      return(RET_ERR_INVALID_JUMP_ADDRESS);
    case 0x3000: /* 0x3xkk, Compare Reg x with value kk, if equal -> Skip next instruction */
      tRegX=(tOPCode&0x0F00)>>8;
      tOPCode&=0x00FF;
      TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x3XKK,REG_PC,pEmulator->tCurrOPCode,tRegX,tOPCode,tRegX,REG_VX(tRegX),tOPCode);
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
      TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x4XKK,REG_PC,pEmulator->tCurrOPCode,tRegX,tOPCode,tRegX,REG_VX(tRegX),tOPCode);
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
        return(RET_ERR_INSTRUCTION_UNKNOWN);
      tRegX=(tOPCode&0x0F00)>>8;
      tRegY=(tOPCode&0x00F0)>>4;
      TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x5XY0,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegX,tRegY);
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
      TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x6XKK,REG_PC,pEmulator->tCurrOPCode,tRegX,tOPCode,tOPCode,tRegX);
      REG_VX(tRegX)=tOPCode;
      break;
    case 0x7000: /* 0x7xkk, Add Value kk to Register x */
      tRegX=(tOPCode&0x0F00)>>8;
      tOPCode&=0x00FF;
      TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x7XKK,REG_PC,pEmulator->tCurrOPCode,tRegX,tOPCode,tOPCode,tRegX);
      REG_VX(tRegX)+=tOPCode;
      break;
    case 0x8000: /* Mathematical Operations */
      tRegX=(tOPCode&0x0F00)>>8;
      tRegY=(tOPCode&0x00F0)>>4;
      switch(tOPCode&0x000F)
      {
        case CHIP8_CMD_MATH_MOV: /* MOV */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x8XY0,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegY,REG_VX(tRegY),tRegX);
          REG_VX(tRegX)=REG_VX(tRegY);
          break;
        case CHIP8_CMD_MATH_OR: /* OR */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x8XY1,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegY,REG_VX(tRegY),tRegX,REG_VX(tRegX));
          REG_VX(tRegX)|=REG_VX(tRegY);
          break;
        case CHIP8_CMD_MATH_AND: /* AND */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x8XY2,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegY,REG_VX(tRegY),tRegX,REG_VX(tRegX));
          REG_VX(tRegX)&=REG_VX(tRegY);
          break;
        case CHIP8_CMD_MATH_XOR: /* XOR */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x8XY3,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegY,REG_VX(tRegY),tRegX,REG_VX(tRegX));
          REG_VX(tRegX)^=REG_VX(tRegY);
          break;
        case CHIP8_CMD_MATH_ADD: /* ADD */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x8XY4,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegY,REG_VX(tRegY),tRegX,REG_VX(tRegX));
          if((tOPCode=REG_VX(tRegX)+REG_VX(tRegY))>0xFF)
            REG_VF=1; /* Set overflow indication on VF */
          REG_VX(tRegX)=tOPCode&0xFF;
          break;
        case CHIP8_CMD_MATH_SUB: /* SUB */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x8XY5,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegY,REG_VX(tRegY),tRegX,REG_VX(tRegX));
          REG_VF=(REG_VX(tRegX)>REG_VX(tRegY))?1:0; /* Set underflow indication on VF */
          REG_VX(tRegX)-=REG_VX(tRegY);
          break;
        case CHIP8_CMD_MATH_SHR: /* SHR */
          TRACE_CHIP8_INSTR(EMU_CHECK_QUIRK(pEmulator,EMU_QUIRK_SHIFT_SOURCE_REG)?TXT_INSTRUCTION_0x8XY6 "(QUIRK ENABLED)":TXT_INSTRUCTION_0x8XY6,
                            REG_PC,
                            pEmulator->tCurrOPCode,
                            tRegX,
                            tRegX,
                            REG_VX(tRegX));
          REG_VF=(REG_VX(tRegX)&0x1)?1:0; /* If LSB (0000 0001) is set, set VF=1, otherways VF=0 */
          if(EMU_CHECK_QUIRK(pEmulator,EMU_QUIRK_SHIFT_SOURCE_REG))
          {
            REG_VX(tRegY)>>=1;
            REG_VX(tRegX)=REG_VX(tRegY);
          }
          else
            REG_VX(tRegX)>>=1;
          break;
        case CHIP8_CMD_MATH_SUBN: /* SUBN */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x8XY7,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegX,REG_VX(tRegX),tRegY,REG_VX(tRegY));
          REG_VF=(REG_VX(tRegX)<REG_VX(tRegY))?1:0; /* Set underflow indication on VF */
          REG_VX(tRegX)=REG_VX(tRegY)-REG_VX(tRegX);
          break;
        case CHIP8_CMD_MATH_SHL: /* SHL */
          TRACE_CHIP8_INSTR(EMU_CHECK_QUIRK(pEmulator,EMU_QUIRK_SHIFT_SOURCE_REG)?TXT_INSTRUCTION_0x8XYE "(QUIRK ENABLED)":TXT_INSTRUCTION_0x8XYE,
                            REG_PC,
                            pEmulator->tCurrOPCode,
                            tRegX,
                            tRegX,
                            REG_VX(tRegX));
          REG_VF=(REG_VX(tRegX)&0x80)?1:0; /* If MSB (1000 0000) is set, set VF=1, otherways VF=0 */
          if(EMU_CHECK_QUIRK(pEmulator,EMU_QUIRK_SHIFT_SOURCE_REG))
          {
            REG_VX(tRegY)<<=1;
            REG_VX(tRegX)=REG_VX(tRegY);
          }
          else
            REG_VX(tRegX)<<=1;
          break;
        default:
          return(RET_ERR_INSTRUCTION_UNKNOWN);
      }
      break;
    case 0x9000: /* 0x9xy0, Compare Reg x with Reg y, if values are not equal -> Skip next instruction */
      if(tOPCode&0x000F) /* Nibble must be 0 */
        return(RET_ERR_INSTRUCTION_UNKNOWN);
      tRegX=(tOPCode&0x0F00)>>8;
      tRegY=(tOPCode&0x00F0)>>4;
      TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0x9XY0,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegX,REG_VX(tRegX),tRegY,REG_VX(tRegY));
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
      TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xANNN,REG_PC,pEmulator->tCurrOPCode,REG_I,REG_I);
      break;
    case 0xB000: /* 0xBnnn, Jump to Reg I + V0 */
      tOPCode&=0x0FFF;
      TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xBNNN,REG_PC,pEmulator->tCurrOPCode,tOPCode,REG_V0,tOPCode);
      tOPCode+=REG_V0;
      if(MEM_JMP_ADDR_VALID(tOPCode))
      {
        REG_PC=tOPCode;
        return(RET_CHIP8_OPCODE_OK); /* Return immediately, no increment of PC needed */
      }
      TRACE_DBG_ERROR_VARG("JMP failed, addr 0x%.4X not valid",tOPCode);
      return(RET_ERR_INVALID_JUMP_ADDRESS);
    case 0xC000: /* 0xCxkk, Generate random number, AND with val kk and store in Reg_Vx */
      tRegX=(tOPCode&0x0F00)>>8;
      tOPCode&=0x00FF;
      TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xCXKK,REG_PC,pEmulator->tCurrOPCode,tRegX,tOPCode,tOPCode,tRegX);
      REG_VX(tRegX)=tChip8_GetRand_m()&tOPCode;
      break;
    case 0xD000: /* 0xDxyn, Draw n Sprites @Pos Reg_Vx/REG_Vy */
      tRegX=(tOPCode&0x0F00)>>8;
      tRegY=(tOPCode&0x00F0)>>4;
      tOPCode&=0x000F;
      if(tOPCode)
      {
        TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xDXYN,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tOPCode,tOPCode,tRegX,REG_VX(tRegX),tRegY,REG_VX(tRegY));
        if((iRc=iChip8_Sprite_m(pEmulator,
                                REG_VX(tRegX),
                                REG_VX(tRegY),
                                tOPCode))!=RET_CHIP8_OPCODE_OK)
        {
          TRACE_DBG_ERROR("iChip8_Sprite_m() failed to draw Screen");
          return(iRc);
        }
      }
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
      else
      {
        TRACE_SUCHIP_INSTR(TXT_INSTRUCTION_0xDXY0,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegY,tRegX,REG_VX(tRegX),tRegY,REG_VX(tRegY));
        if(!pEmulator->tagWindow.info.exModeOn)
        {
          TRACE_DBG_ERROR("Can't use Superchip draw instruction on normal screen mode!");
          return(RET_ERR_SCREEN_DRAW);
        }
        if((iRc=iSuperchip_Sprite_m(pEmulator,
                                    REG_VX(tRegX),
                                    REG_VX(tRegY)))!=RET_SUCHIP_OPCODE_OK)
        {
          TRACE_DBG_ERROR("iSuperchip_Sprite_m() failed to draw Screen");
          return(iRc);
        }
      }
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */
      break;
    case 0xE000: /* Keypress stuff */
      tRegX=(tOPCode&0x0F00)>>8;
      if(REG_VX(tRegX)>0xF) /* Check for invalid key */
      {
        TRACE_DBG_ERROR_VARG("Invalid Key in REG_V%X: %X, allowed: 0-F",tRegX,REG_VX(tRegX));
        return(RET_ERR_KEYCODE_INVALID);
      }
      switch((tOPCode&0x00FF))
      {
        case CHIP8_CMD_KEY_PRESSED: /* 0xEx9E, Skip next instruction if REG_Vx==pressed key */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xEX9E,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          if((pEmulator->tagKeyboard.ulKeyStates&(1<<REG_VX(tRegX))))
          {
            TRACE_DBG_INFO("Key is pressed, skipping next instruction");
            REG_PROGRAMCOUNTER_INCREMENT(); /* Increment here, and at the end of switch, results in skipping */
            break;
          }
          TRACE_DBG_INFO("Key is not pressed");
          break;
        case CHIP8_CMD_KEY_RELEASED: /* 0xExA1, Skip next instruction if REG_Vx==released key */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xEXA1,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          if(!(pEmulator->tagKeyboard.ulKeyStates&(1<<REG_VX(tRegX))))
          {
            TRACE_DBG_INFO("Key is released, skipping next instruction");
            REG_PROGRAMCOUNTER_INCREMENT(); /* Increment here, and at the end of switch, results in skipping */
            break;
          }
          TRACE_DBG_INFO("Key is not released");
          break;
        default:
          return(RET_ERR_INSTRUCTION_UNKNOWN);
      }
      break;
    case 0xF000: /* Misc stuff */
      tRegX=(tOPCode&0x0F00)>>8;
      switch((tOPCode&0x00FF))
      {
        case CHIP8_CMD_DELAY_TIMER_GET: /* 0xFx07, Set REG_Vx = Delay timer value */
          REG_VX(tRegX)=REG_TMRDEL;
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xFX07,REG_PC,pEmulator->tCurrOPCode,tRegX,REG_VX(tRegX),tRegX);
          break;
        case CHIP8_CMD_WAIT_FOR_KEY: /* 0xFx0A, wait until a Key is pressed, store it in REG_Vx */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xFX0A,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX);
          if(iChip8_GetPressedKey_m(&pEmulator->tagKeyboard,
                                    &REG_VX(tRegX)))
            return(RET_CHIP8_OPCODE_OK); /* If no key is pressed, return and call this again */
          break;
        case CHIP8_CMD_DELAY_TIMER_SET: /* 0xFx15, Set DelayTimer= REG_Vx */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xFX15,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          REG_TMRDEL=REG_VX(tRegX);
          break;
        case CHIP8_CMD_SOUND_TIMER_SET: /* 0xFx18, Set SoundTimer = REG_Vx */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xFX18,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          REG_TMRSND=REG_VX(tRegX); /* Other thread will start sound playback */
          break;
        case CHIP8_CMD_I_ADD_VX: /* 0xFx1E, Increment I by the val of REG_Vx */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xFX1E,REG_PC,pEmulator->tCurrOPCode,tRegX,REG_I,tRegX,REG_VX(tRegX));
          REG_I+=REG_VX(tRegX);
          break;
        case CHIP8_CMD_I_SPRITE_GET:  /* 0xFx29, Set I to the address from sprite of val REG_Vx */
          if((tOPCode=REG_VX(tRegX))>0xF)
          {
            TRACE_DBG_ERROR_VARG("Can't set REG_I to Sprite, Number 0x%.2X out of bounds (0-F allowed)",REG_VX(tRegX));
            return(RET_ERR_FONT_OUT_OF_INDEX);
          }
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xFX29,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          REG_I=(tOPCode*5)+OFF_ADDR_FONT_START;
          TRACE_DBG_INFO_VARG("Setting I on Pos for Sprite '%X'@0x%.4X",REG_VX(tRegX),REG_I);
          break;
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
        case SUCHIP_CMD_I_XSPRITE_GET:
          if((tOPCode=REG_VX(tRegX))>0xF)
          {
            TRACE_DBG_ERROR_VARG("Can't set REG_I to Sprite, Number 0x%.2X out of bounds (0-F allowed)",REG_VX(tRegX));
            return(RET_ERR_FONT_OUT_OF_INDEX);
          }
          TRACE_SUCHIP_INSTR(TXT_INSTRUCTION_0xFX30,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          REG_I=(tOPCode*10)+OFF_ADDR_XFONT_START;
          TRACE_DBG_INFO_VARG("Setting I on Pos for XSprite '%X'@0x%.4X",REG_VX(tRegX),REG_I);
          iRc=RET_SUCHIP_OPCODE_OK;
          break;
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */
        case CHIP8_CMD_VX_GET_BCD: /* 0xFx33, Store BCD val of REG_Vx in REG_I-REG_I+2 */
          TRACE_CHIP8_INSTR(TXT_INSTRUCTION_0xFX33,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX,REG_VX(tRegX));
          TRACE_DBG_INFO_VARG("BCD Reg_V%X -> I",tRegX);
          if(!REG_VX(tRegX))
          {
            TRACE_DBG_INFO("Register=0, set mem[I]-mem[I+2] to 0");
            pEmulator->taChipMemory[REG_I]=0;
            pEmulator->taChipMemory[REG_I+1]=0;
            pEmulator->taChipMemory[REG_I+2]=0;
          }
          else
          {
            pEmulator->taChipMemory[REG_I]=REG_VX(tRegX)/100;
            pEmulator->taChipMemory[REG_I+1]=(REG_VX(tRegX)%100)/10;
            pEmulator->taChipMemory[REG_I+2]=(REG_VX(tRegX)%100)%10;
            TRACE_DBG_INFO_VARG("Set mem[I]= 0x%.2X 0x%.2X 0x%.2X",
                                pEmulator->taChipMemory[REG_I],
                                pEmulator->taChipMemory[REG_I+1],
                                pEmulator->taChipMemory[REG_I+2]);
          }
          break;
        case CHIP8_CMD_REGISTERS_STORE: /* 0xFx55, Store register V0-Vx in memory pointed by REG_I */
          TRACE_CHIP8_INSTR((EMU_CHECK_QUIRK(pEmulator,EMU_QUIRK_INCREMENT_I_ON_STORAGE))?TXT_INSTRUCTION_0xFX55 "(QUIRK ENABLED)":TXT_INSTRUCTION_0xFX55,
                            REG_PC,
                            pEmulator->tCurrOPCode,
                            tRegX,
                            tRegX);
          TRACE_DBG_INFO_VARG("Storing Register V0-V%X in memory @0x%.4X-0x%.4X",
                              tRegX,
                              REG_I,
                              REG_I+tRegX);

          if((!MEM_JMP_ADDR_VALID(REG_I)) ||
             (REG_I+tRegX>OFF_ADDR_USER_END))
          {
            TRACE_DBG_ERROR("Can't store Registers, memory would overflow");
            return(RET_ERR_MEM_WOULD_OVERFLOW);
          }
          for(tOPCode=0;tOPCode<=tRegX;++tOPCode)
          {
            pEmulator->taChipMemory[REG_I+tOPCode]=REG_VX(tOPCode);
            TRACE_DBG_INFO_VARG("Store Value 0x%.2X from V%X to mem @ 0x%.4X",
                                REG_VX(tOPCode),
                                tOPCode,
                                REG_I+tOPCode);
          }
          /* If Quirk is enabled, increment I by x */
          if(EMU_CHECK_QUIRK(pEmulator,EMU_QUIRK_INCREMENT_I_ON_STORAGE))
            REG_I+=tRegX;
          break;
        case CHIP8_CMD_REGISTERS_READ: /* 0xFx65, Read Values for Registers V0-Vx from memory pointed by REG_I */
          TRACE_CHIP8_INSTR((EMU_CHECK_QUIRK(pEmulator,EMU_QUIRK_INCREMENT_I_ON_STORAGE))?TXT_INSTRUCTION_0xFX65 "(QUIRK ENABLED)":TXT_INSTRUCTION_0xFX65,
                            REG_PC,
                            pEmulator->tCurrOPCode,
                            tRegX,
                            tRegX);
          TRACE_DBG_INFO_VARG("Reading Register V0-V%X from memory @0x%.4X-0x%.4X",
                              tRegX,
                              REG_I,
                              REG_I+tRegX);

          if((!MEM_JMP_ADDR_VALID(REG_I)) ||
             (REG_I+tRegX>OFF_ADDR_USER_END))
          {
            TRACE_DBG_ERROR("Can't read memory, invalid address");
            return(RET_ERR_MEM_WOULD_OVERFLOW);
          }
          for(tOPCode=0;tOPCode<=tRegX;++tOPCode)
          {
            REG_VX(tOPCode)=pEmulator->taChipMemory[REG_I+tOPCode];
            TRACE_DBG_INFO_VARG("Read Value 0x%.2X from mem @ 0x%.4X to V%X",
                                pEmulator->taChipMemory[REG_I+tOPCode],
                                REG_I+tOPCode,
                                tOPCode);
          }
          /* If Quirk is enabled, increment I by x */
          if(EMU_CHECK_QUIRK(pEmulator,EMU_QUIRK_INCREMENT_I_ON_STORAGE))
            REG_I+=tRegX;
          break;
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
        case SUCHIP_CMD_STORE_TO_RPL:
          TRACE_SUCHIP_INSTR(TXT_INSTRUCTION_0xFX75,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX);

          iRc=RET_SUCHIP_OPCODE_OK;
          break;
        case SUCHIP_CMD_READ_FROM_RPL:
          TRACE_SUCHIP_INSTR(TXT_INSTRUCTION_0xFX85,REG_PC,pEmulator->tCurrOPCode,tRegX,tRegX);

          iRc=RET_SUCHIP_OPCODE_OK;
          break;
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */
        default:
          return(RET_ERR_INSTRUCTION_UNKNOWN);
      }
      break;
    default:
      return(RET_ERR_INSTRUCTION_UNKNOWN);
  }
  REG_PROGRAMCOUNTER_INCREMENT();
  return(iRc);
}

INLINE_FCT byte tChip8_GetRand_m(void)
{
  return(rand()/(RAND_MAX/UCHAR_MAX));
}

INLINE_FCT void vChip8_ShiftRight_m(byte *pData,
                                    unsigned int uiDataLength,
                                    int iShift)
{
  byte tTmp1=0;

  /* At first shift, don't add low part */
  --uiDataLength;
  tTmp1=(pData[uiDataLength]&(0xFF>>(8-iShift))); /* Keep low part */
//printf("Shift data[%u]=0x%.2X, got tmp: 0x%.2X\n",uiDataLength,pData[uiDataLength],tTmp1);
  pData[uiDataLength]>>=iShift;
  do
  {
    --uiDataLength;
    tTmp1=(pData[uiDataLength]&(0xFF>>(8-iShift))); /* Keep low part */
//  printf("Shift data[%u]=0x%.2X, got tmp: 0x%.2X\n",uiDataLength,pData[uiDataLength],tTmp1);
    pData[uiDataLength]>>=iShift;
    pData[uiDataLength+1]|=tTmp1<<(8-iShift); /* Add low part to next byte */
//  printf("Shifted data[%u]=0x%.2X\n",uiDataLength,pData[uiDataLength]);
  }while(uiDataLength);
}

INLINE_FCT void vChip8_ShiftLeft_m(byte *pData,
                                   unsigned int uiDataLength,
                                   int iShift)
{
  byte tTmp1=0;
  unsigned int uiIndex;

  pData[0]<<=iShift;
  for(uiIndex=1;uiIndex<uiDataLength;++uiIndex)
  {
    tTmp1=pData[uiIndex]&(0xFF<<(8-iShift)); /* Keep high part */
//  printf("Shift data[%u]=0x%.2X, got tmp: 0x%.2X\n",uiIndex,pData[uiIndex],tTmp1);
    pData[uiIndex]<<=iShift;
    pData[uiIndex-1]|=((tTmp1)>>(8-iShift));
  }
}

INLINE_FCT int iChip8_Sprite_m(TagEmulator *pEmulator,
                               byte tPosX,
                               byte tPosY,
                               byte tBytes) /* 0xDxyn */
{
  unsigned int uiIndex;
  word tLastVal;
  word tCurrVal;
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
  const unsigned int uiScreenWidth=(pEmulator->tagWindow.info.exModeOn)?SUCHIP_SCREEN_WIDTH:CHIP8_SCREEN_WIDTH;
  const unsigned int uiScreenHeight=(pEmulator->tagWindow.info.exModeOn)?SUCHIP_SCREEN_HEIGHT:CHIP8_SCREEN_HEIGHT;
#else
  const unsigned int uiScreenWidth=CHIP8_SCREEN_WIDTH;
  const unsigned int uiScreenHeight=CHIP8_SCREEN_HEIGHT;
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */

  TRACE_DBG_INFO_VARG("Drawing %u bytes @[%u][%u] (from mem 0x%.4X)",tBytes,tPosX,tPosY,REG_I);

  /* Check memory for overflow */
  if(REG_I+tBytes>OFF_ADDR_USER_END)
  {
    TRACE_DBG_ERROR_VARG("REG_I(0x%.4X)+tBytes(0x%.2X)=0x%.4X > OFF_ADDR_USER_END(0x%.4X), cancel drawing, Memory would overflow",
                         REG_I,
                         tBytes,
                         REG_I+tBytes,
                         OFF_ADDR_USER_END);
    return(RET_ERR_MEM_WOULD_OVERFLOW);
  }
  /* Check if dest coordinates are okay/ on screen  */
  if((tPosX>uiScreenWidth-1) || (tPosY>uiScreenHeight-1))
  {
    TRACE_DBG_ERROR_VARG("Drawing Position(%u-%u/%u-%u) out of Screen(%u/%u), cancel drawing",
                         tPosX,
                         tPosX+8,
                         tPosY,
                         tPosY+tBytes,
                         uiScreenWidth-1,
                         uiScreenHeight-1);
    return(RET_ERR_DRAW_OUT_OF_SCREEN);
  }
  REG_VF=0;
  for(uiIndex=0;uiIndex<tBytes;++uiIndex)
  {
    tLastVal=0;
    tCurrVal=0;
    if(tPosY+uiIndex>uiScreenHeight-1)
    {
      TRACE_DBG_INFO("Cancel drawing, Y-Axis Overflow");
      break;
    }
    tLastVal|=MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX,tPosY+uiIndex);

    if((tPosX+8u<uiScreenWidth) && (tPosX&0x7))
    {
      tLastVal|=(MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX+8,tPosY+uiIndex)<<8);
    }
    tCurrVal|=(pEmulator->taChipMemory[REG_I+uiIndex])>>(tPosX&0x7);
    MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX,tPosY+uiIndex)^=(tCurrVal);

    if((tPosX+8u<uiScreenWidth) && (tPosX&0x7))
    {
      tCurrVal|=(pEmulator->taChipMemory[REG_I+uiIndex] & (0xFF>>(8-(tPosX&0x7)))) << (16-(tPosX&0x7));
      MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX+8u,tPosY+uiIndex)^=tCurrVal>>8;
    }
    if(tPosY+uiIndex>uiScreenHeight-1)
    {
      TRACE_DBG_INFO("Cancel drawing, Y-Axis Overflow");
      break;
    }
    tLastVal|=MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX,tPosY+uiIndex);

    if((!REG_VF) &&
       ((tCurrVal&tLastVal)>0))
    {
//    printf("Pixel was ereased, set REG_VF to 1\n");
      REG_VF=1;
    }
  }
  return(RET_CHIP8_OPCODE_OK);
}


#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
INLINE_FCT int iSuperchip_Sprite_m(TagEmulator *pEmulator,
                                   byte tPosX,
                                   byte tPosY) /* 0xDxy0 */
{
  unsigned int uiIndex;
  word tLastVal;
  word tCurrVal;
  TRACE_DBG_INFO_VARG("(Superchip)Drawing 16 bytes @[%u][%u] (from mem 0x%.4X)",tPosX,tPosY,REG_I);

  if((tPosX>SUCHIP_SCREEN_WIDTH-9) || (tPosY>SUCHIP_SCREEN_HEIGHT-1))
  {
    TRACE_DBG_ERROR_VARG("Drawing Position(%u-%u/%u-%u) out of Screen(%u/%u), cancel drawing",
                         tPosX,
                         tPosX+16,
                         tPosY,
                         tPosY+16,
                         SUCHIP_SCREEN_WIDTH-1,
                         SUCHIP_SCREEN_HEIGHT-1);
    return(RET_ERR_DRAW_OUT_OF_SCREEN);
  }
  for(uiIndex=0;uiIndex<16;++uiIndex)
  {
    if(tPosY+uiIndex>SUCHIP_SCREEN_HEIGHT-1)
    {
      TRACE_DBG_INFO("Cancel drawing, Y-Axis Overflow");
      break;
    }
    /* Draw first byte */
    tLastVal=0;
    tCurrVal=0;
    tLastVal=MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX,tPosY+uiIndex);
    if((tPosX+8u<SUCHIP_SCREEN_WIDTH) && (tPosX&0x7))
    {
      tLastVal|=(MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX+8,tPosY+uiIndex)<<8);
    }
    tCurrVal|=(pEmulator->taChipMemory[REG_I+uiIndex])>>(tPosX&0x7);
    MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX,tPosY+uiIndex)^=(tCurrVal);

    if((tPosX+8u<SUCHIP_SCREEN_WIDTH) && (tPosX&0x7))
    {
      tCurrVal|=(pEmulator->taChipMemory[REG_I+uiIndex] & (0xFF>>(8-(tPosX&0x7)))) << (16-(tPosX&0x7));
      MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX+8u,tPosY+uiIndex)^=tCurrVal>>8;
    }
    if((!REG_VF) &&
       ((tCurrVal&tLastVal)>0))
    {
      REG_VF=1;
    }
    /* Draw 2nd byte */
    tLastVal=0;
    tCurrVal=0;
    tLastVal=MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX+8,tPosY+uiIndex);
    if((tPosX+16u<SUCHIP_SCREEN_WIDTH) && (tPosX&0x7))
    {
      tLastVal|=(MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX+16,tPosY+uiIndex)<<8);
    }
    tCurrVal|=(pEmulator->taChipMemory[REG_I+uiIndex])>>(tPosX&0x7);
    MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX+8,tPosY+uiIndex)^=(tCurrVal);

    if((tPosX+8u<SUCHIP_SCREEN_WIDTH) && (tPosX&0x7))
    {
      tCurrVal|=(pEmulator->taChipMemory[REG_I+(uiIndex*2)+1] & (0xFF>>(8-(tPosX&0x7)))) << (16-(tPosX&0x7));
      MEM_SCREEN_BYTE(pEmulator->tagWindow,tPosX+16u,tPosY+uiIndex)^=tCurrVal>>8;
    }
    if((!REG_VF) &&
       ((tCurrVal&tLastVal)>0))
    {
      REG_VF=1;
    }
  }
  return(RET_SUCHIP_OPCODE_OK);
}
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */

INLINE_FCT int iChip8_GetPressedKey_m(TagKeyboard *ptagKeyboard,
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

INLINE_FCT void vChip8_MemoryInit_m(byte *pMem,
                                    word wStartAddress)
{
  /* Copy Fonts to memory */
  memcpy(&pMem[OFF_ADDR_FONT_START],tChip8_Font_m,OFF_ADDR_FONT_END-OFF_ADDR_FONT_START);
#ifdef ENABLE_SUPERCHIP_INSTRUCTIONS
  memcpy(&pMem[OFF_ADDR_XFONT_START],tSuperChip_Font_m,OFF_ADDR_XFONT_END-OFF_ADDR_XFONT_START);
#endif /* ENABLE_SUPERCHIP_INSTRUCTIONS */
  /* Reset other memory to 0 */
  memset(&pMem[OFF_ADDR_XFONT_END+1],0,OFF_MEM_SIZE-OFF_ADDR_XFONT_END-1);

  /**
   * Init Registers
   */
  /* Set Programcounter to default startaddress */
  REG_PC        = wStartAddress;
  REG_PTR_STACK = OFF_ADDR_STACK_START;
#ifdef CHIP8_TEST_VALUES
  /* Add some values for testing purposes */

  REG_V1=120;
  REG_V2=0;
  REG_V3=112;
  REG_V4=0;

  pMem[wStartAddress++]=0x00;
  pMem[wStartAddress++]=0xFF;
  pMem[wStartAddress++]=0x00;
  pMem[wStartAddress++]=0xE0;
  pMem[wStartAddress++]=0xD1; /* drw1*/
  pMem[wStartAddress++]=0x24;

  pMem[wStartAddress++]=0xA3; /* LD i */
  pMem[wStartAddress++]=0x04;

  pMem[wStartAddress++]=0xD3; /* drw2 */
  pMem[wStartAddress++]=0x24;

  pMem[wStartAddress++]=0xA3; /* LD i */
  pMem[wStartAddress++]=0x08;

  pMem[wStartAddress++]=0xD4; /* drw2 */
  pMem[wStartAddress++]=0x24;

  pMem[wStartAddress++]=0x00;
  pMem[wStartAddress++]=0xC4; /* SCD */

//pMem[wStartAddress++]=0x00;
//pMem[wStartAddress++]=0xFC; /* SCL */

  REG_I=0x300;
  pMem[0x300]=0xAA;
  pMem[0x301]=0x55;
  pMem[0x302]=0x00;
  pMem[0x303]=0xFF;
  pMem[0x304]=0xFF;
  pMem[0x305]=0x00;
  pMem[0x306]=0x55;
  pMem[0x307]=0xAA;

  pMem[0x308]=0xFF;
  pMem[0x309]=0x55;
  pMem[0x30A]=0xAA;
  pMem[0x30B]=0xFF;
  pMem[0x30C]=0x00;
  pMem[0x30D]=0xFF;
  pMem[0x30E]=0x55;
  pMem[0x30F]=0xAA;


#endif
}
