#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
  #include <wx/aboutdlg.h>
#endif

#include "chip8_uimain_wxwidgets.h"
#include "chip8_app_wxwidgets.h"
#include "chip8_Emulator.h"
#include "appconfig.h"

enum
{
  // menu items
  MainWindow_FileLoad                     = 1,
  MainWindow_FileEmuRun,
  MainWindow_FileEmuPause,
  MainWindow_StatusBar,
  MainWindow_About                        = wxID_ABOUT,

  MainWindow_EmuSpeed_0_5                 = 10,
  MainWindow_EmuSpeed_1_0                 = 11,
  MainWindow_EmuSpeed_1_5                 = 12,
  MainWindow_EmuSpeed_2_0                 = 13,
  MainWindow_Keymap                       = 20,
  MainWindow_DumpScreen
};

enum /* Other IDs */
{
  MainWindow_TimerID=1000
};

bool browseForFile(bool openFile,
                   const wxString &description,
                   wxString &outPath,
                   const wxString &fileSelectionMask,
                   wxWindow *parent=NULL,
                   const wxString &defaultPath=wxEmptyString);

wxBEGIN_EVENT_TABLE(wxWinMain,wxFrame)
EVT_MENU(MainWindow_About,                        wxWinMain::OnAbout)
EVT_MENU(MainWindow_FileLoad,                     wxWinMain::OnFileLoad)
EVT_MENU(MainWindow_FileEmuRun,                   wxWinMain::OnEmuStateSetRun)
EVT_MENU(MainWindow_FileEmuPause,                 wxWinMain::OnEmuStateSetPause)
EVT_MENU(MainWindow_EmuSpeed_0_5,                 wxWinMain::OnSetSpeed_0_5)
EVT_MENU(MainWindow_EmuSpeed_1_0,                 wxWinMain::OnSetSpeed_1_0)
EVT_MENU(MainWindow_EmuSpeed_1_5,                 wxWinMain::OnSetSpeed_1_5)
EVT_MENU(MainWindow_EmuSpeed_2_0,                 wxWinMain::OnSetSpeed_2_0)

EVT_MENU(MainWindow_Keymap,                       wxWinMain::OnConfigureKeymap)
EVT_MENU(MainWindow_DumpScreen,                   wxWinMain::OnDumpScreen)

//EVT_MOVE_END(                                     wxWinMain::OnWindowMove)

EVT_TIMER(MainWindow_TimerID,                     wxWinMain::OnTimerTick)

EVT_THREAD(wxEVT_COMMAND_TEXT_UPDATED,  wxWinMain::OnEmuCBThread)

EVT_CLOSE(wxWinMain::OnClose)
wxEND_EVENT_TABLE()


bool wxWinMain::Show(bool show)
{
  if(show)
  {
    this->tTimer.Start(200);
  }

  return(wxFrame::Show(show));
}

