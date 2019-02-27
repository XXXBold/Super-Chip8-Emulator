#ifndef CHIP8_PUBLIC_H_INCLUDED
  #define CHIP8_PUBLIC_H_INCLUDED

typedef unsigned char byte;
typedef unsigned short word;

/**
 * Callbackfunction defined by the user.
 *
 */
typedef int(*pCallbackFunc)(unsigned int event, word opCode);

enum EmuKeys
{
  EMU_KEY_0,
  EMU_KEY_1,
  EMU_KEY_2,
  EMU_KEY_3,
  EMU_KEY_4,
  EMU_KEY_5,
  EMU_KEY_6,
  EMU_KEY_7,
  EMU_KEY_8,
  EMU_KEY_9,
  EMU_KEY_A,
  EMU_KEY_B,
  EMU_KEY_C,
  EMU_KEY_D,
  EMU_KEY_E,
  EMU_KEY_F,
  EMU_KEY_G,
  EMU_KEY_H,
  EMU_KEY_I,
  EMU_KEY_J,
  EMU_KEY_K,
  EMU_KEY_L,
  EMU_KEY_M,
  EMU_KEY_N,
  EMU_KEY_O,
  EMU_KEY_P,
  EMU_KEY_Q,
  EMU_KEY_R,
  EMU_KEY_S,
  EMU_KEY_T,
  EMU_KEY_U,
  EMU_KEY_V,
  EMU_KEY_W,
  EMU_KEY_X,
  EMU_KEY_Y,
  EMU_KEY_Z,
  EMU_KEY_UP,
  EMU_KEY_DOWN,
  EMU_KEY_LEFT,
  EMU_KEY_RIGHT,
  EMU_KEY_DEFAULT = 0xFF
};

enum UsrCBEvent
{
  EMU_EVT_INSTRUCTION_EXECUTED = 0x01,
  EMU_EVT_INSTRUCTION_UNKNOWN  = 0x02,
  EMU_EVT_INSTRUCTION_ERROR    = 0x04,

  EMU_EVT_KEYPRESS_ESCAPE      = 0x100,

  EMU_EVT_ALL                  = 0xFFFF
};

typedef enum
{
  EMU_STATE_INACTIVE,
  EMU_STATE_RUN,
  EMU_STATE_PAUSE,
  EMU_STATE_QUIT,
  EMU_STATE_ERROR_INSTRUCTION
}EEmulatorState;

typedef enum
{
  EMU_SPEED_0_5X, /* Speed x 0.5 */
  EMU_SPEED_1_0X, /* Speed x 1.0, 540Hz (original) */
  EMU_SPEED_1_5X, /* Speed x 1.5,  */
  EMU_SPEED_2_0X, /* Speed x 2.0 */
}EEmulationSpeed;

#endif /* CHIP8_PUBLIC_H_INCLUDED */
