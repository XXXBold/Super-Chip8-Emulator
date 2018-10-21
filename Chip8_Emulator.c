#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "Chip8_Emulator.h"
#include "Chip8_screen.h"
#include "Chip8_freq.h"
#include "Chip8_sound.h"
#include "Chip8_keys.h"

#define CHIP8_MEMORY_SIZE         0xFFF /* 4095 */
#define CHIP8_MEMORY_USABLE_START 0x200

#define CHIP8_STACK_VALUES           0xF /* 16*2 Bytes used as stack space */
#define CHIP8_GENERAL_REGISTER_COUNT 0xF /* 16 General Purpose Registers */

/* CMDs starting with 0x00 */
#define CHIP8_CMD_CLS              0x00E0
#define CHIP8_CMD_RET              0x00EE

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

  #define TRACE_DEBUG_INFO
#define TRACE_DEBUG_ERROR

//#define CHIP8_TEST_VALUES /* Enable this for testing purposes */

#ifdef TRACE_DEBUG_INFO
  #define TRACE_DBG_INFO(txt)            puts(txt)
  #define TRACE_DBG_INFO_VARG(txt,...)   printf(txt,__VA_ARGS__)
#else
  #define TRACE_DBG_INFO(txt)
  #define TRACE_DBG_INFO_VARG(txt,...)
#endif /* TRACE_DEBUG_INFO */

#ifdef TRACE_DEBUG_ERROR
  #define TRACE_DBG_ERROR(txt)           puts(txt)
  #define TRACE_DBG_ERROR_VARG(txt,...)  printf(txt,__VA_ARGS__)
#else
  #define TRACE_DBG_ERROR(txt)
  #define TRACE_DBG_ERROR_VARG(txt,...)
#endif /* TRACE_DEBUG_ERROR */

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* >= C99 */
  #define INLINE_FCT inline
  #define INLINE_PROT static inline
#else /* No inline available from C Standard */
  #define INLINE_FCT static
  #define INLINE_PROT static
#endif /* __STDC_VERSION__ >= C99 */

//#define CHIP8_JMP_ADDR_VALID(addr) (((addr<CHIP8_MEMORY_USABLE_START) || (addr>CHIP8_MEMORY_SIZE-1))?0:1)
  #define CHIP8_JMP_ADDR_VALID(addr) ((addr>CHIP8_MEMORY_SIZE-1)?0:1)
#define CHIP8_PROGRAMCOUNTER_INCREMENT() tagChipStatus_m.tChip8_RegisterProgramCounter+=2
#define CHIP8_PROGRAMCOUNTER_SKIP_NEXT() tagChipStatus_m.tChip8_RegisterProgramCounter+=4
#define CHIP8_CHECK_PIXEL_EREASED(curr,last) (((curr&last)<last)?1:0)

typedef struct
{
  byte taChip8_RegistersGeneral[CHIP8_GENERAL_REGISTER_COUNT];
  byte tChip8_RegisterSoundTimer;
  byte tChip8_RegisterDelayTimer;
  word taStack[CHIP8_STACK_VALUES];
  word tStackPointer;
  word tChip8_RegisterI;
  word tChip8_RegisterProgramCounter;
}TagChipStatus;

typedef void (*pfSysInstruction) (word tOpCode);
typedef void (*pfMathInstruction) (byte *ptRegX, byte tRegY);

static TagChipStatus tagChipStatus_m;
static byte taChip8_Memory_g[CHIP8_MEMORY_SIZE]={0xFF}; /* Set first byte to 0xFF, to indicate first run and call srand() */

static byte taScreenBuffer_m[CHIP8_SCREEN_WIDTH/8][CHIP8_SCREEN_HEIGHT];

static const byte tChip8_font_m[80]=
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

INLINE_PROT void vOp_Sys_Internal_m(word tOpCode); /* 0x0xxx, 0x00E0=CLS, 0x00EE=RET */

INLINE_PROT void vOp_Sys_JMP_m(word tOpCode); /* 0x1nnn */

INLINE_PROT void vOp_Sys_CALL_m(word tOpCode); /* 0x2nnn */

INLINE_PROT void vOp_Sys_SKEQ_val_m(byte tRegX, byte tValue); /* 0x3xkk */

INLINE_PROT void vOp_Sys_SKNEQ_val_m(byte tRegX, byte tValue); /* 0x4xkk */

INLINE_PROT void vOp_Sys_SKEQ_reg_m(byte tRegX, byte tRegY); /* 0x5xy0 */

INLINE_PROT void vOp_Sys_MOV_val_m(byte tRegX, byte tValue); /* 0x6xkk */

INLINE_PROT void vOp_Sys_ADD_val_m(byte tRegX, byte tValue); /* 0x7xkk */

INLINE_PROT void vOp_Sys_Math_m(word tOpCode); /* 0x8xyn, Mathematical Operations */

INLINE_PROT void vOp_Sys_SKNEQ_reg_m(byte tRegX, byte tRegY); /* 0x9xy0 */

