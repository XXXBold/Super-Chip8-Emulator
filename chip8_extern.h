#ifndef CHIP8_EXTERN_H_INCLUDED
  #define CHIP8_EXTERN_H_INCLUDED

#include "chip8_global.h"

/**
 * Initialises global stuff for extern needed libraries
 * Inside this function, all Modules (Sound, Screen, Keys, ...)
 * should be initialized
 * Call before Starting Emulator
 *
 * @param pEmulator  Settings to run the emulator, must be a global variable
 *
 * @return int 0 on success, -1 on error
 */
int iChip8_External_Init_g(TagEmulator *pEmulator);

/**
 * cleans up global stuff for extern needed libraries
 * Call when closing Emulator
 */
void vChip8_External_Close_g(TagEmulator *pEmulator);

#endif /* CHIP8_EXTERN_H_INCLUDED */
