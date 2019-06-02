#ifndef CHIP8_PUBLIC_H_INCLUDED
  #define CHIP8_PUBLIC_H_INCLUDED

typedef unsigned char byte;
typedef unsigned short word;

  #define ENABLE_SUPERCHIP_INSTRUCTIONS

/**
 * Callbackfunction defined by the user.
 *
 */
typedef int(*pCallbackFunc)(unsigned int event, word opCode);

enum
{
  /**
   * This will Increment Register I if values are stored from or read to Registers from Memory, <br>
   * Applies to Instructions: 0xFX55, 0xFX65
   */
  EMU_QUIRK_INCREMENT_I_ON_STORAGE    =0x1,
  /**
   * On Shift Right or left, this will Shift the Source Register and store it after in the destination Register.
   * Applies to Instructions: 0x8XY, 0x8XY
   */
  EMU_QUIRK_SHIFT_SOURCE_REG          =0x2,
  /**
   * Will simply skip instruction codes which aren't recognized by the emulator.
   */
  EMU_QUIRK_SKIP_INSTRUCTIONS_UNKNOWN =0x4,
  /**
   * Will skip instruction codes, which couldn't be executed because of an error (e.g. memory overflow, invalid values etc)
   */
  EMU_QUIRK_SKIP_INSTRUCTIONS_INVALID =0x8,
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
   * Calls pCallbackFunc on every Chip8-Instruction which gets executed by the Emulator. <br>
   * If using this, make sure callbacks are handled very quick, otherwise the Emulator <br>
   * could slow down in it's execution speed.
   */
  EMU_EVT_CHIP8_INSTRUCTION_EXECUTED  = 0x0001,
  /**
   * Calls pCallbackFunc on every Superchip-Instruction which gets executed by the Emulator. <br>
   * Only used when ENABLE_SUPERCHIP_INSTRUCTIONS is defined.
   */
  EMU_EVT_SUCHIP_INSTRUCTION_EXECUTED = 0x0002,
  /**
   * (Superchip only) Calls pCallbackFunc if a program requests to terminate by itself.
   */
  EMU_EVT_PROGRAM_EXIT                = 0x0010,
  /**
   * Calls pCallbackFunc if the current instruction code is unknown.
   */
  EMU_EVT_ERR_INSTRUCTION_UNKNOWN     = 0x0100,
  /**
   * Calls pCallbackFunc if the current Instruction code would cause an error. <br>
   * Possible Errors are e.g. Buffer overflow, Invalid values, etc.
   */
  EMU_EVT_ERR_INVALID_INSTRUCTION     = 0x0200,
  /**
   * Calls pCallbackFunc if an internal error occured. <br>
   * This might be a bug in the Emulator and should be reported.
   */
  EMU_EVT_ERR_INTERNAL                = 0x0800,
  /**
   * Calls pCallbackFunc if Escape is pressed. This can be used to pause the Emulator, <br>
   * but do not call any chip8_ Functions directly in the callback, this could cause a deadlock.
   */
  EMU_EVT_KEYPRESS_ESCAPE             = 0x1000,
  /**
   * Calls pCallbackFunc if any of the event occurs.
   */
  EMU_EVT_ALL                         = 0xFFFF
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
  EMU_STATE_ERR_INVALID_INSTRUCTION,

  /**
   * The processed Instruction doesn't match a specific Chip8 Instruction. <br>
   * Probably you have to define ENABLE_SUPERCHIP_INSTRUCTIONS to make the Emulator recognize Superchip instructions. <br>
   * If it's already enabled, the ROM itself might be faulty.
   */
  EMU_STATE_ERR_UNKNOWN_INSTRUCTION,
  /**
   * Indicates an internal Error. this should not happen and might be a bug in the Emulator itself. <br>
   * Please report if such an error occurs.
   */
  EMU_STATE_ERR_INTERNAL,
}EEmulatorState;

/**
 * Available speed modifications
 */
typedef enum
{
  /**
   * 1/4 Speed
   */
  EMU_SPEED_0_25X=1,
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
  /**
   * x5
   */
  EMU_SPEED_5_0X,
  /**
   * x10
   */
  EMU_SPEED_10_0X,
}EEmulationSpeed;

#endif /* CHIP8_PUBLIC_H_INCLUDED */