INLINE_PROT void vOp_Sys_MOV_I_m(word tOpCode); /* 0xAnnn */
INLINE_PROT void vOp_Sys_JMP_V0_m(word tOpCode); /* 0xBnnn */
INLINE_PROT void vOp_Sys_RAND_m(byte tRegX, byte tValue); /* 0xCxkk */
INLINE_PROT void vOp_Sys_SPRITE_m(byte tPosX, byte tPosY, byte tBytes); /* 0xDxyn */
INLINE_PROT void vOp_Sys_KEY_m(word tOpCode); /* 0xEx9E, 0xExA1 */
INLINE_PROT void vOp_Sys_MISC_m(word tOpCode); /* 0xFxxx */

INLINE_PROT void vChip8_UnknownInstruction_m(word tInstruction);

void ThreadChip8_FreqFunc_m(void);

int iChip8_Init_g(void)
{
  TRACE_DBG_INFO("Initialsing Chip-8 Emulator...");
  /* Check if first call of this function, initialise rng then */
  if(taChip8_Memory_g[0]==0xFF)
  {
    srand((unsigned int)time(NULL));
  }
  if(iChip8_Screen_Init_g())
  {
    fprintf(stderr,"Failed to initialize\n");
    return(1);
  }
  if(iChip8_Frequency_Start_g(ThreadChip8_FreqFunc_m))
  {
    fputs("iChip8_Frequency_Start_g() failed",stderr);
  }
  /* Reset full memory to zero */
  memset(&tagChipStatus_m,0,sizeof(tagChipStatus_m));

  /* Set Programcounter to default startaddress */
  tagChipStatus_m.tChip8_RegisterProgramCounter=CHIP8_MEMORY_USABLE_START;

  /* Add Fonts to memory */
  memcpy(taChip8_Memory_g,tChip8_font_m,sizeof(tChip8_font_m));

#ifdef CHIP8_TEST_VALUES
  /* Add some values for testing purposes */
  tagChipStatus_m.taChip8_RegistersGeneral[1]=0x8;
  tagChipStatus_m.taChip8_RegistersGeneral[2]=0x0;

  taChip8_Memory_g[0x200]=0xD1;
  taChip8_Memory_g[0x201]=0x25;
  tagChipStatus_m.tChip8_RegisterI=0x300;
  taChip8_Memory_g[0x300]=0xFF;
  taChip8_Memory_g[0x301]=0x0F;
  taChip8_Memory_g[0x302]=0xF0;
  taChip8_Memory_g[0x303]=0x00;
  taChip8_Memory_g[0x304]=0xAA;
  taChip8_Memory_g[0x305]=0xFF;
  taChip8_Memory_g[0x306]=0xFF;
  taChip8_Memory_g[0x307]=0xFF;

#endif

  return(0);
}

int iChip8_LoadFile_g(const char *pcFilePath,
                      word tStartAddress)
{
  FILE *fp;
  byte taMemTemp[CHIP8_MEMORY_SIZE-CHIP8_MEMORY_USABLE_START];

  TRACE_DBG_INFO_VARG("Loading File \"%s\" @0x%X\n",pcFilePath,tStartAddress);
  if(!CHIP8_JMP_ADDR_VALID(tStartAddress))
  {
    fprintf(stderr,"iChip8_LoadFile_g() failed: invalid Startaddress: 0x%X\n",tStartAddress);
    return(-1);
  }
  errno=0;
  if(!(fp=fopen(pcFilePath,"rb")))
  {
    fprintf(stderr,"iChip8_LoadFile_g() failed to open File \"%s\": (%d) %s\n",pcFilePath,errno,strerror(errno));
    return(-1);
  }
  if(fread(taMemTemp,sizeof(byte),sizeof(taMemTemp)-tStartAddress,fp)<1)
  {
    if(ferror(fp))
    {
      fprintf(stderr,"iChip8_LoadFile_g(): fread() failed on file \"%s\"\n",pcFilePath);
    }
    else
      fprintf(stderr,"iChip8_LoadFile_g(): File \"%s\" is empty\n",pcFilePath);

    fclose(fp);
    return(-1);
  }
  if(!feof(fp))
  {
    fprintf(stderr,"iChip8_LoadFile_g(): file \"%s\" is too big (> 0x%X-0x%X)\n",pcFilePath,tStartAddress,CHIP8_MEMORY_SIZE);
    fclose(fp);
    return(-1);
  }
  memcpy(&taChip8_Memory_g[tStartAddress],taMemTemp,sizeof(taMemTemp)-tStartAddress);
  fclose(fp);
  return(0);
}

void vChip8_Close_g(void)
{
  TRACE_DBG_INFO("Shutting down Chip-8 Emulator...");
  vChip8_Screen_Close_g();
  vChip8_Frequency_Stop_g();
}

