#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include "chip8_uimain_wxwidgets.h"
#include "chip8_Emulator.h"

#define APP_VERSION_MAJOR    0
#define APP_VERSION_MINOR    1
#define APP_DISPLAY_NAME     "Chip-8 Emulator"
#define APP_VERSION_DEVSTATE "Beta"

#define MY_WINDOW_SCALE 10u

typedef struct
{
  unsigned int uiEvent;
  word tOpCode;
}TagThreadEventParam;

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

int iEmuCBFunc_m(unsigned int event,
                 word currOPCode);

wxBEGIN_EVENT_TABLE(wxWinMain,wxFrame)
EVT_MENU(MainWindow_About,                        wxWinMain::OnAbout)
EVT_MENU(MainWindow_FileLoad,                     wxWinMain::OnFileLoad)
EVT_MENU(MainWindow_FileEmuRun,                   wxWinMain::OnEmuStateSetRun)
EVT_MENU(MainWindow_FileEmuPause,                 wxWinMain::OnEmuStateSetPause)
EVT_MENU(MainWindow_EmuSpeed_0_5,                 wxWinMain::OnSetSpeed_0_5)
EVT_MENU(MainWindow_EmuSpeed_1_0,                 wxWinMain::OnSetSpeed_1_0)
EVT_MENU(MainWindow_EmuSpeed_1_5,                 wxWinMain::OnSetSpeed_1_5)
EVT_MENU(MainWindow_EmuSpeed_2_0,                 wxWinMain::OnSetSpeed_2_0)

//EVT_MOVE_END(                                     wxWinMain::OnWindowMove)

EVT_TIMER(MainWindow_TimerID,                                wxWinMain::OnTimerTick)

EVT_THREAD(wxEVT_COMMAND_TEXT_UPDATED,  wxWinMain::OnEmuCBThread)

EVT_CLOSE(wxWinMain::OnClose)
wxEND_EVENT_TABLE()

IMPLEMENT_APP(Chip8_GUI)

wxDECLARE_APP(Chip8_GUI);

bool Chip8_GUI::OnInit()
{
  wxWinMain *wMain;
  int iPosX;
  int iPosY;

  if(!wxApp::OnInit())
    return(false);

  wMain = new wxWinMain(NULL,wxID_ANY,"Chip8-Emulator");
  wMain->SetWindowScale(MY_WINDOW_SCALE);
//wMain->GetClientPosOnScreen(&iPosX,&iPosY);
  iPosX=200;
  iPosY=200;
  if(chip8_Init(iPosX,
                iPosY,
                MY_WINDOW_SCALE,
                NULL,
                iEmuCBFunc_m,
                EMU_EVT_INSTRUCTION_ERROR | EMU_EVT_INSTRUCTION_UNKNOWN | EMU_EVT_KEYPRESS_ESCAPE))
  {
    DEBUG_WXPUTS("chip8_Init() failed, quit...");
    return(false);
  }
  wMain->OptionSetSpeed(EMU_SPEED_1_0X);
  wMain->Show(true);
  return(true);
}

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

  mbarMain->Append( menuOptions, wxT("Options") );

  menuHelp = new wxMenu();
  wxMenuItem* menuItemAbout;
  menuItemAbout = new wxMenuItem( menuHelp, MainWindow_About, wxString( wxT("About...") ) , wxEmptyString, wxITEM_NORMAL );
  menuHelp->Append( menuItemAbout );

  mbarMain->Append( menuHelp, wxT("Help") );

  this->SetMenuBar( mbarMain );

  statusBar = this->CreateStatusBar( 2, wxST_SIZEGRIP, MainWindow_StatusBar );
  tTimer.SetOwner( this, MainWindow_TimerID );

  this->Centre( wxBOTH );}

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

void wxWinMain::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  wxMessageBox(wxString::Format("Welcome to the %s!\n"
                                "Version %d.%d (%s)",
                                APP_DISPLAY_NAME,
                                APP_VERSION_MAJOR,
                                APP_VERSION_MINOR,
                                APP_VERSION_DEVSTATE),
               "About",
               wxOK|wxCENTER|wxICON_INFORMATION);
}

void wxWinMain::OnFileLoad(wxCommandEvent& WXUNUSED(event))
{
  wxString strPath;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(browseForFile(true,
                   "Browse for Chip-8 File",
                   strPath,
                   "Chip-8 File (.ch8)|*.ch8",
                   this))
  {
    if(chip8_LoadFile(strPath.ToAscii(),0x200))
    {
      wxPuts("chip8_LoadFile() failed");
      this->statusBar->SetStatusText("Failed to load the file!",1);
    }
    else
      this->statusBar->SetStatusText("Load file: " + strPath,1);
  }
}

void wxWinMain::OnWindowMove(wxMoveEvent& WXUNUSED(event))
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

    case EMU_EVT_INSTRUCTION_EXECUTED:
      break;

    case EMU_EVT_INSTRUCTION_UNKNOWN:
      break;

    case EMU_EVT_INSTRUCTION_ERROR:
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
