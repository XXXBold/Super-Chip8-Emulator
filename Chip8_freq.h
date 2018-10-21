#ifndef CHIP8_FREQ_H_INCLUDED
  #define CHIP8_FREQ_H_INCLUDED

typedef void(FreqFunc)(void);

int iChip8_Frequency_Start_g(FreqFunc func);
void vChip8_Frequency_Stop_g(void);


#endif /* CHIP8_WIN32_H_INCLUDED */
