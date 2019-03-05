#ifndef CHIP8_PUBLIC_H_INCLUDED
  #define CHIP8_PUBLIC_H_INCLUDED

typedef unsigned char byte;
typedef unsigned short word;

/**
 * Callbackfunction defined by the user.
 *
 */
typedef int(*pCallbackFunc)(unsigned int event, word opCode);

enum
{
  /**
   * Emulator has 16 Keys (0..9, A..F)
   */
  EMU_KEY_COUNT=0x10
};

enum EmuKeys
{
  EMU_KEY_0=0,
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

/**
 * Events for pCallbackFunc, can be OR-ed together, as needed.
 */
enum UsrCBEvent
{
  /**
   * Calls pCallbackFunc on every Instruction which gets executed by the Emulator. <br>
   * If using this, make sure callbacks are handled very quick, otherwise the Emulator <br>
   * could slow down in it's execution speed.
   */
  EMU_EVT_INSTRUCTION_EXECUTED = 0x01,
  /**
   * Calls pCallbackFunc if the current instruction code is unknown.
   */
  EMU_EVT_INSTRUCTION_UNKNOWN  = 0x02,
  /**
   * Calls pCallbackFunc if the current Instruction code would cause an error. <br>
   * Possible Errors are e.g. Buffer overflow, Invalid values, etc.
   */
  EMU_EVT_INSTRUCTION_ERROR    = 0x04,

  /**
   * Calls pCallbackFunc if Escape is pressed. This can be used to pause the Emulator, <br>
   * but do not call any chip8_ Functions directly in the callback, this could cause a deadlock.
   */
  EMU_EVT_KEYPRESS_ESCAPE      = 0x100,

  /**
   * Calls pCallbackFunc if any of the event occurs.
   */
  EMU_EVT_ALL                  = 0xFFFF
};


/**
 * These are the diffrent possible Emulator states.
 */
typedef enum
{
  /**
   * Emulator is currently inactive. This should not be the case after chip8_Init <br>
   * was called successful.
   */
  EMU_STATE_INACTIVE,
  /**
   * Emulator is active and currently running.
   */
  EMU_STATE_RUN,
  /**
   * Emulator is in paused state.
   */
  EMU_STATE_PAUSE,
  /**
   * Emulator was Quit, this should not happen until chip8_Close is called.
   */
  EMU_STATE_QUIT,
  /**
   * An Instruction couldn't be processed and Emulator was stopped for now. <br>
   * This is technically the same state as EMU_STATE_PAUSE, but can't be resumed <br>
   * without further investigation, or it will fail again immediatly.
   */
  EMU_STATE_ERROR_INSTRUCTION
}EEmulatorState;

/**
 * Available speed modifications
 */
typedef enum
{
  /**
   * Half of the default speed
   */
  EMU_SPEED_0_5X,
  /**
   * Original Speed (540 Instructions/s) as mentoined on a few sources.
   */
  EMU_SPEED_1_0X,
  /**
   * 1 1/2 of the original speed
   */
  EMU_SPEED_1_5X,
  /**
   * Double of the original speed
   */
  EMU_SPEED_2_0X,
}EEmulationSpeed;

#endif /* CHIP8_PUBLIC_H_INCLUDED */