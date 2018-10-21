CC=gcc
LINK=gcc

FILES=Chip8_Emulator Chip8_freq_win32 Chip8_keys_sdl2 Chip8_screen_sdl2 Chip8_sound_sdl2 main
OBJ=$(addsuffix .o,$(FILES))

CFLAGS=-Wall -Wextra -Wall -m64 -Wno-missing-braces -Wno-missing-field-initializers -Wformat=2 -Wpointer-arith -Wstrict-overflow=5 -Wstrict-prototypes -Wnested-externs -Wcast-qual -Wlogical-op -Wstrict-aliasing=2 -Wold-style-definition -fno-omit-frame-pointer -ffloat-store
LFLAGS=-lSDL2main -lSDL2 -m64

%.o: %.c
		$(CC) -c -o $@ $< $(CFLAGS)

Chip_8_Emulator: $(OBJ)
		$(LINK) -o $@.exe $^ $(LFLAGS)
		
.PHONY: clean		
		
clean:
	rm *.o *.exe 2>nul