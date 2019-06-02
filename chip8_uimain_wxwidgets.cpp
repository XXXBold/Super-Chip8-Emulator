#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
  #include <wx/aboutdlg.h>
#endif

#include "chip8_uimain_wxwidgets.h"
#include "chip8_app_wxwidgets.h"
#include "chip8_emulator.h"

#include "license.h"

#ifdef __linux__
  #include "chip8_logo.xpm"
#endif /* __linux__ */

enum
{
  // menu items
  MainWindow_FileLoad                           = 1,
  MainWindow_FileEmuRun,
  MainWindow_FileEmuPause,
  MainWindow_FileEmuReset,
  MainWindow_StatusBar,
  MainWindow_About                              = wxID_ABOUT,

  MainWindow_Quirk_LD_Increment_REG_I           = 10,
  MainWindow_Quirk_Shift_Source_REG,
  MainWindow_Quirk_Instr_Ignore_Unknown,
  MainWindow_Quirk_Instr_Ignore_Invalid,
  MainWindow_Keymap,
  MainWindow_DumpScreen,

  SPEED_ENUM_OFFSET                             = 20,
  MainWindow_EmuSpeed_0_25                      = EMU_SPEED_0_25X+SPEED_ENUM_OFFSET,
  MainWindow_EmuSpeed_0_5,
  MainWindow_EmuSpeed_1_0,
  MainWindow_EmuSpeed_1_5,
  MainWindow_EmuSpeed_2_0,
  MainWindow_EmuSpeed_5_0,
  MainWindow_EmuSpeed_10_0,
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
EVT_MENU(MainWindow_About,                             wxWinMain::OnAbout)
EVT_MENU(MainWindow_FileLoad,                          wxWinMain::OnFileLoad)
EVT_MENU(MainWindow_FileEmuRun,                        wxWinMain::OnEmuStateSetRun)
EVT_MENU(MainWindow_FileEmuPause,                      wxWinMain::OnEmuStateSetPause)
EVT_MENU(MainWindow_FileEmuReset,                      wxWinMain::OnEmuReset)

EVT_MENU(MainWindow_EmuSpeed_0_25,                     wxWinMain::OnSetSpeed)
EVT_MENU(MainWindow_EmuSpeed_0_5,                      wxWinMain::OnSetSpeed)
EVT_MENU(MainWindow_EmuSpeed_1_0,                      wxWinMain::OnSetSpeed)
EVT_MENU(MainWindow_EmuSpeed_1_5,                      wxWinMain::OnSetSpeed)
EVT_MENU(MainWindow_EmuSpeed_2_0,                      wxWinMain::OnSetSpeed)
EVT_MENU(MainWindow_EmuSpeed_5_0,                      wxWinMain::OnSetSpeed)
EVT_MENU(MainWindow_EmuSpeed_10_0,                     wxWinMain::OnSetSpeed)

/* Quirks */
EVT_MENU(MainWindow_Quirk_Instr_Ignore_Unknown,        wxWinMain::OnQuirkChange)
EVT_MENU(MainWindow_Quirk_Instr_Ignore_Invalid,        wxWinMain::OnQuirkChange)
EVT_MENU(MainWindow_Quirk_LD_Increment_REG_I,          wxWinMain::OnQuirkChange)
EVT_MENU(MainWindow_Quirk_Shift_Source_REG,            wxWinMain::OnQuirkChange)

EVT_MENU(MainWindow_Keymap,                            wxWinMain::OnConfigureKeymap)
EVT_MENU(MainWindow_DumpScreen,                        wxWinMain::OnDumpScreen)

//EVT_MOVE_END(                                     wxWinMain::OnWindowMove)

EVT_TIMER(MainWindow_TimerID,                          wxWinMain::OnTimerTick)

EVT_THREAD(wxEVT_COMMAND_TEXT_UPDATED,                 wxWinMain::OnEmuCBThread)

EVT_CLOSE(wxWinMain::OnClose)
wxEND_EVENT_TABLE()

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

  wxMenuItem* menuItemEmuReset;
  menuItemEmuReset = new wxMenuItem( menuFile, MainWindow_FileEmuReset, wxString( wxT("Reset") ) , wxEmptyString, wxITEM_NORMAL );
  menuFile->Append( menuItemEmuReset );

  mbarMain->Append( menuFile, wxT("File") );