void vChip8_Process_g(void)
{
  word tOpCode;
  tOpCode=taChip8_Memory_g[tagChipStatus_m.tChip8_RegisterProgramCounter];
  tOpCode=(tOpCode<<8)|taChip8_Memory_g[tagChipStatus_m.tChip8_RegisterProgramCounter+1];
  TRACE_DBG_INFO_VARG("Current OPCode: 0x%.4X @0x%.4X\n",tOpCode,tagChipStatus_m.tChip8_RegisterProgramCounter);
  switch((tOpCode&0xF000))
  {
    case 0x0000:
      TRACE_DBG_INFO("Instruction: 0x0xxx: Sysinternal");
      vOp_Sys_Internal_m(tOpCode);
      break;
    case 0x1000:
      TRACE_DBG_INFO("Instruction: 0x1nnn: JMP");
      vOp_Sys_JMP_m((tOpCode&0x0FFF));
      break;
    case 0x2000:
      TRACE_DBG_INFO("Instruction: 0x2nnn: CALL");
      vOp_Sys_CALL_m((tOpCode&0xFFF));
      break;
    case 0x3000:
      TRACE_DBG_INFO("Instruction: 0x3xkk: SKEQ_val");
      vOp_Sys_SKEQ_val_m((tOpCode&0x0F00)>>8,(tOpCode&0x00FF));
      break;
    case 0x4000:
      TRACE_DBG_INFO("Instruction: 0x4xkk: SKNEQ_val");
      vOp_Sys_SKNEQ_val_m((tOpCode&0x0F00)>>8,(tOpCode&0x00FF));
      break;
    case 0x5000:
      TRACE_DBG_INFO("Instruction: 0x5xy0: SKEQ_reg");
      vOp_Sys_SKEQ_reg_m((tOpCode&0x0F00)>>8,(tOpCode&0x00F0)>>4);
      break;
    case 0x6000:
      TRACE_DBG_INFO("Instruction: 0x6xkk: MOV_val");
      vOp_Sys_MOV_val_m((tOpCode&0x0F00)>>8,(tOpCode&0x00FF));
      break;
    case 0x7000:
      TRACE_DBG_INFO("Instruction: 0x7xkk: ADD_val");
      vOp_Sys_ADD_val_m((tOpCode&0x0F00)>>8,(tOpCode&0x00FF));
      break;
    case 0x8000:
      TRACE_DBG_INFO("Instruction: 0x8xyn: Mathematical");
      vOp_Sys_Math_m(tOpCode);
      break;
    case 0x9000:
      TRACE_DBG_INFO("Instruction: 0x9xy0: SKNEQ_reg");
      vOp_Sys_SKNEQ_reg_m((tOpCode&0x0F00)>>8,(tOpCode&0x00F0)>>4);
      break;
    case 0xA000:
      TRACE_DBG_INFO("Instruction: 0xAnnn: MOV_I");
      vOp_Sys_MOV_I_m(tOpCode);
      break;
    case 0xB000:
      TRACE_DBG_INFO("Instruction: 0xBnnn: JMP_I");
      vOp_Sys_JMP_V0_m(tOpCode);
      break;
    case 0xC000:
      TRACE_DBG_INFO("Instruction: 0xCxkk: RAND");
      vOp_Sys_RAND_m((tOpCode&0x0F00)>>8,(tOpCode&0x00FF));
      break;
    case 0xD000:
      TRACE_DBG_INFO("Instruction: 0xDxyn: SPRITE");
      vOp_Sys_SPRITE_m(tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8],
                       tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x00F0)>>4],
                       (tOpCode&0x000F));
      break;
    case 0xE000:
      TRACE_DBG_INFO("Instruction: 0xExxx: KEY");
      vOp_Sys_KEY_m(tOpCode);
      break;
    case 0xF000:
      TRACE_DBG_INFO("Instruction: 0xFxxx: MISC");
      vOp_Sys_MISC_m(tOpCode);
      break;
    default:
      fprintf(stderr,"Unknown Instruction Code 0x%x in Process (Can't happen!)\n",tOpCode);
      return;
  }
//vChip8_DumpScreen_g();
  iChip8_Screen_Update_g(taScreenBuffer_m);
}

INLINE_FCT void vOp_Sys_Internal_m(word tOpCode) /* 0x0xxx, 0x00E0=CLS, 0x00EE=RET */
{
  switch(tOpCode)
  {
    case CHIP8_CMD_CLS:
      TRACE_DBG_INFO("Clearing Screen");
      memset(taScreenBuffer_m,0,sizeof(taScreenBuffer_m));
      CHIP8_PROGRAMCOUNTER_INCREMENT();
      break;
    case CHIP8_CMD_RET:
      if(tagChipStatus_m.tStackPointer)
      {
        tagChipStatus_m.tChip8_RegisterProgramCounter=tagChipStatus_m.taStack[tagChipStatus_m.tStackPointer];
        TRACE_DBG_INFO_VARG("Returning from subroutine (->@0x%.4X), deepness: <--%u/%u\n",tagChipStatus_m.tChip8_RegisterProgramCounter,tagChipStatus_m.tStackPointer,CHIP8_STACK_VALUES);
        --tagChipStatus_m.tStackPointer;
        CHIP8_PROGRAMCOUNTER_INCREMENT();
      }
      else
        TRACE_DBG_ERROR("RET failed, already on top, end pgm here?");
      break;
    default:
      vChip8_UnknownInstruction_m(tOpCode);
      break;
  }
}

