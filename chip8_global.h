#ifndef CHIP8_GLOBAL_H_INCLUDED
  #define CHIP8_GLOBAL_H_INCLUDED

#ifdef _WIN32
  //TODO: windows quick wait event
#elif __unix__
  #include <features.h>
  #if defined (_POSIX_C_SOURCE) && (_POSIX_C_SOURCE>=199309L)
  #include <time.h> /* For nanosleep */
  #else
    #error No microsleep available for your platform
  #endif
#endif /* _WIN32 */

#ifndef SDL_MAIN_HANDLED
  #define SDL_MAIN_HANDLED
#endif /* !SDL_MAIN_HANDLED */
#include <SDL2/SDL.h>

#include "chip8_public.h"

#define SCREEN_CALC_INDEX(x,y) ((y)*(CHIP8_SCREEN_WIDTH/8)+((x)/8))

/* Various numeric constants used for the emulator */
enum
{
  CHIP8_SCREEN_WIDTH           =64,
  CHIP8_SCREEN_HEIGHT          =32,
  CHIP8_SCREEN_DEFAULT_SCALE   = 5,

  CHIP8_FREQ_RUN_HZ            =540,
  CHIP8_FREQ_TIMER_DELAY_HZ    =60,
  CHIP8_FREQ_TIMER_SOUND_HZ    =60,

  CHIP8_SOUND_FREQ             =440,

  EMU_PLAYSOUND_CHANNELS       =1,
  EMU_PLAYSOUND_SAMPLE_RATE_HZ =44100,
  EMU_PLAYSOUND_SAMPLES_BUFFER =1024,
  EMU_PLAYSOUND_AMPLITUDE      =3,     /* 1-127, but 3 is absolutely fine imo */

  EMU_KEY_MASK_0               =0x0001,
  EMU_KEY_MASK_1               =0x0002,
  EMU_KEY_MASK_2               =0x0004,
  EMU_KEY_MASK_3               =0x0008,
  EMU_KEY_MASK_4               =0x0010,
  EMU_KEY_MASK_5               =0x0020,
  EMU_KEY_MASK_6               =0x0040,
  EMU_KEY_MASK_7               =0x0080,
  EMU_KEY_MASK_8               =0x0100,
  EMU_KEY_MASK_9               =0x0200,
  EMU_KEY_MASK_A               =0x0400,
  EMU_KEY_MASK_B               =0x0800,
  EMU_KEY_MASK_C               =0x1000,
  EMU_KEY_MASK_D               =0x2000,
  EMU_KEY_MASK_E               =0x4000,
  EMU_KEY_MASK_F               =0x8000,

  EMU_KEY_MASK_ESC             =0x80000000
};

#define EMU_UPD_SET_FLAG(emu,flag) ((emu)->uiSettingsChanged)|=flag
#define EMU_UPD_CHK_FLAG(emu,flag) (((emu)->uiSettingsChanged)&flag)
#define EMU_UPD_RST_FLAG(emu,flag) (((emu)->uiSettingsChanged)&=~(flag))

typedef enum
{
  THREAD_UPDATE_STATE_INACTIVE,
  THREAD_UPDATE_STATE_ACTIVE,
  THREAD_UPDATE_STATE_PAUSED,
  THREAD_UPDATE_STATE_QUIT,
  THREAD_UPDATE_STATE_ERROR,
}ESDLUpdateThread;

enum EmuSettingsUpdateFlags
{
  EMU_UPDATE_SETTING_EMULATION_SPEED    = 0x000001,

  EMU_UPDATE_SETTING_SCREEN_SCALE       = 0x000010,
  EMU_UPDATE_SETTING_SCREEN_POS         = 0x000020,
  EMU_UPDATE_SETTING_SCREEN_SHOW        = 0x000040,
  EMU_UPDATE_SETTING_SCREEN_HIDE        = 0x000080,
  EMU_UPDATE_SETTING_SCREEN_CLEAR       = 0x000100,

  EMU_UPDATE_SETTING_RUNTHREAD_RUN      = 0x001000,
  EMU_UPDATE_SETTING_RUNTHREAD_PAUSE    = 0x002000,
  EMU_UPDATE_SETTING_RUNTHREAD_QUIT     = 0x004000,

  EMU_UPDATE_SETTING_UPDATETHREAD_RUN   = 0x010000,
  EMU_UPDATE_SETTING_UPDATETHREAD_PAUSE = 0x020000,
  EMU_UPDATE_SETTING_UPDATETHREAD_QUIT  = 0x040000,

  EMU_UPDATE_SETTING_KEYBOARD_KEYMAP    = 0x100000
};

enum Errors /* Return codes from emulator */
{
  ERR_NONE=0,
  ERR_INVALID_JUMP_ADDRESS,
  ERR_MEM_WOULD_OVERFLOW,
  ERR_FONT_OUT_OF_INDEX,
  ERR_SCREEN_X_OVERFLOW,
  ERR_SCREEN_DRAW,
  ERR_STACK_MAX_CALLS,
  ERR_STACK_ON_TOP,
  ERR_KEYCODE_INVALID,
  ERR_INSTRUCTION_UNKNOWN,
};