  menuOptions = new wxMenu();
  smenuSpeed = new wxMenu();
  wxMenuItem* smenuSpeedItem = new wxMenuItem( menuOptions, wxID_ANY, wxT("Speed"), wxEmptyString, wxITEM_NORMAL, smenuSpeed );
  menuItemSpeed0_25 = new wxMenuItem( smenuSpeed, MainWindow_EmuSpeed_0_25, wxString( wxT("x0.25") ) , wxEmptyString, wxITEM_CHECK );
  smenuSpeed->Append( menuItemSpeed0_25 );

  menuItemSpeed0_5 = new wxMenuItem( smenuSpeed, MainWindow_EmuSpeed_0_5, wxString( wxT("x0.5") ) , wxEmptyString, wxITEM_CHECK );
  smenuSpeed->Append( menuItemSpeed0_5 );

  menuItemSpeed1_0 = new wxMenuItem( smenuSpeed, MainWindow_EmuSpeed_1_0, wxString( wxT("x1.0") ) , wxEmptyString, wxITEM_CHECK );
  smenuSpeed->Append( menuItemSpeed1_0 );

  menuItemSpeed1_5 = new wxMenuItem( smenuSpeed, MainWindow_EmuSpeed_1_5, wxString( wxT("x1.5") ) , wxEmptyString, wxITEM_CHECK );
  smenuSpeed->Append( menuItemSpeed1_5 );

  menuItemSpeed2_0 = new wxMenuItem( smenuSpeed, MainWindow_EmuSpeed_2_0, wxString( wxT("x2.0") ) , wxEmptyString, wxITEM_CHECK );
  smenuSpeed->Append( menuItemSpeed2_0 );

  menuItemSpeed5_0 = new wxMenuItem( smenuSpeed, MainWindow_EmuSpeed_5_0, wxString( wxT("x5") ) , wxEmptyString, wxITEM_CHECK );
  smenuSpeed->Append( menuItemSpeed5_0 );

  menuItemSpeed10_0 = new wxMenuItem( smenuSpeed, MainWindow_EmuSpeed_10_0, wxString( wxT("x10") ) , wxEmptyString, wxITEM_CHECK );
  smenuSpeed->Append( menuItemSpeed10_0 );

  menuOptions->Append( smenuSpeedItem );

  smenuQuirks = new wxMenu();
  wxMenuItem* smenuQuirksItem = new wxMenuItem( menuOptions, wxID_ANY, wxT("Quirks"), wxEmptyString, wxITEM_NORMAL, smenuQuirks );
  menuItemQuirk_InstrIgnoreUnknown = new wxMenuItem( smenuQuirks, MainWindow_Quirk_Instr_Ignore_Unknown, wxString( wxT("[All] Ignore(=Skip) Unknown Instructions") ) , wxEmptyString, wxITEM_CHECK );
  smenuQuirks->Append( menuItemQuirk_InstrIgnoreUnknown );

  menuItemQuirk_InstrIgnoreInvalid = new wxMenuItem( smenuQuirks, MainWindow_Quirk_Instr_Ignore_Invalid, wxString( wxT("[All] Ignore(=Skip) Invalid Instructions") ) , wxEmptyString, wxITEM_CHECK );
  smenuQuirks->Append( menuItemQuirk_InstrIgnoreInvalid );

  menuItemQuirk_LDIncrementRegI = new wxMenuItem( smenuQuirks, MainWindow_Quirk_LD_Increment_REG_I, wxString( wxT("[0xFX55,0xFX65] Increment Reg. I on LD") ) , wxEmptyString, wxITEM_CHECK );
  smenuQuirks->Append( menuItemQuirk_LDIncrementRegI );

  menuItemQuirk_ShiftSourceReg = new wxMenuItem( smenuQuirks, MainWindow_Quirk_Shift_Source_REG, wxString( wxT("[0x8X06,0x8X0E] Shift Source Register") ) , wxEmptyString, wxITEM_CHECK );
  smenuQuirks->Append( menuItemQuirk_ShiftSourceReg );

  menuOptions->Append( smenuQuirksItem );

  wxMenuItem* menuItemKeymap;
  menuItemKeymap = new wxMenuItem( menuOptions, MainWindow_Keymap, wxString( wxT("Keymap...") ) , wxEmptyString, wxITEM_NORMAL );
  menuOptions->Append( menuItemKeymap );

  wxMenuItem* menuItemDumpScreen;
  menuItemDumpScreen = new wxMenuItem( menuOptions, MainWindow_DumpScreen, wxString( wxT("Dump Screen") ) , wxEmptyString, wxITEM_NORMAL );
  menuOptions->Append( menuItemDumpScreen );

