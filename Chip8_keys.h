#ifndef CHIP8_KEYS_H_INCLUDED
  #define CHIP8_KEYS_H_INCLUDED

void vChip8_Keys_GetState_g(unsigned short *pusKeyMap);
unsigned char ucChip8_Keys_WaitForNext_g(void);

#endif /* CHIP8_KEYS_H_INCLUDED */
