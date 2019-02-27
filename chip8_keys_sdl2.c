#include <stdio.h>
#include <stdlib.h>

#include "chip8_keys.h"
#include "chip8_global.h"

int iChip8_Keys_Init_g(TagKeyboard *ptagKeyboard)
{
  TRACE_DBG_INFO("Initialise Keys...");
  /**
   *  Disable unneeded events
   */
  /* Disable Mouse events */
  SDL_EventState(SDL_MOUSEMOTION,      SDL_IGNORE);
  SDL_EventState(SDL_MOUSEBUTTONDOWN,  SDL_IGNORE);
  SDL_EventState(SDL_MOUSEBUTTONUP,    SDL_IGNORE);
  SDL_EventState(SDL_MOUSEWHEEL,       SDL_IGNORE);

  /* Drag & drop */
  SDL_EventState(SDL_DROPFILE,         SDL_IGNORE);
  SDL_EventState(SDL_DROPTEXT,         SDL_IGNORE);
  SDL_EventState(SDL_DROPBEGIN,        SDL_IGNORE);
  SDL_EventState(SDL_DROPCOMPLETE,     SDL_IGNORE);
  ptagKeyboard->ulKeyStates=0;
  ptagKeyboard->ulKeyStatesLast=0;
  return(0); /* further init needed for sdl2 */
}

void vChip8_Keys_Close_g(TagKeyboard *ptagKeyboard)
{
  (void)ptagKeyboard;
  TRACE_DBG_INFO("Close Keys..."); /* No close needed for sdl2 */
}

int iChip8_Keys_SetKeymap_g(TagKeyboard *ptagKeyBoard)
{
  unsigned int iRc=0;
  unsigned char ucIndex;
  for(ucIndex=0;ucIndex<sizeof(ptagKeyBoard->ausKeyMap)/sizeof(unsigned short);++ucIndex)
  {
    switch(ptagKeyBoard->ucaUsrKeyMap[ucIndex])
    {
      case EMU_KEY_0:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_0;     break;
      case EMU_KEY_1:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_1;     break;
      case EMU_KEY_2:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_2;     break;
      case EMU_KEY_3:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_3;     break;
      case EMU_KEY_4:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_4;     break;
      case EMU_KEY_5:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_5;     break;
      case EMU_KEY_6:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_6;     break;
      case EMU_KEY_7:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_7;     break;
      case EMU_KEY_8:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_8;     break;
      case EMU_KEY_9:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_9;     break;
      case EMU_KEY_A:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_A;     break;
      case EMU_KEY_B:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_B;     break;
      case EMU_KEY_C:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_C;     break;
      case EMU_KEY_D:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_D;     break;
      case EMU_KEY_E:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_E;     break;
      case EMU_KEY_F:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_F;     break;
      case EMU_KEY_G:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_G;     break;
      case EMU_KEY_H:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_H;     break;
      case EMU_KEY_I:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_I;     break;
      case EMU_KEY_J:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_J;     break;
      case EMU_KEY_K:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_K;     break;
      case EMU_KEY_L:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_L;     break;
      case EMU_KEY_M:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_M;     break;
      case EMU_KEY_N:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_N;     break;
      case EMU_KEY_O:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_O;     break;
      case EMU_KEY_P:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_P;     break;
      case EMU_KEY_Q:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_Q;     break;
      case EMU_KEY_R:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_R;     break;
      case EMU_KEY_S:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_S;     break;
      case EMU_KEY_T:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_T;     break;
      case EMU_KEY_U:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_U;     break;
      case EMU_KEY_V:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_V;     break;
      case EMU_KEY_W:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_W;     break;
      case EMU_KEY_X:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_X;     break;
      case EMU_KEY_Y:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_Y;     break;
      case EMU_KEY_Z:     ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_Z;     break;
      case EMU_KEY_UP:    ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_UP;    break;
      case EMU_KEY_DOWN:  ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_DOWN;  break;
      case EMU_KEY_LEFT:  ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_LEFT;  break;
      case EMU_KEY_RIGHT: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_RIGHT; break;
      default:
        TRACE_DBG_ERROR_VARG("chip8_Keys_SetKeymap_m(): Key [%X] unknwon keycode (0x%X), using default...",
                             ucIndex,
                             ptagKeyBoard->ucaUsrKeyMap[ucIndex]);
        iRc=1;
      /* Fall through */
      case EMU_KEY_DEFAULT:
        switch(ucIndex)
        {
          case 0x0: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_0; break;
          case 0x1: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_1; break;
          case 0x2: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_2; break;
          case 0x3: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_3; break;
          case 0x4: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_4; break;
          case 0x5: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_5; break;
          case 0x6: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_6; break;
          case 0x7: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_7; break;
          case 0x8: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_8; break;
          case 0x9: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_9; break;
          case 0xA: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_A; break;
          case 0xB: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_B; break;
          case 0xC: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_C; break;
          case 0xD: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_D; break;
          case 0xE: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_E; break;
          case 0xF: ptagKeyBoard->ausKeyMap[ucIndex]=SDL_SCANCODE_F; break;
        }
        break;
    }
  }
  return(iRc);
}

void vChip8_Keys_UpdateState_g(TagKeyboard *ptagKeyboard)
{
  const Uint8 *puiTmp;

  SDL_PumpEvents();
  puiTmp=SDL_GetKeyboardState(NULL);

  /* Save last keyboard state */
  ptagKeyboard->ulKeyStatesLast=ptagKeyboard->ulKeyStates;

  ptagKeyboard->ulKeyStates=((puiTmp[ptagKeyboard->ausKeyMap[0x0]])?EMU_KEY_MASK_0:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x1]])?EMU_KEY_MASK_1:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x2]])?EMU_KEY_MASK_2:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x3]])?EMU_KEY_MASK_3:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x4]])?EMU_KEY_MASK_4:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x5]])?EMU_KEY_MASK_5:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x6]])?EMU_KEY_MASK_6:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x7]])?EMU_KEY_MASK_7:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x8]])?EMU_KEY_MASK_8:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0x9]])?EMU_KEY_MASK_9:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0xA]])?EMU_KEY_MASK_A:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0xB]])?EMU_KEY_MASK_B:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0xC]])?EMU_KEY_MASK_C:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0xD]])?EMU_KEY_MASK_D:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0xE]])?EMU_KEY_MASK_E:0|
                             (puiTmp[ptagKeyboard->ausKeyMap[0xF]])?EMU_KEY_MASK_F:0|
                             (puiTmp[SDL_SCANCODE_ESCAPE])         ?EMU_KEY_MASK_ESC:0);
//printf("SDL Updated keys: 0x%X, last: 0x%X\n",ptagKeyboard->ulKeyStates,ptagKeyboard->ulKeyStatesLast);
}