INLINE_FCT void vOp_Sys_JMP_m(word tAddress) /* 0x1nnn */
{
  if(CHIP8_JMP_ADDR_VALID(tAddress))
  {
    tagChipStatus_m.tChip8_RegisterProgramCounter=tAddress;
    return;
  }
  TRACE_DBG_ERROR_VARG("JMP failed, tried to jump to addr 0x%.4X (0x%.4X-0x%.4X allowed)\n",tAddress,CHIP8_MEMORY_USABLE_START,CHIP8_MEMORY_SIZE-1);
}

INLINE_FCT void vOp_Sys_CALL_m(word tAddress) /* 0x2nnn */
{
  if(CHIP8_JMP_ADDR_VALID(tAddress))
  {
    if(tagChipStatus_m.tStackPointer<CHIP8_STACK_VALUES-1)
    {
      ++tagChipStatus_m.tStackPointer;
      tagChipStatus_m.taStack[tagChipStatus_m.tStackPointer]=tagChipStatus_m.tChip8_RegisterProgramCounter;
      tagChipStatus_m.tChip8_RegisterProgramCounter=tAddress;
    }
    else
      TRACE_DBG_ERROR("CALL failed, max emulator nested calls reached\n");
    return;
  }
  TRACE_DBG_ERROR_VARG("CALL failed, tried to jump to addr 0x%.4X (0x%.4X-0x%.4X allowed)\n",tAddress,CHIP8_MEMORY_USABLE_START,CHIP8_MEMORY_SIZE-1);
}

INLINE_FCT void vOp_Sys_SKEQ_val_m(byte tRegX, byte tValue) /* 0x3xkk */
{
  if(tagChipStatus_m.taChip8_RegistersGeneral[tRegX]==tValue)
  {
    TRACE_DBG_INFO_VARG("Skipping next instruction, Reg[%u]/Val=%u\n",tRegX,tValue);
    CHIP8_PROGRAMCOUNTER_SKIP_NEXT();
    return;
  }
  CHIP8_PROGRAMCOUNTER_INCREMENT();
  TRACE_DBG_INFO_VARG("Can't skip, Value(=0x%.2X)/Register[%u](=0x%.2X) are not Equal\n",tValue,tRegX,tagChipStatus_m.taChip8_RegistersGeneral[tRegX]);
}

INLINE_FCT void vOp_Sys_SKNEQ_val_m(byte tRegX, byte tValue) /* 0x4xkk */
{
  if(tagChipStatus_m.taChip8_RegistersGeneral[tRegX]!=tValue)
  {
    TRACE_DBG_INFO_VARG("Skipping next instruction, Reg[%u]=%u, Val=%u\n",
                        tRegX,
                        tagChipStatus_m.taChip8_RegistersGeneral[tRegX],
                        tValue);
    CHIP8_PROGRAMCOUNTER_SKIP_NEXT();
    return;
  }
  CHIP8_PROGRAMCOUNTER_INCREMENT();
  TRACE_DBG_INFO("Can't skip, Value/Register are Equal");
}

INLINE_FCT void vOp_Sys_SKEQ_reg_m(byte tRegX, byte tRegY) /* 0x5xy0 */
{
  if(tagChipStatus_m.taChip8_RegistersGeneral[tRegX]==tagChipStatus_m.taChip8_RegistersGeneral[tRegY])
  {
    TRACE_DBG_INFO_VARG("Skipping next instruction, Reg[%u]=%u, Reg[%u]=%u\n",
                        tRegX,
                        tagChipStatus_m.taChip8_RegistersGeneral[tRegX],
                        tRegY,
                        tagChipStatus_m.taChip8_RegistersGeneral[tRegY]);
    CHIP8_PROGRAMCOUNTER_SKIP_NEXT();
    return;
  }
  CHIP8_PROGRAMCOUNTER_INCREMENT();
  TRACE_DBG_INFO("Can't skip, Registers are not Equal");
}

INLINE_FCT void vOp_Sys_MOV_val_m(byte tRegX, byte tValue) /* 0x6xkk */
{
  TRACE_DBG_INFO_VARG("Val 0x%.2X->Reg[%u]\n",tValue,tRegX);
  tagChipStatus_m.taChip8_RegistersGeneral[tRegX]=tValue;
  CHIP8_PROGRAMCOUNTER_INCREMENT();
}

INLINE_FCT void vOp_Sys_ADD_val_m(byte tRegX, byte tValue) /* 0x7xkk */
{
  TRACE_DBG_INFO_VARG("Reg[%u]0x%.2x+0x%.2X\n",tRegX,tagChipStatus_m.taChip8_RegistersGeneral[tRegX],tValue);
  tagChipStatus_m.taChip8_RegistersGeneral[tRegX]+=tValue;
  CHIP8_PROGRAMCOUNTER_INCREMENT();
}