/**
 * Memory Map for CHIP-8 Emulator
 *
 * |-----------------| 0x0   (0)------|
 * | Program memory  |                |
 * | for sprites     |                |
 * | (reserved)      |                |
 * |                 |                |
 * |-----------------| 0x50  (80)     |
 * | unused          |                |
 * | (reserved)      |                |
 * |                 |                |
 * |-----------------| 0xFF  (255)    |
 * | Registers       |                |
 * | Program Counter | 0x100 (256)    |
 * | Stack Pointer   | 0x102 (258)    |-- 0x0 (0) - 0x1FF (511):
 * | Resgister I     | 0x104 (260)    |-- Reserved for interpreter
 * | Delay Timer     | 0x106 (262)    |
 * | Sound Timer     | 0x107 (263)    |
 * | General Purpose | 0x108 (264) -  |
 * | 8-bit Registers |                |
 * | 16x (V0-VF)     |                |
 * |                 |                |
 * |-----------------| 0x117 (279)    |
 * | unused          |                |
 * | (reserved)      |                |
 * |                 |                |
 * |-----------------| 0x1FF (511)----|
 * | Data            |                |-- 0x200 (512) - 0xE9F (3743)
 * |                 |                |-- Used for Programs
 * |-----------------| 0xE9F (3743)---|
 * | Call Stack      |                |
 * | (reserved)      |                |
 * |                 |                |-- 0xEA0 (3744) - 0xFFF (4095)
 * |-----------------| 0xEFF (3839)   |-- reserved for interpreter
 * [ Screen refresh  |                |
 * | (reserved)      |                |
 * |                 |                |
 * |-----------------| 0xFFF (4095)---|
 *
 */
enum /* Offset addresses in memory */
{
  OFF_ADDR_FONT_START   = 0x0,
  OFF_ADDR_FONT_END     = 0x50,
  OFF_REG_PC            = 0x100,
  OFF_REG_PTR_STACK     = OFF_REG_PC+sizeof(word),
  OFF_REG_I             = OFF_REG_PTR_STACK+sizeof(word),
  OFF_REG_TMRDEL        = OFF_REG_I+sizeof(word),
  OFF_REG_TMRSND        = OFF_REG_TMRDEL+sizeof(byte),
  OFF_REG_V0            = OFF_REG_TMRSND+sizeof(byte),
  OFF_REG_V1,
  OFF_REG_V2,
  OFF_REG_V3,
  OFF_REG_V4,
  OFF_REG_V5,
  OFF_REG_V6,
  OFF_REG_V7,
  OFF_REG_V8,
  OFF_REG_V9,
  OFF_REG_VA,
  OFF_REG_VB,
  OFF_REG_VC,
  OFF_REG_VD,
  OFF_REG_VE,
  OFF_REG_VF,
  OFF_ADDR_USER_START   = 0x200,
  OFF_ADDR_USER_END     = 0xE9F,
  OFF_ADDR_STACK_START  = OFF_ADDR_USER_END+1,
  OFF_ADDR_STACK_END    = 0xEFF,
  OFF_ADDR_SCREEN_START = OFF_ADDR_STACK_END+1,
  OFF_ADDR_SCREEN_END   = 0xFFF,
  OFF_MEM_SIZE          = 0x1000 /* 4096 */
};

/**
 * processThreadFunc  Functionpointer to call for processing 1 instruction
 * pRegTimerDelay     Pointer to Delay timer Register
 * pRegTimerSound     Pointer to Sound Timer Register
 * iWindowScale       Scale factor for window
 * eSpeed             Speed factor
 */

typedef struct TagEmulator_T       TagEmulator;
typedef struct TagWindow_T         TagWindow;
typedef struct TagPlaySound_T      TagPlaySound;
typedef struct TagKeyboard_T       TagKeyboard;
typedef struct TagThreadEmu_T      TagThreadEmu;
typedef struct TagThreadRenderer_T TagThreadRenderer;

typedef int(*processThreadFunc)(TagEmulator *pEmulator);

