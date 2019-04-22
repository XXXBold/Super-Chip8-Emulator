#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include "chip8_app_wxwidgets.h"
#include "chip8_uimain_wxwidgets.h"
#include "chip8_uikeymap_wxwidgets.h"
#include "chip8_emulator.h"
#include "appconfig.h"

#ifdef _WIN32
  #define DEFAULT_LOADFILE_PATH "C:\\"
#else
  #define DEFAULT_LOADFILE_PATH "/home"
#endif

int iEmuCBFunc_m(unsigned int event,
                 word currOPCode);

IMPLEMENT_APP_NO_MAIN(Chip8_GUI)

int main(int argc, char *argv[])
{
  if(chip8_Init(200,
                200,
                MY_WINDOW_SCALE,
                NULL,
                iEmuCBFunc_m,
                EMU_EVT_INSTRUCTION_ERROR | EMU_EVT_INSTRUCTION_UNKNOWN | EMU_EVT_KEYPRESS_ESCAPE | EMU_EVT_SUCHIP_INSTRUCTION_EXECUTED))
  {
    DEBUG_WXPUTS("chip8_Init() failed, quit...");
    return(false);
  }
  return(wxEntry(argc,argv));
}

bool Chip8_GUI::OnInit()
{
  wxWinMain *wMain;

  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(!wxApp::OnInit())
    return(false);

  if(!this->bConfigLoad())
  {
    DEBUG_WXPUTS("bConfigLoad() failed, quit...");
    return(false);
  }
  chip8_SetKeymap(this->ucaKeymap);
  wMain = new wxWinMain(NULL,wxID_ANY,APP_DISPLAY_NAME);
//wMain->SetWindowScale(MY_WINDOW_SCALE);
//wMain->GetClientPosOnScreen(&iPosX,&iPosY);
  wMain->Show(true);

  wMain->OptionSetSpeed(EMU_SPEED_1_0X);
  return(true);
}

int iEmuCBFunc_m(unsigned int event,
                 word currOPCode)
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  wxThreadEvent tagEv(wxEVT_THREAD,wxEVT_COMMAND_TEXT_UPDATED);
  TagThreadEventParam tagParam;

  tagParam.tOpCode=currOPCode;
  tagParam.uiEvent=event;

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
  wxString strTmp;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if((strTmp=strPath.BeforeLast('/')) != wxEmptyString)
    strPath=strTmp;
  else if((strTmp=strPath.BeforeLast('\\')) != wxEmptyString)
    strPath=strTmp;
  if(strPath.Length()>=sizeof(this->caLastFileLoadPath))
  {
    DEBUG_WXPUTS("Can't set path, too long");
    return;
  }
  strcpy(this->caLastFileLoadPath,strPath.ToAscii());
  DEBUG_WXPUTS("Set Last Load Path to " + strPath);
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
    this->ucaKeymap[uiIndex]=DATA_GET_INT(this->taCfgEntries[CFG_INDEX_KEYMAP_START+uiIndex].tagData);
  }
}

void Chip8_GUI::vSetKeymapToConfig()
{
  unsigned int uiIndex;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  for(uiIndex=0;uiIndex<sizeof(this->ucaKeymap);++uiIndex)
  {
    DATA_SET_INT(this->taCfgEntries[CFG_INDEX_KEYMAP_START+uiIndex].tagData,this->ucaKeymap[uiIndex]);
  }
}

bool Chip8_GUI::bConfigLoad()
{
  unsigned int uiIndex;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  strcpy(this->caLastFileLoadPath,DEFAULT_LOADFILE_PATH);
  this->taCfgEntries[CFG_INDEX_LAST_SEL_PATH].groupName =CFG_TXT_GROUP_PATHS;
  this->taCfgEntries[CFG_INDEX_LAST_SEL_PATH].keyName   =CFG_TXT_KEY_LAST_SEL_PATH;
  DATA_SET_STRING(this->taCfgEntries[CFG_INDEX_LAST_SEL_PATH].tagData,
                  this->caLastFileLoadPath,
                  sizeof(this->caLastFileLoadPath));

  for(uiIndex=0;uiIndex<sizeof(this->ucaKeymap);++uiIndex)
  {
    this->taCfgEntries[CFG_INDEX_KEYMAP_START+uiIndex].groupName=CFG_TXT_GROUP_KEYMAP;
  }
  uiIndex=CFG_INDEX_KEYMAP_START;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_0;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_0);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_1;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_1);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_2;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_2);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_3;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_3);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_4;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_4);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_5;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_5);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_6;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_6);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_7;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_7);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_8;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_8);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_9;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_9);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_A;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_A);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_B;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_B);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_C;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_C);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_D;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_D);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_E;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_E);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_F;
  DATA_SET_INT(this->taCfgEntries[uiIndex].tagData,EMU_KEY_F);
  this->entriesCount=sizeof(this->taCfgEntries)/sizeof(AppConfigEntry);

  if(!(this->appCfg=appConfig_Load(APP_DISPLAY_NAME,this->taCfgEntries,this->entriesCount,NULL,NULL)))
  {
    return(false);
  }
  this->vGetKeymapFromConfig();
  return(true);
}