wxWinMain::wxWinMain(wxWindow* parent,
                     wxWindowID id,
                     const wxString& title,
                     const wxPoint& pos,
                     const wxSize& size,
                     long style )
          :wxFrame( parent, id, title, pos, size, style )
{
  this->SetSizeHints( wxDefaultSize, wxDefaultSize );

  mbarMain = new wxMenuBar( 0 );
  menuFile = new wxMenu();
  wxMenuItem* menuItemLoadFile;
  menuItemLoadFile = new wxMenuItem( menuFile, MainWindow_FileLoad, wxString( wxT("Load File...") ) , wxEmptyString, wxITEM_NORMAL );
  menuFile->Append( menuItemLoadFile );

  wxMenuItem* menuItemEmuRun;
  menuItemEmuRun = new wxMenuItem( menuFile, MainWindow_FileEmuRun, wxString( wxT("Set Run") ) , wxEmptyString, wxITEM_NORMAL );
  menuFile->Append( menuItemEmuRun );

  wxMenuItem* menuItemEmuPause;
  menuItemEmuPause = new wxMenuItem( menuFile, MainWindow_FileEmuPause, wxString( wxT("Set Pause") ) , wxEmptyString, wxITEM_NORMAL );
  menuFile->Append( menuItemEmuPause );

  mbarMain->Append( menuFile, wxT("File") );

  menuOptions = new wxMenu();
  smenuSpeed = new wxMenu();
  wxMenuItem* smenuSpeedItem = new wxMenuItem( menuOptions, wxID_ANY, wxT("Speed"), wxEmptyString, wxITEM_NORMAL, smenuSpeed );
  menuItemSpeed0_5 = new wxMenuItem( smenuSpeed, MainWindow_EmuSpeed_0_5, wxString( wxT("x0.5") ) , wxEmptyString, wxITEM_CHECK );
  smenuSpeed->Append( menuItemSpeed0_5 );

  menuItemSpeed1_0 = new wxMenuItem( smenuSpeed, MainWindow_EmuSpeed_1_0, wxString( wxT("x1.0") ) , wxEmptyString, wxITEM_CHECK );
  smenuSpeed->Append( menuItemSpeed1_0 );

  menuItemSpeed1_5 = new wxMenuItem( smenuSpeed, MainWindow_EmuSpeed_1_5, wxString( wxT("x1.5") ) , wxEmptyString, wxITEM_CHECK );
  smenuSpeed->Append( menuItemSpeed1_5 );

  menuItemSpeed2_0 = new wxMenuItem( smenuSpeed, MainWindow_EmuSpeed_2_0, wxString( wxT("x2.0") ) , wxEmptyString, wxITEM_CHECK );
  smenuSpeed->Append( menuItemSpeed2_0 );

  menuOptions->Append( smenuSpeedItem );

  wxMenuItem* menuItemKeymap;
  menuItemKeymap = new wxMenuItem( menuOptions, MainWindow_Keymap, wxString( wxT("Keymap...") ) , wxEmptyString, wxITEM_NORMAL );
  menuOptions->Append( menuItemKeymap );

  wxMenuItem* menuItemDumpScreen;
  menuItemDumpScreen = new wxMenuItem( menuOptions, MainWindow_DumpScreen, wxString( wxT("Dump Screen") ) , wxEmptyString, wxITEM_NORMAL );
  menuOptions->Append( menuItemDumpScreen );

  mbarMain->Append( menuOptions, wxT("Options") );

  menuHelp = new wxMenu();
  wxMenuItem* menuItemAbout;
  menuItemAbout = new wxMenuItem( menuHelp, MainWindow_About, wxString( wxT("About...") ) , wxEmptyString, wxITEM_NORMAL );
  menuHelp->Append( menuItemAbout );

  mbarMain->Append( menuHelp, wxT("Help") );

  this->SetMenuBar( mbarMain );

  statusBar = this->CreateStatusBar( 2, wxSTB_SIZEGRIP, MainWindow_StatusBar );
  tTimer.SetOwner( this, MainWindow_TimerID );

  this->Centre( wxBOTH );

  //Also initialise keymap window
  this->pWinKeymap = new wxWinKeyMap(this);
  this->SetIcon(wxICON(appicon));
}

wxWinMain::~wxWinMain()
{
  if(appConfig_Save(wxGetApp().appCfg))
  {
    wxMessageBox("appConfig_Save() failed, can't store your configuration!\n"
                 "Config File Path is: \n"
                 "\"" + wxString::FromAscii(appConfig_GetPath(wxGetApp().appCfg)) + "\"",
                 "Failed to save configuration",
                 wxOK | wxCENTER | wxICON_EXCLAMATION,
                 this);
  }
  appConfig_Close(wxGetApp().appCfg);
  this->Destroy();
}

void wxWinMain::OnSetSpeed_0_5(wxCommandEvent& WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  this->OptionSetSpeed(EMU_SPEED_0_5X);
}