struct TagEmulator_T
{
  /* Volatile needed, otherwise gcc removes waitloops for optimization */
  volatile unsigned int uiSettingsChanged;
  word tCurrOPCode;
  byte taChipMemory[OFF_MEM_SIZE];
  SDL_mutex *pMutex;
  struct TagWindow_T
  {
    int iPosX;
    int iPosY;
    int iMonitorWidth;
    int iMonitorHeigth;
    int iMonitorHz;
    unsigned int uiScale;
    int iVisible;
    byte taLastScreen[(CHIP8_SCREEN_WIDTH/8) * CHIP8_SCREEN_HEIGHT];
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
  }tagWindow;
  struct TagPlaySound_T
  {
    int iSoundPlaying;
    unsigned int uiFreqSoundHz;
    unsigned int uiUsedSoundSize;
    char caSoundBuffer[EMU_PLAYSOUND_SAMPLE_RATE_HZ];
    SDL_AudioDeviceID tDevID;
  }tagPlaySound;
  struct TagKeyboard_T
  {
    unsigned long ulKeyStates;
    unsigned long ulKeyStatesLast;
    unsigned char ucaUsrKeyMap[EMU_KEY_COUNT];
    unsigned short ausKeyMap[EMU_KEY_COUNT];
  }tagKeyboard;
  struct TagThreadEmu_T
  {
    EEmulatorState eEmuState;
    processThreadFunc procFunc;
    pCallbackFunc usrCBFunc;
    unsigned int usrCBEvents;
    EEmulationSpeed eSpeed;
    SDL_Thread *pThread;
  }tagThrdEmu;
  struct TagThreadRenderer_T
  {
    ESDLUpdateThread eThreadState;
    SDL_Thread *pThread;
  }tagThrdRenderer;
};

extern TagEmulator tagEmulator_g;

#define REG_PC        *(word*)(&tagEmulator_g.taChipMemory[OFF_REG_PC])
#define REG_PTR_STACK *(word*)(&tagEmulator_g.taChipMemory[OFF_REG_PTR_STACK])
#define REG_I         *(word*)(&tagEmulator_g.taChipMemory[OFF_REG_I])
#define REG_TMRDEL    tagEmulator_g.taChipMemory[OFF_REG_TMRDEL]
#define REG_TMRSND    tagEmulator_g.taChipMemory[OFF_REG_TMRSND]
#define REG_VX(index) tagEmulator_g.taChipMemory[OFF_REG_V0+(index)]
#define REG_V0        tagEmulator_g.taChipMemory[OFF_REG_V0]
#define REG_V1        tagEmulator_g.taChipMemory[OFF_REG_V1]
#define REG_V2        tagEmulator_g.taChipMemory[OFF_REG_V2]
#define REG_V3        tagEmulator_g.taChipMemory[OFF_REG_V3]
#define REG_V4        tagEmulator_g.taChipMemory[OFF_REG_V4]
#define REG_V5        tagEmulator_g.taChipMemory[OFF_REG_V5]
#define REG_V6        tagEmulator_g.taChipMemory[OFF_REG_V6]
#define REG_V7        tagEmulator_g.taChipMemory[OFF_REG_V7]
#define REG_V8        tagEmulator_g.taChipMemory[OFF_REG_V8]
#define REG_V9        tagEmulator_g.taChipMemory[OFF_REG_V9]
#define REG_VA        tagEmulator_g.taChipMemory[OFF_REG_VA]
#define REG_VB        tagEmulator_g.taChipMemory[OFF_REG_VB]
#define REG_VC        tagEmulator_g.taChipMemory[OFF_REG_VC]
#define REG_VD        tagEmulator_g.taChipMemory[OFF_REG_VD]
#define REG_VE        tagEmulator_g.taChipMemory[OFF_REG_VE]
#define REG_VF        tagEmulator_g.taChipMemory[OFF_REG_VF]


/* Enable/disable trace here as needed */
#define TRACE_DEBUG_INFO
#define TRACE_DEBUG_ERROR
#define TRACE_INSTRUCTIONS

#ifdef TRACE_DEBUG_INFO
  #define TRACE_DBG_INFO(txt)            fputs("INFO: " txt "\n", stdout)
  #define TRACE_DBG_INFO_VARG(txt,...)   fprintf(stdout,"INFO: " txt "\n",__VA_ARGS__)
#else
  #define TRACE_DBG_INFO(txt)
  #define TRACE_DBG_INFO_VARG(txt,...)
#endif /* TRACE_DEBUG_INFO */

#ifdef TRACE_DEBUG_ERROR
  #define TRACE_DBG_ERROR(txt)           fputs("ERROR: " txt "\n", stderr)
  #define TRACE_DBG_ERROR_VARG(txt,...)  fprintf(stderr, "ERROR: " txt "\n",__VA_ARGS__)
#else
  #define TRACE_DBG_ERROR(txt)
  #define TRACE_DBG_ERROR_VARG(txt,...)
#endif /* TRACE_DEBUG_ERROR */

#ifdef TRACE_INSTRUCTIONS
  #define TRACE_INSTR(txt,...)      fprintf(stdout, txt "\n",__VA_ARGS__)
#else
  #define TRACE_INSTR(txt,...)
#endif /* TRACE_DEBUG_INFO */

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* >= C99 */
  #define INLINE_FCT inline
  #define INLINE_PROT static inline
#else /* No inline available from C Standard */
  #define INLINE_FCT static
  #define INLINE_PROT static
#endif /* __STDC_VERSION__ >= C99 */

#endif /* CHIP8_GLOBAL_H_INCLUDED */
