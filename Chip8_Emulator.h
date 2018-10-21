#ifndef CHIP8_EMULATOR_H_INCLUDED
  #define CHIP8_EMULATOR_H_INCLUDED

typedef unsigned char byte;
typedef unsigned short word;

int iChip8_Init_g(void);
void vChip8_Close_g(void);
void vChip8_Process_g(void);
void vChip8_DumpScreen_g(void);
int iChip8_LoadFile_g(const char *pcFilePath,
                      word tStartAddress);

#endif /* CHIP8_EMULATOR_H_INCLUDED */
