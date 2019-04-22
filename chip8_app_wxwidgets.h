#ifndef CHIP8_APP_WXWIDGETS_H_INCLUDED
  #define CHIP8_APP_WXWIDGETS_H_INCLUDED

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include "chip8_emulator.h"
#include "appconfig.h"

//Define this to enable debug output for gui
//#define GUI_DEBUG_TRACE

#ifdef GUI_DEBUG_TRACE
  #define DEBUG_WXPUTS(str) wxPuts(str)
  //Fallback if macro not defined
  #ifndef __GNUG__
    #define __PRETTY_FUNCTION__ "__PRETTY_FUNCTION__ not defined"
  #endif
#else
  #define DEBUG_WXPUTS(str)
#endif /* GUI_DEBUG_TRACE */

#define APP_VERSION_MAJOR     0
#define APP_VERSION_MINOR     3
#define APP_VERSION_PATCH     0
#define APP_VERSION_BUILDTIME "Build: " __DATE__ ", " __TIME__
#define APP_DISPLAY_NAME      "Chip-8_Emulator"
#define APP_VERSION_DEVSTATE  "Beta"
#define APP_PROJECT_HOMEPAGE  "https://github.com/XXXBold/Chip8Emulator"
#define APP_PROJECT_DEVELOPER "XXXBold"

#define MY_WINDOW_SCALE 10u

#define CFG_TXT_GROUP_KEYMAP      "Keymap"
#define CFG_TXT_GROUP_PATHS       "Paths"

#define CFG_TXT_KEY_KEYMAP_0      "Key0"
#define CFG_TXT_KEY_KEYMAP_1      "Key1"
#define CFG_TXT_KEY_KEYMAP_2      "Key2"
#define CFG_TXT_KEY_KEYMAP_3      "Key3"
#define CFG_TXT_KEY_KEYMAP_4      "Key4"
#define CFG_TXT_KEY_KEYMAP_5      "Key5"
#define CFG_TXT_KEY_KEYMAP_6      "Key6"
#define CFG_TXT_KEY_KEYMAP_7      "Key7"
#define CFG_TXT_KEY_KEYMAP_8      "Key8"
#define CFG_TXT_KEY_KEYMAP_9      "Key9"
#define CFG_TXT_KEY_KEYMAP_A      "KeyA"
#define CFG_TXT_KEY_KEYMAP_B      "KeyB"
#define CFG_TXT_KEY_KEYMAP_C      "KeyC"
#define CFG_TXT_KEY_KEYMAP_D      "KeyD"
#define CFG_TXT_KEY_KEYMAP_E      "KeyE"
#define CFG_TXT_KEY_KEYMAP_F      "KeyF"

#define CFG_TXT_KEY_LAST_SEL_PATH "LastLoadPath"

enum appConfigIndex
{
  CFG_INDEX_LAST_SEL_PATH = 0,
  CFG_INDEX_KEYMAP_START  = 1
};

typedef struct
{
  unsigned int uiEvent;
  word tOpCode;
}TagThreadEventParam;

class Chip8_GUI : public wxApp
{
public:
  virtual bool OnInit(void);
  void vApp_GetKeymap(unsigned char ucaKeymap[EMU_KEY_COUNT]);
  void vApp_SetKeymap(unsigned char ucaNewKeymap[EMU_KEY_COUNT]);

  void vApp_SetLastLoadPath(wxString& strPath);
  wxString strApp_GetLastLoadPath();

  AppConfig appCfg;
  size_t entriesCount;
  AppConfigEntry taCfgEntries[17];
private:
  bool bConfigLoad();
  void vGetKeymapFromConfig();
  void vSetKeymapToConfig();

  unsigned char ucaKeymap[EMU_KEY_COUNT];
  char caLastFileLoadPath[260];
};

DECLARE_APP(Chip8_GUI)

#endif //CHIP8_APP_WXWIDGETS_H_INCLUDED