void wxWinMain::OnSetSpeed_1_0(wxCommandEvent& WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  this->OptionSetSpeed(EMU_SPEED_1_0X);
}

void wxWinMain::OnSetSpeed_1_5(wxCommandEvent& WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  this->OptionSetSpeed(EMU_SPEED_1_5X);
}

void wxWinMain::OnSetSpeed_2_0(wxCommandEvent& WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  this->OptionSetSpeed(EMU_SPEED_2_0X);
}

void wxWinMain::OnConfigureKeymap(wxCommandEvent& WXUNUSED(event))
{
  this->pWinKeymap->Show(true);
}

void wxWinMain::OnDumpScreen(wxCommandEvent& WXUNUSED(event))
{
  chip8_DumpScreen();
}

void wxWinMain::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  wxAboutDialogInfo wAbout;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  wAbout.SetName(APP_DISPLAY_NAME);
  wAbout.SetVersion(wxString::Format("Version %d.%d.%d (%s)",
                                     APP_VERSION_MAJOR,
                                     APP_VERSION_MINOR,
                                     APP_VERSION_PATCH,
                                     APP_VERSION_DEVSTATE));
  wAbout.SetDescription("Welcome to this Emulator!\n"
                        "Here you can Emulate Chip-8 and Superchip Games.\n"
                        "Check out the Website for more Information and Updates.\n");
  wAbout.SetCopyright("(C) 2018-2019 by XXXBold");
  wAbout.SetWebSite(APP_PROJECT_HOMEPAGE);
  wAbout.AddDeveloper(APP_PROJECT_DEVELOPER);

  wxAboutBox(wAbout,this);
}

void wxWinMain::OnFileLoad(wxCommandEvent& WXUNUSED(event))
{
  wxString strPath;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(browseForFile(true,
                   "Browse for Chip-8 File",
                   strPath,
                   "Chip-8 File (.ch8)|*.ch8|All files (*.*)|*.*",
                   this,
                   wxGetApp().strApp_GetLastLoadPath()))
  {
    wxGetApp().vApp_SetLastLoadPath(strPath);
    if(chip8_LoadFile(strPath.ToAscii(),0x200))
    {
      wxPuts("chip8_LoadFile() failed");
      wxMessageBox("Failed to load File!",
                   "File load Error",
                   wxOK | wxCENTER | wxICON_EXCLAMATION);
    }
    else
      this->statusBar->SetStatusText("Load file: " + strPath,1);
  }
}

void wxWinMain::OnWindowMove(wxMoveEvent& WXUNUSED(event)) //not used atm
{
  int iPosX, iPosY;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  this->GetClientPosOnScreen(&iPosX,&iPosY);
  chip8_SetWindowPosition(iPosX,iPosY);
  chip8_SetWindowVisible(1);
}

void wxWinMain::OnEmuStateSetPause(wxCommandEvent& WXUNUSED(event))
{
  chip8_SetPause(1);
}

void wxWinMain::OnEmuStateSetRun(wxCommandEvent& WXUNUSED(event))
{
  chip8_SetPause(0);
}

void wxWinMain::OnClose(wxCloseEvent& WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  chip8_Close();
  this->Destroy();
}