INLINE_FCT void vOp_Sys_Math_m(word tOpCode)
{
  int iTmp;
  byte tRegY=tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x00F0)>>4];
  byte *ptRegX=&tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8];
  TRACE_DBG_INFO_VARG("Math, RegX[%u]=0x%.2X, RegY[%u]=0x%.2X\n",(tOpCode&0x0F00)>>8,*ptRegX,(tOpCode&0x00F0)>>4,tRegY);
  switch((tOpCode&0xF))
  {
    case 0x0: /* MOV */
      TRACE_DBG_INFO("MOV");
      *ptRegX=tRegY;
      break;
    case 0x1: /* OR */
      TRACE_DBG_INFO("OR");
      *ptRegX|=tRegY;
      break;
    case 0x2: /* AND */
      TRACE_DBG_INFO("AND");
      *ptRegX&=tRegY;
      break;
    case 0x3: /* XOR */
      TRACE_DBG_INFO("XOR");
      *ptRegX^=tRegY;
      break;
    case 0x4: /* ADD */
      TRACE_DBG_INFO("ADD");
      iTmp=*ptRegX+tRegY;
      tagChipStatus_m.taChip8_RegistersGeneral[0xF]=((iTmp>255)?1:0); /* Set overflow indication on VF */
      *ptRegX=(iTmp&0xFF);
      break;
    case 0x5: /* SUB */
      TRACE_DBG_INFO("SUB");
      tagChipStatus_m.taChip8_RegistersGeneral[0xF]=((*ptRegX>tRegY)?1:0); /* Set underflow indication on VF */
      *ptRegX=*ptRegX-tRegY;
      *ptRegX>>=((sizeof(byte)*CHAR_BIT)-8); /* Correct if byte != 8 bits */
      break;
    case 0x6: /* SHR */
      TRACE_DBG_INFO("SHR");
      tagChipStatus_m.taChip8_RegistersGeneral[0xF]=(*ptRegX&0x1); /* If LSB (0000 0001) is set, set VF=1, otherways VF=0 */
      *ptRegX>>=1;
      break;
    case 0x7: /* SUBN */
      TRACE_DBG_INFO("SUBN");
      tagChipStatus_m.taChip8_RegistersGeneral[0xF]=((tRegY>*ptRegX)?1:0); /* Set underflow indication on VF */
      *ptRegX=tRegY-*ptRegX;
      *ptRegX>>=((sizeof(byte)*CHAR_BIT)-8); /* Correct if byte != 8 bits */
      break;
    case 0xE: /* SHL */
      TRACE_DBG_INFO("SHL");
      tagChipStatus_m.taChip8_RegistersGeneral[0xF]=(*ptRegX&0x80); /* If MSB (1000 0000) is set, set VF=1, otherways VF=0 */
      *ptRegX<<=1;
      break;
    default: /* Unknown Instruction code */
      fprintf(stderr,"Unknown Mathematical instruction: X: 0x%.4X, Y: 0x%.4X\n",*ptRegX,tRegY);
      return;
  }
  CHIP8_PROGRAMCOUNTER_INCREMENT();
}

INLINE_FCT void vOp_Sys_SKNEQ_reg_m(byte tRegX, byte tRegY) /* 0x9xy0 */
{
  if(tagChipStatus_m.taChip8_RegistersGeneral[tRegX]!=tagChipStatus_m.taChip8_RegistersGeneral[tRegY])
  {
    TRACE_DBG_INFO_VARG("Skipping next instruction, Reg[%u]=%u, Reg[%u]=%u\n",
                        tRegX,
                        tagChipStatus_m.taChip8_RegistersGeneral[tRegX],
                        tRegY,
                        tagChipStatus_m.taChip8_RegistersGeneral[tRegY]);
    CHIP8_PROGRAMCOUNTER_SKIP_NEXT();
    return;
  }
  CHIP8_PROGRAMCOUNTER_INCREMENT();
  TRACE_DBG_INFO("Can't skip, Registers are Equal");
}

INLINE_FCT void vOp_Sys_MOV_I_m(word tOpCode) /* 0xAnnn */
{
  tagChipStatus_m.tChip8_RegisterI=(tOpCode&0x0FFF);
  TRACE_DBG_INFO_VARG("Set Register I=%u\n",tagChipStatus_m.tChip8_RegisterI);
  CHIP8_PROGRAMCOUNTER_INCREMENT();
}

INLINE_FCT void vOp_Sys_JMP_V0_m(word tOpCode) /* 0xBnnn */
{
  tOpCode&=0x0FFF;
  tOpCode+=tagChipStatus_m.taChip8_RegistersGeneral[0];
  if(CHIP8_JMP_ADDR_VALID(tOpCode))
  {
    tagChipStatus_m.tChip8_RegisterProgramCounter=tOpCode;
    CHIP8_PROGRAMCOUNTER_INCREMENT();
    return;
  }
  TRACE_DBG_ERROR_VARG("JMP failed, addr 0x%.4X not valid",tOpCode);
}

