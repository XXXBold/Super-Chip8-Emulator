#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include "chip8_app_wxwidgets.h"
#include "chip8_uimain_wxwidgets.h"
#include "chip8_uikeymap_wxwidgets.h"
#include "chip8_Emulator.h"
#include "appconfig.h"

#ifdef _WIN32
  #define DEFAULT_LOADFILE_PATH "C:\\"
#else
  #define DEFAULT_LOADFILE_PATH "/home"
#endif

int iEmuCBFunc_m(unsigned int event,
                 word currOPCode);

IMPLEMENT_APP(Chip8_GUI)

bool Chip8_GUI::OnInit()
{
  wxWinMain *wMain;
  int iPosX;
  int iPosY;

  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(!wxApp::OnInit())
    return(false);

  if(!this->bConfigLoad())
  {
    return(false);
  }

  wMain = new wxWinMain(NULL,wxID_ANY,APP_DISPLAY_NAME);
//wMain->SetWindowScale(MY_WINDOW_SCALE);
//wMain->GetClientPosOnScreen(&iPosX,&iPosY);
  wMain->Show(true);
  iPosX=200;
  iPosY=200;
  if(chip8_Init(iPosX,
                iPosY,
                MY_WINDOW_SCALE,
                this->ucaKeymap,
                iEmuCBFunc_m,
                EMU_EVT_INSTRUCTION_ERROR | EMU_EVT_INSTRUCTION_UNKNOWN | EMU_EVT_KEYPRESS_ESCAPE | EMU_EVT_SUCHIP_INSTRUCTION_EXECUTED))
  {
    DEBUG_WXPUTS("chip8_Init() failed, quit...");
    return(false);
  }
  wMain->OptionSetSpeed(EMU_SPEED_1_0X);
  return(true);
}

int iEmuCBFunc_m(unsigned int event,
                 word currOPCode)
{
  wxThreadEvent tagEv(wxEVT_THREAD,wxEVT_COMMAND_TEXT_UPDATED);
  TagThreadEventParam tagParam;

  tagParam.tOpCode=currOPCode;
  tagParam.uiEvent=event;
//DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  tagEv.SetPayload(tagParam);
  wxQueueEvent(wxGetApp().GetTopWindow()->GetEventHandler(),tagEv.Clone());
  return(0);
}

void Chip8_GUI::vApp_GetKeymap(unsigned char ucaKeymap[EMU_KEY_COUNT])
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  memcpy(ucaKeymap,this->ucaKeymap,EMU_KEY_COUNT);
}

void Chip8_GUI::vApp_SetKeymap(unsigned char ucaNewKeymap[EMU_KEY_COUNT])
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  memcpy(this->ucaKeymap,ucaNewKeymap,EMU_KEY_COUNT);
  chip8_SetKeymap(this->ucaKeymap);
  this->vSetKeymapToConfig();
}

void Chip8_GUI::vApp_SetLastLoadPath(wxString& strPath)
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(strPath.Length()>=sizeof(this->caLastFileLoadPath))
  {
    wxPuts("Can'r set path, too long");
    return;
  }
  strcpy(this->caLastFileLoadPath,strPath.ToAscii());
}

wxString Chip8_GUI::strApp_GetLastLoadPath()
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  return(wxString::FromAscii(this->caLastFileLoadPath));
}

void Chip8_GUI::vGetKeymapFromConfig()
{
  unsigned int uiIndex;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  for(uiIndex=0;uiIndex<sizeof(this->ucaKeymap);++uiIndex)
  {
    this->ucaKeymap[uiIndex]=this->taCfgEntries[CFG_INDEX_KEYMAP_START+uiIndex].data.iVal;
  }
}

void Chip8_GUI::vSetKeymapToConfig()
{
  unsigned int uiIndex;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  for(uiIndex=0;uiIndex<sizeof(this->ucaKeymap);++uiIndex)
  {
    this->taCfgEntries[CFG_INDEX_KEYMAP_START+uiIndex].data.iVal=this->ucaKeymap[uiIndex];
  }
}

