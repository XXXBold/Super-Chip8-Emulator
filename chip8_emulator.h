#ifndef CHIP8_EMULATOR_H_INCLUDED
  #define CHIP8_EMULATOR_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "chip8_public.h"

/**
 * Initialises the emulator, pauses execution.
 *
 * @param winPosX _IN_ Position X of the Emulator Screen, in Monitor coordinates.
 * @param winPosY _IN_ Position Y of the Emulator Screen, in Monitor coordinates.
 * @param winScaleFactor
 *                _IN_OPT_ Window Scale Factor, 1= Original 64x32. <br>
 *                When Superchip support is enabled (ENABLE_SUPERCHIP_INSTRUCTIONS defined), should be at least 2. <br>
 *                Pass 0 for Default Scale (=5, means 320x160px).
 * @param keyMap  _IN_OPT_ Keymap for the Keys of the Emulator (0-F). <br>
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
 * @param cbFunc  _IN_OPT_ Callbackfunction to call when executing an Emulator Instruction. <br>
 *                Do not call any chip8_ functions from this header directly within the callback, <br>
 *                this can cause deadlocking.
 *                Pass NULL if not needed.
 * @param callbackEvents
 *                _IN_OPT_ Define on which Events procFunc should be called, 0 if Callbacks aren't needed.
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
 * Resets the emulator memory (and all registers) to the last file loaded.
 */
void chip8_Reset(void);
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
 *               _IN_ New Scale Factor. Must fit your monitor.
 */
void chip8_SetWindowScale(unsigned char windowScale);


/**
 * Sets the window Position to the specified coordinates, absolute on current screen.
 *
 * @param iPosX  _IN_ Position X
 * @param iPosY  _IN_ Position Y
 */
void chip8_SetWindowPosition(int iPosX,
                             int iPosY);

/**
 * Sets the Title for the Rendering Window
 *
 * @param windowTitle
 *               _IN_ New window title to be set. For maximum length, @see chip8_global.h CHIP8_SCREEN_TITLE_MAXLEN
 */
void chip8_SetWindowTitle(const char *windowTitle);

/**
 * Displays the the Window and brings it to front, or hides it.
 *
 * @param visible _IN_ 1 == show window, 0 = hide
 */
void chip8_SetWindowVisible(int visible);
/**
 * Set the Emulation Speed to the desired value,
 * only use the predefined EEmulationSpeed values.
 *
 * @param eSpeed _IN_ Speed to set (0.5-2X currently available)
 */
void chip8_SetEmulationSpeed(EEmulationSpeed eSpeed);

/**
 * Pause or resume the runthread.
 *
 * @param pause  _IN_ 0 = resume, nonzero = pause
 */
void chip8_SetPause(int pause);

/**
 * Set a new Keymap.
 *
 * @param newKeymap _IN_ The new Keymap to be used, see description on chip8_Init() for details.
 */
void chip8_SetKeymap(unsigned char newKeymap[EMU_KEY_COUNT]);

/**
 * Loads a .ch8-File to the Emulator.
 *
 * @param pcFilePath _IN_ Path to .ch8-File
 * @param tStartAddress
 *                   _IN_ The startaddress of the first instruction to load the file to. Usually, this should be 0x200.
 *
 * @return o on success, nonzero on error.
 */
int chip8_LoadFile(const char *pcFilePath,
                   word tStartAddress);


/**
 * Enables some quirks (a.k.a Workarounds) for the Emulator. <br>
 * This might be used to emulate some ROMs properly.
 *
 * @param quirks _IN_ All Quriks ORed together which should be enabled (others will be disabled)
 *
 * @see chip8_public.h the EMU_QUIRK_ enums for possible quirks. Invalid values are silently ignored.
 */
void chip8_EnableQuirks(unsigned int quirks);

/**
 * Prints the current Content of the Screen to stdout.
 * 0= Background, 1= Foreground
 * Use with care when the Emulator thread is running.
 *
 * @param fp     _IN_ Stream where to dump the contents (e.g. stdout, stderr)
 */
void chip8_DumpScreen(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif /* CHIP8_EMULATOR_H_INCLUDED */