INLINE_FCT void vOp_Sys_RAND_m(byte tRegX, byte tValue) /* 0xCxkk */
{
  TRACE_DBG_INFO_VARG("RAND()&0x%.2X @Register[%u]\n",tValue,tRegX);
  tagChipStatus_m.taChip8_RegistersGeneral[tRegX]=(rand()%0x100)&tValue;
  CHIP8_PROGRAMCOUNTER_INCREMENT();
}

void vChip8_DumpScreen_g(void)
{
  int iIndex, iIndexB;
  char caTextBuffer[CHIP8_SCREEN_WIDTH+3]={'|'};

  caTextBuffer[CHIP8_SCREEN_WIDTH+1]='|';
  caTextBuffer[CHIP8_SCREEN_WIDTH+2]='\0';
  puts("Current video screen:\n"
       "------------------------------------------------------------------");
  for(iIndex=0;iIndex<CHIP8_SCREEN_HEIGHT;++iIndex)
  {
//  putchar('|');
    for(iIndexB=0;iIndexB<CHIP8_SCREEN_WIDTH;++iIndexB)
    {
      caTextBuffer[iIndexB+1]=((taScreenBuffer_m[iIndexB/8][iIndex]&(1<<(7-iIndexB%8)))?'1':'0');
//    putchar((taScreenBuffer_m[iIndexB/8][iIndex]&(1<<(7-iIndexB%8)))?'1':'0'); /* bitorder: 76543210 */
//    putchar((taScreenBuffer_m[iIndexB/8][iIndex]&(1<<(iIndexB%8)))?'1':'0'); /* bitorder: 01234567 */
    }
    puts(caTextBuffer);
  }
  puts("------------------------------------------------------------------");
}

INLINE_FCT void vOp_Sys_SPRITE_m(byte tPosX, byte tPosY, byte tBytes) /* 0xDxyn */
{
  unsigned int uiIndex;
  byte tScreenTmp;
  TRACE_DBG_INFO_VARG("Drawing %u bytes from 0x%.4X @Pos(%u/%u)\n",tBytes,tagChipStatus_m.tChip8_RegisterI,tPosX,tPosY);
  if((!CHIP8_JMP_ADDR_VALID(tagChipStatus_m.tChip8_RegisterI)) ||
     (tagChipStatus_m.tChip8_RegisterI+tBytes>CHIP8_MEMORY_SIZE))
  {
    TRACE_DBG_ERROR("Drawing cancelled, memory would overflown");
    return;
  }
  if(tPosX>CHIP8_SCREEN_WIDTH-1)
  {
    TRACE_DBG_ERROR_VARG("Screen overflow in X-Direction (%u allowed, is: %u)\n",CHIP8_SCREEN_WIDTH-1,tPosX);
    return;
  }
  tagChipStatus_m.taChip8_RegistersGeneral[0xF]=0;
  for(uiIndex=0;uiIndex<tBytes;++uiIndex)
  {
    if(tPosY+uiIndex>CHIP8_SCREEN_HEIGHT-1)
    {
      TRACE_DBG_INFO_VARG("Ignoring Y-Overflow Drawing(Allowed: %u, is: %u)\n",CHIP8_SCREEN_HEIGHT-1,tPosY+uiIndex);
      break;
    }
    /* Bitorder 76543210 */
    tScreenTmp=taScreenBuffer_m[tPosX/8][tPosY+uiIndex];
    taScreenBuffer_m[tPosX/8][tPosY+uiIndex]^=((taChip8_Memory_g[tagChipStatus_m.tChip8_RegisterI+uiIndex])>>((tPosX%8)));


    if((!tagChipStatus_m.taChip8_RegistersGeneral[0xF]) &&
       (CHIP8_CHECK_PIXEL_EREASED(taScreenBuffer_m[tPosX/8][tPosY+uiIndex],tScreenTmp))
       )
    {
      printf("VF set to 1!\n");// TODO: set VF=1 if XOR causes pixel to be ereased, else set VF=0
      getchar();
      tagChipStatus_m.taChip8_RegistersGeneral[0xF]=1;
    }
    /* Check for Screen overflow */
    if((tPosX+uiIndex>CHIP8_SCREEN_WIDTH-9))
    {
      if(tPosY+tBytes+uiIndex>=CHIP8_SCREEN_HEIGHT)
        TRACE_DBG_INFO_VARG("(Wraparound) Ignoring Y-Overflow Drawing: [0][%u (%u allowed)]\n",tPosY+tBytes+uiIndex,CHIP8_SCREEN_HEIGHT-1);
      else
      {
        taScreenBuffer_m[0][tPosY+tBytes+uiIndex]^=((taChip8_Memory_g[tagChipStatus_m.tChip8_RegisterI+uiIndex])<<(8-(tPosX%8)));
        if((!tagChipStatus_m.taChip8_RegistersGeneral[0xF]) &&
           (CHIP8_CHECK_PIXEL_EREASED(taScreenBuffer_m[tPosX/8][tPosY+uiIndex],tScreenTmp))
           )
        {
          printf("VF set to 1!\n");// TODO: set VF=1 if XOR causes pixel to be ereased, else set VF=0
          getchar();
          tagChipStatus_m.taChip8_RegistersGeneral[0xF]=1;
        }
        TRACE_DBG_INFO_VARG("(Wraparound) taScreenBuffer_m[%u][%u]=0x%.2X / [0][%u]=0x%.2X @0x%.2X(=0x%.2X)\n",
                            tPosX/8,
                            tPosY+uiIndex,
                            taScreenBuffer_m[tPosX/8][tPosY+uiIndex],
                            tPosY+tBytes+uiIndex,
                            taScreenBuffer_m[0][tPosY+tBytes+uiIndex],
                            tagChipStatus_m.tChip8_RegisterI+uiIndex,
                            taChip8_Memory_g[tagChipStatus_m.tChip8_RegisterI+uiIndex]);
      }
    }
    else
    {
      taScreenBuffer_m[tPosX/8+1][tPosY+uiIndex]^=((taChip8_Memory_g[tagChipStatus_m.tChip8_RegisterI+uiIndex])<<(8-(tPosX%8)));
      if((!tagChipStatus_m.taChip8_RegistersGeneral[0xF]) &&
         (CHIP8_CHECK_PIXEL_EREASED(taScreenBuffer_m[tPosX/8][tPosY+uiIndex],tScreenTmp))
         )
      {
        printf("VF set to 1!\n");// TODO: set VF=1 if XOR causes pixel to be ereased, else set VF=0
        getchar();
        tagChipStatus_m.taChip8_RegistersGeneral[0xF]=1;
      }
      TRACE_DBG_INFO_VARG("taScreenBuffer_m[%u][%u]=0x%.2X / [%u][%u]=0x%.2X @0x%.2X(=0x%.2X)\n",
                          tPosX/8,
                          tPosY+uiIndex,
                          taScreenBuffer_m[tPosX/8][tPosY+uiIndex],
                          tPosX/8+1,
                          tPosY+uiIndex,
                          taScreenBuffer_m[tPosX/8+1][tPosY+uiIndex],
                          tagChipStatus_m.tChip8_RegisterI+uiIndex,
                          taChip8_Memory_g[tagChipStatus_m.tChip8_RegisterI+uiIndex]);
    }

  }
  CHIP8_PROGRAMCOUNTER_INCREMENT();
}

