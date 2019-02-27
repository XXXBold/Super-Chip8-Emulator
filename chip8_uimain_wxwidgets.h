#ifndef CHIP8_UIMAIN_WXWIDGETS_H_INCLUDED
  #define CHIP8_UIMAIN_WXWIDGETS_H_INCLUDED

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

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

class Chip8_GUI : public wxApp
{
public:
  virtual bool OnInit(void);
private:
};

class wxWinMain : public wxFrame
{
  public:

    wxWinMain(wxWindow* parent,
              wxWindowID id = wxID_ANY,
              const wxString& title = wxEmptyString,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxSize( 440,250 ),
              long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL);

    bool Show(bool show=true);

    void OnAbout(wxCommandEvent& event);
    void OnFileLoad(wxCommandEvent& event);
    void OnEmuStateSetPause(wxCommandEvent& event);
    void OnEmuStateSetRun(wxCommandEvent& event);
    void OnWindowMove(wxMoveEvent& event);
    void OnEmuCBThread(wxThreadEvent &event);
    void OnTimerTick(wxTimerEvent& event);

    void OnSetSpeed_0_5(wxCommandEvent& event);
    void OnSetSpeed_1_0(wxCommandEvent& event);
    void OnSetSpeed_1_5(wxCommandEvent& event);
    void OnSetSpeed_2_0(wxCommandEvent& event);

    void OnClose(wxCloseEvent& event);

    void SetWindowScale(unsigned int uiScale);
    void GetClientPosOnScreen(int *piPosX, int *piPosY);
    void OptionSetSpeed(int idCurrSpeed);

  private:
    wxDECLARE_EVENT_TABLE();
  protected:
    wxMenuBar* mbarMain;
    wxMenu* menuFile;
    wxMenu* menuOptions;
    wxMenu* smenuSpeed;
    wxMenuItem* menuItemSpeed0_5;
    wxMenuItem* menuItemSpeed1_0;
    wxMenuItem* menuItemSpeed1_5;
    wxMenuItem* menuItemSpeed2_0;
    wxMenu* menuHelp;
    wxStatusBar* statusBar;
    wxTimer tTimer;

};

#endif //CHIP8_UIMAIN_WXWIDGETS_H_INCLUDED