void wxWinMain::OnEmuCBThread(wxThreadEvent& event)
{
  TagThreadEventParam tagData;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  tagData=event.GetPayload<TagThreadEventParam>();

  wxPuts(wxString::Format("event: 0x%X, opcode: 0x%X\n",tagData.uiEvent,tagData.tOpCode));

  switch(tagData.uiEvent)
  {
    case EMU_EVT_KEYPRESS_ESCAPE:
      chip8_SetPause(1);
      chip8_SetWindowVisible(0);
      break;

    case EMU_EVT_CHIP8_INSTRUCTION_EXECUTED:
      break;

    case EMU_EVT_SUCHIP_INSTRUCTION_EXECUTED:
      wxPuts("SUPERCHIP INSTRUCTION!");
      break;

    case EMU_EVT_INSTRUCTION_UNKNOWN:
      wxMessageBox(wxString::Format("Instruction unknown, OPCode: 0x%.4X!",tagData.tOpCode),
                   "Emulation Error",
                   wxOK | wxCENTER | wxICON_EXCLAMATION,
                   this);
      chip8_SetWindowVisible(0);
      break;

    case EMU_EVT_INSTRUCTION_ERROR:
      wxMessageBox(wxString::Format("Instruction Error, can't process OPCode: 0x%.4X!",tagData.tOpCode),
                   "Emulation Error",
                   wxOK | wxCENTER | wxICON_EXCLAMATION,
                   this);
      chip8_SetWindowVisible(0);
      break;

    default:
      wxPuts("Unknown ThreadEvent!");
      break;
  }
}

void wxWinMain::OnTimerTick(wxTimerEvent& WXUNUSED(event))
{
  wxString strStatus="Emulator State: ";
  switch(chip8_GetEmulatorState())
  {
    case EMU_STATE_RUN:
      this->statusBar->SetStatusText(strStatus + "RUNNING",0);
      break;
    case EMU_STATE_PAUSE:
      this->statusBar->SetStatusText(strStatus + "PAUSED",0);
      break;
    case EMU_STATE_INACTIVE:
      this->statusBar->SetStatusText(strStatus + "INACTIVE",0);
      break;
    case EMU_STATE_ERROR_INSTRUCTION:
      this->statusBar->SetStatusText(strStatus + "ERROR IN INSTRUCTION",0);
      break;
    case EMU_STATE_QUIT:
      this->statusBar->SetStatusText(strStatus + "QUIT",0);
      break;
    default:
      this->statusBar->SetStatusText(strStatus + "???",0);
      break;
  }
}

void wxWinMain::SetWindowScale(unsigned int uiScale)
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  this->SetClientSize(64*uiScale,32*uiScale);
  this->Centre();
}

void wxWinMain::GetClientPosOnScreen(int *piPosX, int *piPosY)
{
  wxRect tagRect;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  tagRect=this->GetClientRect();
  *piPosX=tagRect.GetX();
  *piPosY=tagRect.GetY();
  this->ClientToScreen(piPosX,piPosY);
}

void wxWinMain::OptionSetSpeed(int idCurrSpeed)
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  this->menuItemSpeed0_5->Check(false);
  this->menuItemSpeed1_0->Check(false);
  this->menuItemSpeed1_5->Check(false);
  this->menuItemSpeed2_0->Check(false);

  switch(idCurrSpeed)
  {
    case EMU_SPEED_0_5X:
      this->menuItemSpeed0_5->Check(true);
      chip8_SetEmulationSpeed(EMU_SPEED_0_5X);
      break;
    case EMU_SPEED_1_0X:
      this->menuItemSpeed1_0->Check(true);
      chip8_SetEmulationSpeed(EMU_SPEED_1_0X);
      break;
    case EMU_SPEED_1_5X:
      this->menuItemSpeed1_5->Check(true);
      chip8_SetEmulationSpeed(EMU_SPEED_1_5X);
      break;
    case EMU_SPEED_2_0X:
      this->menuItemSpeed2_0->Check(true);
      chip8_SetEmulationSpeed(EMU_SPEED_2_0X);
      break;
  }
}

bool browseForFile(bool openFile,
                   const wxString &description,
                   wxString &outPath,
                   const wxString &fileSelectionMask,
                   wxWindow *parent,
                   const wxString &defaultPath)
{
  wxFileDialog fileSelector(parent,
                            _(description),
                            defaultPath,
                            "",
                            fileSelectionMask,
                            (openFile)?wxFD_OPEN|wxFD_FILE_MUST_EXIST:wxFD_SAVE);
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(fileSelector.ShowModal() == wxID_CANCEL)
    return(false);
  outPath=fileSelector.GetPath();
  return(true);
}