INLINE_FCT void vOp_Sys_KEY_m(word tOpCode) /* 0xEx9E, 0xExA1 */
{
  unsigned short usKeyMap;
  byte tKey=tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>12];
  if(tKey>0xF)
  {
    TRACE_DBG_ERROR_VARG("Invalid Key in Register[%u]: %u, allowed: 0-F\n",(tOpCode&0x0F00)>>12,tKey);
    tKey&=0xF;
    return;   // TODO: howto?
  }
  vChip8_Keys_GetState_g(&usKeyMap);
  if((tOpCode&0x00FF)==CHIP8_CMD_KEY_PRESSED)
  {
    TRACE_DBG_INFO_VARG("Checking for key %X to be pressed...\n",tKey);
    if((usKeyMap&(1<<tKey)))
    {
      TRACE_DBG_INFO("Key is pressed, skipping next instruction");
      CHIP8_PROGRAMCOUNTER_SKIP_NEXT();
      return;
    }
    TRACE_DBG_INFO("Key is not pressed");
  }
  else if((tOpCode&0x00FF)==CHIP8_CMD_KEY_RELEASED)
  {
    TRACE_DBG_INFO_VARG("Checking for key %X to be released...\n",tKey);
    if(!(usKeyMap&(1<<tKey)))
    {
      TRACE_DBG_INFO("Key is released, skipping next instruction");
      CHIP8_PROGRAMCOUNTER_SKIP_NEXT();
      return;
    }
    TRACE_DBG_INFO("Key is not released");
  }
  else
  {
    vChip8_UnknownInstruction_m(tOpCode);
    return;
  }
  CHIP8_PROGRAMCOUNTER_INCREMENT();
}