bool Chip8_GUI::bConfigLoad()
{
  unsigned int uiIndex;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  strcpy(this->caLastFileLoadPath,DEFAULT_LOADFILE_PATH);
  this->taCfgEntries[CFG_INDEX_LAST_SEL_PATH].dataType  =DATA_TYPE_STRING;
  this->taCfgEntries[CFG_INDEX_LAST_SEL_PATH].groupName =CFG_TXT_GROUP_PATHS;
  this->taCfgEntries[CFG_INDEX_LAST_SEL_PATH].keyName   =CFG_TXT_KEY_LAST_SEL_PATH;
  this->taCfgEntries[CFG_INDEX_LAST_SEL_PATH].data.pcVal=this->caLastFileLoadPath;
  this->taCfgEntries[CFG_INDEX_LAST_SEL_PATH].dataSize  =sizeof(this->caLastFileLoadPath);

  for(uiIndex=0;uiIndex<sizeof(this->ucaKeymap);++uiIndex)
  {
    this->taCfgEntries[CFG_INDEX_KEYMAP_START+uiIndex].groupName =CFG_TXT_GROUP_KEYMAP;
    this->taCfgEntries[CFG_INDEX_KEYMAP_START+uiIndex].dataType  =DATA_TYPE_INT;
    this->taCfgEntries[CFG_INDEX_KEYMAP_START+uiIndex].dataSize  =sizeof(int);
  }
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x0].keyName   =CFG_TXT_KEY_KEYMAP_0;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x0].data.iVal =EMU_KEY_0;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x1].keyName   =CFG_TXT_KEY_KEYMAP_1;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x1].data.iVal =EMU_KEY_1;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x2].keyName   =CFG_TXT_KEY_KEYMAP_2;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x2].data.iVal =EMU_KEY_2;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x3].keyName   =CFG_TXT_KEY_KEYMAP_3;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x3].data.iVal =EMU_KEY_3;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x4].keyName   =CFG_TXT_KEY_KEYMAP_4;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x4].data.iVal =EMU_KEY_4;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x5].keyName   =CFG_TXT_KEY_KEYMAP_5;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x5].data.iVal =EMU_KEY_5;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x6].keyName   =CFG_TXT_KEY_KEYMAP_6;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x6].data.iVal =EMU_KEY_6;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x7].keyName   =CFG_TXT_KEY_KEYMAP_7;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x7].data.iVal =EMU_KEY_7;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x8].keyName   =CFG_TXT_KEY_KEYMAP_8;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x8].data.iVal =EMU_KEY_8;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x9].keyName   =CFG_TXT_KEY_KEYMAP_9;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0x9].data.iVal =EMU_KEY_9;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0xA].keyName   =CFG_TXT_KEY_KEYMAP_A;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0xA].data.iVal =EMU_KEY_A;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0xB].keyName   =CFG_TXT_KEY_KEYMAP_B;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0xB].data.iVal =EMU_KEY_B;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0xC].keyName   =CFG_TXT_KEY_KEYMAP_C;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0xC].data.iVal =EMU_KEY_C;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0xD].keyName   =CFG_TXT_KEY_KEYMAP_D;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0xD].data.iVal =EMU_KEY_D;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0xE].keyName   =CFG_TXT_KEY_KEYMAP_E;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0xE].data.iVal =EMU_KEY_E;

  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0xF].keyName   =CFG_TXT_KEY_KEYMAP_F;
  this->taCfgEntries[CFG_INDEX_KEYMAP_START+0xF].data.iVal =EMU_KEY_F;

  this->entriesCount=sizeof(this->taCfgEntries)/sizeof(AppConfigEntry);

  if(!(this->appCfg=appConfig_Load(APP_DISPLAY_NAME,this->taCfgEntries,this->entriesCount,NULL,NULL)))
  {
    return(false);
  }
  this->vGetKeymapFromConfig();
  return(true);
}
