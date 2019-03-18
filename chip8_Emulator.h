#ifndef CHIP8_EMULATOR_H_INCLUDED
  #define CHIP8_EMULATOR_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "chip8_public.h"

/**
 * Initialises the emulator, pauses execution.
 *
 * @param winPosX Position X of the Emulator Screen, in Monitor coordinates.
 * @param winPosY Position Y of the Emulator Screen, in Monitor coordinates.
 * @param winScaleFactor
 *                Window Scale Factor, 1= Original 64x32.
 *                Pass 0 for Default Scale (=5, means 320x160px).
 * @param keyMap  Keymap for the Keys of the Emulator (0-F). <br>
 *                Pass an array of unsigned char with length of 16 bytes. <br>
 *                The enum EmuKeys (EMU_KEY_) will be set on the corresponding array index, <br>
 *                to keep a single key as default, Pass EMU_KEY_DEFAULT for the index. <br>
 *                Example: <br>
 *                Keymap[0]=EMU_KEY_W -> Map Emulator Key 0 to Keyboard key W <br>
 *                Keymap[1]=EMU_KEY_A -> Map Emulator Key 1 to Keyboard key A <br>
 *                Keymap[2]=EMU_KEY_S -> Map Emulator Key 2 to Keyboard key S <br>
 *                Keymap[3]=EMU_KEY_D -> Map Emulator Key 3 to Keyboard key D <br>
 *                And so on <br>
 *                Pass NULL for default (0=0,1=1,..F=F) Keymap <br>
 * @param cbFunc  Callbackfunction to call when executing an Emulator Instruction. <br>
 *                Do not call any chip8_ functions from this header directly within the callback, <br>
 *                this can cause deadlocking.
 *                Pass NULL if not needed.
 * @param callbackEvents
 *                Define on which Events procFunc should be called.
 *
 * @return 0 on success, nonzero on error.
 * @see chip8_public.h for details
 */
int  chip8_Init(int winPosX,
                int winPosY,
                unsigned char winScaleFactor,
                const unsigned char keyMap[EMU_KEY_COUNT],
                pCallbackFunc cbFunc,
                unsigned int callbackEvents);


/**
 * Cleans up ressources and closes the emulator
 */
void chip8_Close(void);


/**
 * Returns the current status of the Emulator.
 * @see enum EEMulatorState in chip8_public.h file.
 * @return Emulator's state
 */
EEmulatorState chip8_GetEmulatorState(void);

/**
 * Set the Window to a new Scale. 1 means original Resolution (64x32).
 * Default is x5.
 *
 * @param windowScale
 *               New Scale Factor. Must fit your monitor.
 */
void chip8_SetWindowScale(unsigned char windowScale);


/**
 * Sets the window Position to the specified coordinates, absolute on current screen.
 *
 * @param iPosX  Position X
 * @param iPosY  Position Y
 */
void chip8_SetWindowPosition(int iPosX,
                             int iPosY);

/**
 * Displays the the Window and brings it to front, or hides it.
 *
 * @param visible 1 == show window, 0 = hide
 */
void chip8_SetWindowVisible(int visible);
/**
 * Set the Emulation Speed to the desired value,
 * only use the predefined EEmulationSpeed values.
 *
 * @param eSpeed Speed to set (0.5-2X currently available)
 */
void chip8_SetEmulationSpeed(EEmulationSpeed eSpeed);

/**
 * Pause or resume the runthread.
 *
 * @param pause  0 = resume, nonzero = pause
 */
void chip8_SetPause(int pause);

/**
 * Set a new Keymap.
 *
 * @param newKeymap The new Keymap to be used, see description on chip8_Init() for details.
 */
void chip8_SetKeymap(unsigned char newKeymap[EMU_KEY_COUNT]);

/**
 * Loads a .ch8-File to the Emulator.
 *
 * @param pcFilePath Path to .ch8-File
 * @param tStartAddress
 *                   The startaddress of the first instruction to load the file to. Usually, this should be 0x200.
 *
 * @return o on success, nonzero on error.
 */
int  chip8_LoadFile(const char *pcFilePath,
                    word tStartAddress);

/**
 * Prints the current Content of the Screen to stdout.
 * 0= Background, 1= Foreground
 * Use with care when the Emulator thread is running.
 */
void chip8_DumpScreen(void);

#ifdef __cplusplus
}
#endif

#endif /* CHIP8_EMULATOR_H_INCLUDED */