  mbarMain->Append( menuOptions, wxT("Options") );

  menuHelp = new wxMenu();
  wxMenuItem* menuItemAbout;
  menuItemAbout = new wxMenuItem( menuHelp, MainWindow_About, wxString( wxT("&About\tF1") ) , wxEmptyString, wxITEM_NORMAL );
  menuHelp->Append( menuItemAbout );

  mbarMain->Append( menuHelp, wxT("Help") );

  this->SetMenuBar( mbarMain );

  statusBar = this->CreateStatusBar( 2, wxSTB_SIZEGRIP, MainWindow_StatusBar );
  tTimer.SetOwner( this, MainWindow_TimerID );

  this->Centre( wxBOTH );

  this->Centre( wxBOTH );

  /* Loaded in ressource file (.rc) (Windows), linux: included above */
  this->appIcon=wxICON(chip8_logo);
}

wxWinMain::~wxWinMain()
{
  this->Destroy();
}

bool wxWinMain::Show(bool show)
{
  if(show)
  {
#ifdef __WXGTK__ /* GTK+ has a native control for licenses */
    this->wAbout.SetLicense(ucaLicense_m);
#else /* Shorten License text for non-native controls */
    this->wAbout.SetLicense(wxString::FromAscii(ucaLicense_m).Left(250) +
                            "...\n"
                            "Check out the Homepage for full License!");
#endif
    this->wAbout.SetIcon(this->appIcon);
    this->wAbout.SetName(APP_DISPLAY_NAME);
    this->wAbout.SetVersion(wxString::Format("Version %d.%d.%d (%s)\n"
                                             "%s (%s)",
                                             APP_VERSION_MAJOR,
                                             APP_VERSION_MINOR,
                                             APP_VERSION_PATCH,
                                             APP_VERSION_DEVSTATE,
                                             APP_VERSION_BUILDTIME,
                                             APP_VERSION_ARCH));
    this->wAbout.SetDescription("Welcome to this Emulator!\n"
                                "Here you can Emulate Chip-8 and Superchip ROMs.\n"
                                "Check out the Website for more Information and Updates.\n");
    this->wAbout.SetCopyright("(C) 2018-2019 by XXXBold");
    this->wAbout.SetWebSite(APP_PROJECT_HOMEPAGE);
    this->wAbout.AddDeveloper(APP_PROJECT_DEVELOPER);

    this->SetIcon(this->appIcon);
    this->pWinKeymap=new wxWinKeyMap(this,wxID_ANY,"Edit Keymap");
    this->menuItemSpeed1_0->Check(true);
    this->tTimer.Start(200);
  }
  return(wxFrame::Show(show));
}

void wxWinMain::OnSetSpeed(wxCommandEvent& event)
{
  int iCurrSpeed;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  iCurrSpeed=event.GetId()-SPEED_ENUM_OFFSET;

  this->menuItemSpeed0_25->Check(false);
  this->menuItemSpeed0_5->Check(false);
  this->menuItemSpeed1_0->Check(false);
  this->menuItemSpeed1_5->Check(false);
  this->menuItemSpeed2_0->Check(false);
  this->menuItemSpeed5_0->Check(false);
  this->menuItemSpeed10_0->Check(false);

  DEBUG_WXPUTS(wxString::Format("Set speed to %d",iCurrSpeed));
  switch(iCurrSpeed)
  {
    case EMU_SPEED_0_25X:
      this->menuItemSpeed0_25->Check(true);
      break;
    case EMU_SPEED_0_5X:
      this->menuItemSpeed0_5->Check(true);
      break;
    case EMU_SPEED_1_0X:
      this->menuItemSpeed1_0->Check(true);
      break;
    case EMU_SPEED_1_5X:
      this->menuItemSpeed1_5->Check(true);
      break;
    case EMU_SPEED_2_0X:
      this->menuItemSpeed2_0->Check(true);
      break;
    case EMU_SPEED_5_0X:
      this->menuItemSpeed5_0->Check(true);
      break;
    case EMU_SPEED_10_0X:
      this->menuItemSpeed10_0->Check(true);
      break;
    default:
      DEBUG_WXPUTS("Failed to set speed, unknown value! reset to 1.0...");
      iCurrSpeed=EMU_SPEED_1_0X;
      this->menuItemSpeed1_0->Check(true);
      break;
  }
  chip8_SetEmulationSpeed(static_cast<EEmulationSpeed>(iCurrSpeed));
}