INLINE_FCT void vOp_Sys_MISC_m(word tOpCode) /* 0xFxxx */
{
  unsigned int uiIndex;
  switch((tOpCode&0x00FF))
  {
    case CHIP8_CMD_DELAY_TIMER_GET:
      TRACE_DBG_INFO_VARG("Storing DelayTimer val @Reg[%u]\n",(tOpCode&0x0F00)>>8);
      tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8]=tagChipStatus_m.tChip8_RegisterDelayTimer;
      break;
    case CHIP8_CMD_WAIT_FOR_KEY:
      TRACE_DBG_INFO_VARG("Waiting for Key and Store @Reg[%u]\n",(tOpCode&0x0F00)>>8);
      tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8]=ucChip8_Keys_WaitForNext_g();
      break;
    case CHIP8_CMD_DELAY_TIMER_SET:
      TRACE_DBG_INFO_VARG("Set DelayTimer Val to Reg[%u]=0x%.2X\n",(tOpCode&0x0F00)>>8,tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8]);
      tagChipStatus_m.tChip8_RegisterDelayTimer=tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8];
      break;
    case CHIP8_CMD_SOUND_TIMER_SET:
      TRACE_DBG_INFO_VARG("Set SoundTimer Val to Reg[%u]=0x%.2X\n",(tOpCode&0x0F00)>>8,tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8]);
      tagChipStatus_m.tChip8_RegisterSoundTimer=tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8];
      break;
    case CHIP8_CMD_I_ADD_VX:
      TRACE_DBG_INFO_VARG("Add Reg[%u] to I(=%u)\n",(tOpCode&0x0F00)>>8,tagChipStatus_m.tChip8_RegisterI);
      tagChipStatus_m.tChip8_RegisterI+=tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8];
      break;
    case CHIP8_CMD_I_SPRITE_GET:
      TRACE_DBG_INFO_VARG("Setting I on Pos for Sprite '%X'@0x%.4X\n",(tOpCode&0x0F00)>>8,5*((tOpCode&0x0F00)>>8));
      tagChipStatus_m.tChip8_RegisterI=5*((tOpCode&0x0F00)>>8);
      break;
    case CHIP8_CMD_VX_GET_BCD:
      TRACE_DBG_INFO_VARG("BCD Reg[%u] -> I\n",(tOpCode&0x0F00)>>8);
      if(!tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8])
      {
        TRACE_DBG_INFO("Register=0, setting I=0");
        tagChipStatus_m.tChip8_RegisterI=0;
      }
      else
      {
        tagChipStatus_m.tChip8_RegisterI=(tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8])/100;
        tagChipStatus_m.tChip8_RegisterI|=(((tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8])/10)%10)<<4;
        tagChipStatus_m.tChip8_RegisterI|=(((tagChipStatus_m.taChip8_RegistersGeneral[(tOpCode&0x0F00)>>8])%100)%10)<<8;
        TRACE_DBG_INFO_VARG("Set I=0x%.4X\n",tagChipStatus_m.tChip8_RegisterI);
      }
      break;
    case CHIP8_CMD_REGISTERS_STORE:
      TRACE_DBG_INFO_VARG("Storing Register V0-V%X in memory @0x%.4X-0x%.4X\n",
                          (tOpCode&0x0F00)>>8,
                          tagChipStatus_m.tChip8_RegisterI,
                          tagChipStatus_m.tChip8_RegisterI+((tOpCode&0x0F00)>>8));

      if((!CHIP8_JMP_ADDR_VALID(tagChipStatus_m.tChip8_RegisterI)) ||
         (tagChipStatus_m.tChip8_RegisterI+((tOpCode&0x0F00)>>8)>CHIP8_MEMORY_SIZE))
      {
        TRACE_DBG_ERROR("Can't store Registers, memory would overflow");
        return;
      }
      for(uiIndex=0;uiIndex<=(tOpCode&0x0F00)>>8;++uiIndex)
      {
        taChip8_Memory_g[tagChipStatus_m.tChip8_RegisterI+uiIndex]=tagChipStatus_m.taChip8_RegistersGeneral[uiIndex];
      }
      break;
    case CHIP8_CMD_REGISTERS_READ:
      TRACE_DBG_INFO_VARG("Reading Register V0-V%X from memory @0x%.4X-0x%.4X\n",
                          (tOpCode&0x0F00)>>8,
                          tagChipStatus_m.tChip8_RegisterI,
                          tagChipStatus_m.tChip8_RegisterI+((tOpCode&0x0F00)>>8));

      if((!CHIP8_JMP_ADDR_VALID(tagChipStatus_m.tChip8_RegisterI)) ||
         (tagChipStatus_m.tChip8_RegisterI+((tOpCode&0x0F00)>>8)>CHIP8_MEMORY_SIZE))
      {
        TRACE_DBG_ERROR("Can't read memory, invalid address");
        return;
      }
      for(uiIndex=0;uiIndex<=(tOpCode&0x0F00)>>8;++uiIndex)
      {
        tagChipStatus_m.taChip8_RegistersGeneral[uiIndex]=taChip8_Memory_g[tagChipStatus_m.tChip8_RegisterI+uiIndex];
      }
      break;
    default:
      vChip8_UnknownInstruction_m(tOpCode);
      return;
  }
  CHIP8_PROGRAMCOUNTER_INCREMENT();
}

INLINE_FCT void vChip8_UnknownInstruction_m(word tInstruction)
{
  fprintf(stderr,"***Unknown instruction: 0x%.4X***\n",tInstruction);
}

void ThreadChip8_FreqFunc_m(void)
{
  if(tagChipStatus_m.tChip8_RegisterSoundTimer)
  {
    if(!(--tagChipStatus_m.tChip8_RegisterSoundTimer))
      vChip8_Sound_Stop_g();
  }
  if(tagChipStatus_m.tChip8_RegisterDelayTimer)
  {
    --tagChipStatus_m.tChip8_RegisterDelayTimer;
  }
}

