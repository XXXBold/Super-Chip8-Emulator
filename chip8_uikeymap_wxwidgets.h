#ifndef CHIP8_UIKEYMAP_WXWIDGETS_H_INCLUDED
  #define CHIP8_UIKEYMAP_WXWIDGETS_H_INCLUDED

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

class wxWinKeyMap : public wxFrame
{
  public:
    wxWinKeyMap(wxWindow* parent,
                wxWindowID id = wxID_ANY,
                const wxString& title = "Keymap",
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxSize( -1,-1 ),
                long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

    bool Show(bool show=true);

    void OnChoiceSelect(wxCommandEvent& event);

    void OnClose(wxCloseEvent& event);

  private:
    wxDECLARE_EVENT_TABLE();

    void vSaveKeymap();
    void vPopulateControls();
    void vUpdateKeymapControls();

    unsigned char ucaKeymapTmp[16];
    bool bKeymapChanged;
  protected:
    wxChoice* chEmuKey0;
    wxChoice* chEmuKey8;
    wxChoice* chEmuKey1;
    wxChoice* chEmuKey9;
    wxChoice* chEmuKey2;
    wxChoice* chEmuKeyA;
    wxChoice* chEmuKey3;
    wxChoice* chEmuKeyB;
    wxChoice* chEmuKey4;
    wxChoice* chEmuKeyC;
    wxChoice* chEmuKey5;
    wxChoice* chEmuKeyD;
    wxChoice* chEmuKey6;
    wxChoice* chEmuKeyE;
    wxChoice* chEmuKey7;
    wxChoice* chEmuKeyF;


};

#endif //CHIP8_UIKEYMAP_WXWIDGETS_H_INCLUDED