void wxWinMain::OnQuirkChange(wxCommandEvent& WXUNUSED(event))
{
  unsigned int uiQuirks=0;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  if(this->menuItemQuirk_InstrIgnoreUnknown->IsChecked())
    uiQuirks|=EMU_QUIRK_SKIP_INSTRUCTIONS_UNKNOWN;
  if(this->menuItemQuirk_InstrIgnoreInvalid->IsChecked())
    uiQuirks|=EMU_QUIRK_SKIP_INSTRUCTIONS_INVALID;

  if(this->menuItemQuirk_LDIncrementRegI->IsChecked())
    uiQuirks|=EMU_QUIRK_INCREMENT_I_ON_STORAGE;
  if(this->menuItemQuirk_ShiftSourceReg->IsChecked())
    uiQuirks|=EMU_QUIRK_SHIFT_SOURCE_REG;

  chip8_EnableQuirks(uiQuirks);
}

void wxWinMain::OnConfigureKeymap(wxCommandEvent& WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  this->pWinKeymap->Show(true);
}

void wxWinMain::OnDumpScreen(wxCommandEvent& WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  chip8_DumpScreen(stderr);
}

void wxWinMain::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  wxAboutBox(wAbout,this);
}

void wxWinMain::OnFileLoad(wxCommandEvent& WXUNUSED(event))
{
  wxString strPath;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(browseForFile(true,
                   "Browse for Chip-8 File",
                   strPath,
#ifdef _WIN32
                   "Chip-8 File (.ch8)|*.ch8|All files (*.*)|*.*",
#else
                   "Chip-8 File (.ch8)|*.ch8|All files (*.*)|*",
#endif /* _WIN32 */
                   this,
                   wxGetApp().strApp_GetLastLoadPath()))
  {
    if(chip8_LoadFile(strPath.ToAscii(),0x200))
    {
      DEBUG_WXPUTS("chip8_LoadFile() failed");
      wxMessageBox("Failed to load File!",
                   "File load Error",
                   wxOK | wxCENTER | wxICON_EXCLAMATION);
    }
    else
    {
      this->statusBar->SetStatusText("Load file: "+strPath.AfterLast(wxFILE_SEP_PATH),1);
      chip8_SetWindowTitle(strPath.ToAscii());
      wxGetApp().vApp_SetLastLoadPath(strPath);
    }
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

void wxWinMain::OnEmuReset(wxCommandEvent& WXUNUSED(event))
{
  chip8_Reset();
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
      DEBUG_WXPUTS("SUPERCHIP INSTRUCTION!");
      break;

    case EMU_EVT_PROGRAM_EXIT:
      DEBUG_WXPUTS("END OF PROGRAM REACHED");
      break;

    case EMU_EVT_ERR_INSTRUCTION_UNKNOWN:
      wxMessageBox(wxString::Format("Instruction unknown, OPCode: 0x%.4X",tagData.tOpCode),
                   "Emulation Error",
                   wxOK | wxCENTER | wxICON_EXCLAMATION,
                   this);
      chip8_SetWindowVisible(0);
      break;

    case EMU_EVT_ERR_INVALID_INSTRUCTION:
      wxMessageBox(wxString::Format("Instruction Error, can't process OPCode: 0x%.4X",tagData.tOpCode),
                   "Emulation Error",
                   wxOK | wxCENTER | wxICON_EXCLAMATION,
                   this);
      chip8_SetWindowVisible(0);
      break;

    case EMU_EVT_ERR_INTERNAL:
      wxMessageBox(wxString::Format("Internal Error occured, last OPCode: 0x%.4X\n"
                                    "This should not happen, please report this issue.",tagData.tOpCode),
                   "Emulation Error",
                   wxOK | wxCENTER | wxICON_ERROR,
                   this);
      chip8_SetWindowVisible(0);
      break;

    default:
      DEBUG_WXPUTS("Unknown ThreadEvent!");
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
    case EMU_STATE_ERR_INVALID_INSTRUCTION:
      this->statusBar->SetStatusText(strStatus + "ERROR IN INSTRUCTION",0);
      break;
    case EMU_STATE_ERR_UNKNOWN_INSTRUCTION:
      this->statusBar->SetStatusText(strStatus + "UNKNOWN INSTRUCTION",0);
      break;
    case EMU_STATE_ERR_INTERNAL:
      this->statusBar->SetStatusText(strStatus + "INTERNAL ERROR",0);
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
