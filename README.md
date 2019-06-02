# Super-Chip8-Emulator
Emulator for Chip-8 Applications using SDL2 and wxWidgets.

Current status: WIP, tested on Windows, Beta release binary available for Windows (64-bit).

For getting some roms, refer to e.g. https://github.com/dmatlack/chip8/tree/master/roms or turn on your prefered search engine to tell you where's more ;)

For using the Linux build, make sure you've installed wxwidgets and SDL2 libraries:

For Ubuntu/debian based distros, install dependencies as follows (maybe some others aswell):

- apt install libsdl2-dev libwxgtk3.0-dev

## Releases
- 0.1: First Release
- 0.2-beta: Added Superchip instructions
- 0.3-beta: Added support for linux, changed renderer
- 0.3.1-beta: Bugfix, Register VF is now set correctly
- 0.4-beta: Improved Superchip support (Should now be fully functional), added more Quirks, added Icon & show it properly, added license. For Linux: Updated to wxWidgets based on GTK3. If upgrading from a previous version, check keymapping, it will change at first launch of this version.

## TODOs
- Add build instructions
- More code cleanup

## Credits & References
- SDL2 Library: https://www.libsdl.org/
- wxWidgets Library: http://www.wxwidgets.org/
- Cowgods' Chip-8 Reference: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
- BestCoder for his Test ROM: https://slack-files.com/T3CH37TNX-F3RKEUKL4-b05ab4930d / https://slack-files.com/T3CH37TNX-F3RF5KT43-0fb93dbd1f
