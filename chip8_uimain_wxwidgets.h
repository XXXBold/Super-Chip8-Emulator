#ifndef CHIP8_UIMAIN_WXWIDGETS_H_INCLUDED
  #define CHIP8_UIMAIN_WXWIDGETS_H_INCLUDED

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
  #include <wx/aboutdlg.h>
#endif

#include "chip8_uikeymap_wxwidgets.h"

class wxWinMain : public wxFrame
{
  public:
    wxWinMain(wxWindow* parent,
              wxWindowID id = wxID_ANY,
              const wxString& title = wxEmptyString,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxSize( 440,220 ),
              long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL);

    ~wxWinMain();

    bool Show(bool show=true);

    void OnAbout(wxCommandEvent& event);
    void OnFileLoad(wxCommandEvent& event);
    void OnEmuStateSetPause(wxCommandEvent& event);
    void OnEmuStateSetRun(wxCommandEvent& event);
    void OnEmuReset(wxCommandEvent& event);

    void OnWindowMove(wxMoveEvent& event);
    void OnEmuCBThread(wxThreadEvent &event);
    void OnTimerTick(wxTimerEvent& event);

    void OnSetSpeed(wxCommandEvent & event);
    void OnQuirkChange(wxCommandEvent& event);
    void OnConfigureKeymap(wxCommandEvent& event);
    void OnDumpScreen(wxCommandEvent& event);

    void OnClose(wxCloseEvent& event);

    void SetWindowScale(unsigned int uiScale);
    void GetClientPosOnScreen(int *piPosX, int *piPosY);

  private:
    wxWinKeyMap *pWinKeymap;
    wxIcon appIcon;
    wxAboutDialogInfo wAbout;

    wxDECLARE_EVENT_TABLE();
  protected:
    wxMenuBar* mbarMain;
    wxMenu* menuFile;
    wxMenu* menuOptions;
    wxMenu* smenuSpeed;
    wxMenuItem* menuItemSpeed0_25;
    wxMenuItem* menuItemSpeed0_5;
    wxMenuItem* menuItemSpeed1_0;
    wxMenuItem* menuItemSpeed1_5;
    wxMenuItem* menuItemSpeed2_0;
    wxMenuItem* menuItemSpeed5_0;
    wxMenuItem* menuItemSpeed10_0;
    wxMenu* smenuQuirks;
    wxMenuItem* menuItemQuirk_InstrIgnoreUnknown;
    wxMenuItem* menuItemQuirk_InstrIgnoreInvalid;
    wxMenuItem* menuItemQuirk_LDIncrementRegI;
    wxMenuItem* menuItemQuirk_ShiftSourceReg;
    wxMenu* menuHelp;
    wxStatusBar* statusBar;
    wxTimer tTimer;
};

#endif //CHIP8_UIMAIN_WXWIDGETS_H_INCLUDED
