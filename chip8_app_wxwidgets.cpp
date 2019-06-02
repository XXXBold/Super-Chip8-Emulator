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
                EMU_EVT_ERR_INVALID_INSTRUCTION |
                EMU_EVT_ERR_INSTRUCTION_UNKNOWN |
                EMU_EVT_ERR_INTERNAL            |
                EMU_EVT_KEYPRESS_ESCAPE         |
                EMU_EVT_PROGRAM_EXIT            |
                EMU_EVT_SUCHIP_INSTRUCTION_EXECUTED))
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
  wMain->SetWindowScale(MY_WINDOW_SCALE);
//wMain->GetClientPosOnScreen(&iPosX,&iPosY);
  wMain->Show(true);

  return(true);
}

int Chip8_GUI::OnExit()
{
  this->vConfigSave();
  return(0);
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
  DEBUG_WXPUTS("Set Last Load Path to " + strPath);
  if(dataType_Update_String(&this->taCfgEntries[CFG_INDEX_LAST_SEL_PATH].tagData,
                            strPath.ToAscii(),
                            strPath.Len()+1))
  {
    DEBUG_WXPUTS("Failed to set the last load path");
  }
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
    dataType_Update_Int(&this->taCfgEntries[CFG_INDEX_KEYMAP_START+uiIndex].tagData,
                        this->ucaKeymap[uiIndex]);
  }
}

bool Chip8_GUI::bConfigLoad()
{
  unsigned int uiIndex;
  int iRc;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  this->taCfgEntries[CFG_INDEX_LAST_SEL_PATH].groupName =CFG_TXT_GROUP_PATHS;
  this->taCfgEntries[CFG_INDEX_LAST_SEL_PATH].keyName   =CFG_TXT_KEY_LAST_SEL_PATH;
  dataType_Set_String(&this->taCfgEntries[CFG_INDEX_LAST_SEL_PATH].tagData,
                      this->caLastFileLoadPath,
                      sizeof(this->caLastFileLoadPath),
                      DEFAULT_LOADFILE_PATH,
                      sizeof(DEFAULT_LOADFILE_PATH));

  for(uiIndex=0;uiIndex<sizeof(this->ucaKeymap);++uiIndex)
  {
    this->taCfgEntries[CFG_INDEX_KEYMAP_START+uiIndex].groupName=CFG_TXT_GROUP_KEYMAP;
  }

  uiIndex=CFG_INDEX_KEYMAP_START;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_0;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_0,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_1;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_1,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_2;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_2,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_3;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_3,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_4;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_4,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_5;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_5,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_6;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_6,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_7;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_7,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_8;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_8,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_9;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_9,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_A;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_A,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_B;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_B,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_C;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_C,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_D;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_D,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_E;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_E,eDataRepresentation_Int_Hexadecimal);
  ++uiIndex;
  this->taCfgEntries[uiIndex].keyName   =CFG_TXT_KEY_KEYMAP_F;
  dataType_Set_Int(&this->taCfgEntries[uiIndex].tagData,EMU_KEY_F,eDataRepresentation_Int_Hexadecimal);

  switch((iRc=appConfig_New(&this->appCfg,
                            this->taCfgEntries,
                            sizeof(this->taCfgEntries)/sizeof(AppConfigEntry),
                            APP_DISPLAY_NAME,
                            NULL,
                            NULL)))
  {
    case APPCFG_ERR_NONE:
      break;
    default:
      DEBUG_WXPUTS(wxString::Format("appConfig_New() failed (%d): %s",iRc,appConfig_GetErrorString(iRc)));
      return(false);
  }
  switch((iRc=appConfig_DataLoad(this->appCfg)))
  {
    case APPCFG_LOAD_NEW:
    case APPCFG_LOAD_EXISTING:
      iRc=0;
      break; /* Okay */
    case APPCFG_ERR_DATA_MALFORMED:
      if(wxMessageBox(wxString::Format("appConfig_DataLoad() failed (%d): %s\n"
                                       "Config file is at \"%s\"\n"
                                       "Do you want to remove the configuration File?\n"
                                       "If Yes, a restart of the Application creates a new config",
                                       iRc,
                                       appConfig_GetErrorString(iRc),
                                       appConfig_GetPath(this->appCfg)),
                      "Error loading Config",
                      wxYES_NO | wxCENTER | wxICON_EXCLAMATION) == wxYES)
      {
        if((iRc=appConfig_DataDelete(this->appCfg)) != APPCFG_ERR_NONE)
          DEBUG_WXPUTS(wxString::Format("appConfig_DataDelete() failed for config file \"%s\" (%d): %s",
                                        iRc,
                                        appConfig_GetErrorString(iRc)));
      }
      iRc=1;
      break;
    default:
      wxMessageBox(wxString::Format("appConfig_DataLoad() failed (%d): %s",
                                    iRc,
                                    appConfig_GetErrorString(iRc)),
                   "Error loading Config",
                   wxOK | wxCENTER | wxICON_ERROR);
      iRc=1;
      break;
  }
  if(iRc)
  {
    appConfig_Close(this->appCfg);
    return(false);
  }
#ifdef GUI_DEBUG_TRACE
  appConfig_DumpContents(this->appCfg,stderr);
#endif
  this->vGetKeymapFromConfig();
  return(true);
}

void Chip8_GUI::vConfigSave()
{
  int iRc;

  switch((iRc=appConfig_DataSave(this->appCfg)))
  {
    case APPCFG_ERR_NONE:
      break;
    default:
      wxMessageBox(wxString::Format("appConfig_Save() failed (%d) %s\n"
                                    "Unable to store your configuration!\n"
                                    "Config File Path is: \"%s\"\n"
                                    "On I/O Error: Maybe Check permissions for file write access?",
                                    iRc,
                                    appConfig_GetErrorString(iRc),
                                    appConfig_GetPath(this->appCfg)),
                   "Error saving configuration",
                   wxOK | wxCENTER | wxICON_EXCLAMATION,
                   NULL);
  }
  appConfig_Close(this->appCfg);
}
