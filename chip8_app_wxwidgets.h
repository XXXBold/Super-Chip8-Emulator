#ifndef CHIP8_APP_WXWIDGETS_H_INCLUDED
  #define CHIP8_APP_WXWIDGETS_H_INCLUDED

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include "chip8_emulator.h"
#include "appconfig.h"

//Define this to enable debug output for gui
  #define GUI_DEBUG_TRACE

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
#define APP_VERSION_MINOR     4
#define APP_VERSION_PATCH     0
#define APP_VERSION_BUILDTIME "Build: " __DATE__ ", " __TIME__
#define APP_DISPLAY_NAME      "Chip-8_Emulator"
#define APP_VERSION_DEVSTATE  "Beta"
#define APP_PROJECT_HOMEPAGE  "https://github.com/XXXBold/Super-Chip8-Emulator"
#define APP_PROJECT_DEVELOPER "XXXBold"
#ifdef __i386__
  #define APP_VERSION_ARCH "32-bit"
#elif __x86_64__
  #define APP_VERSION_ARCH "64-bit"
#else
  #define APP_VERSION_ARCH "\?\?-bit"
#endif

#define MY_WINDOW_SCALE 10u

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
  virtual bool OnInit();
  virtual int OnExit();
  void vApp_GetKeymap(unsigned char ucaKeymap[EMU_KEY_COUNT]);
  void vApp_SetKeymap(unsigned char ucaNewKeymap[EMU_KEY_COUNT]);

  void vApp_SetLastLoadPath(wxString& strPath);
  wxString strApp_GetLastLoadPath();

  AppConfig appCfg;
  AppConfigEntry taCfgEntries[17];
private:
  bool bConfigLoad();
  void vConfigSave();
  void vGetKeymapFromConfig();
  void vSetKeymapToConfig();

  unsigned char ucaKeymap[EMU_KEY_COUNT];
  char caLastFileLoadPath[260];
};

DECLARE_APP(Chip8_GUI)

#endif //CHIP8_APP_WXWIDGETS_H_INCLUDED
