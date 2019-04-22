#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include "chip8_uikeymap_wxwidgets.h"
#include "chip8_app_wxwidgets.h"
#include "chip8_emulator.h"

enum
{
  // my wxIDs
  CHOICE_KEYMAP_ANY   =10,
};


wxBEGIN_EVENT_TABLE(wxWinKeyMap,wxFrame)
EVT_CHOICE(CHOICE_KEYMAP_ANY,wxWinKeyMap::OnChoiceSelect)

EVT_CLOSE(wxWinKeyMap::OnClose)
wxEND_EVENT_TABLE()

wxWinKeyMap::wxWinKeyMap(wxWindow* parent,
                         wxWindowID id,
                         const wxString& title,
                         const wxPoint& pos,
                         const wxSize& size,
                         long style )
            : wxFrame( parent, id, title, pos, size, style )
{
  this->SetSizeHints( wxDefaultSize, wxDefaultSize );

  wxFlexGridSizer* fgSizer1;
  fgSizer1 = new wxFlexGridSizer( 0, 4, 0, 0 );
  fgSizer1->SetFlexibleDirection( wxBOTH );
  fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  wxStaticText* strEmuKey_1;
  strEmuKey_1 = new wxStaticText( this, wxID_ANY, wxT("Emu Key:"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKey_1->Wrap( -1 );
  fgSizer1->Add( strEmuKey_1, 0, wxALL, 5 );

  wxStaticText* strKeyboard_1;
  strKeyboard_1 = new wxStaticText( this, wxID_ANY, wxT("Keyboard:"), wxDefaultPosition, wxDefaultSize, 0 );
  strKeyboard_1->Wrap( -1 );
  fgSizer1->Add( strKeyboard_1, 0, wxALL, 5 );

  wxStaticText* strEmuKey_2;
  strEmuKey_2 = new wxStaticText( this, wxID_ANY, wxT("Emu Key:"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKey_2->Wrap( -1 );
  fgSizer1->Add( strEmuKey_2, 0, wxALL, 5 );

  wxStaticText* strKeyboard_2;
  strKeyboard_2 = new wxStaticText( this, wxID_ANY, wxT("Keyboard:"), wxDefaultPosition, wxDefaultSize, 0 );
  strKeyboard_2->Wrap( -1 );
  fgSizer1->Add( strKeyboard_2, 0, wxALL, 5 );

  wxStaticText* strEmuKey0;
  strEmuKey0 = new wxStaticText( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKey0->Wrap( -1 );
  strEmuKey0->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKey0, 0, wxLEFT|wxRIGHT, 5 );

  wxArrayString chEmuKey0Choices;
  chEmuKey0 = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKey0Choices, 0 );
  chEmuKey0->SetSelection( 0 );
  fgSizer1->Add( chEmuKey0, 0, wxLEFT|wxRIGHT, 5 );

  wxStaticText* strEmuKey8;
  strEmuKey8 = new wxStaticText( this, wxID_ANY, wxT("8"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKey8->Wrap( -1 );
  strEmuKey8->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKey8, 0, wxLEFT|wxRIGHT, 5 );

  wxArrayString chEmuKey8Choices;
  chEmuKey8 = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKey8Choices, 0 );
  chEmuKey8->SetSelection( 0 );
  fgSizer1->Add( chEmuKey8, 0, wxLEFT|wxRIGHT, 5 );

  wxStaticText* strEmuKey1;
  strEmuKey1 = new wxStaticText( this, wxID_ANY, wxT("1"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKey1->Wrap( -1 );
  strEmuKey1->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKey1, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKey1Choices;
  chEmuKey1 = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKey1Choices, 0 );
  chEmuKey1->SetSelection( 0 );
  fgSizer1->Add( chEmuKey1, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKey9;
  strEmuKey9 = new wxStaticText( this, wxID_ANY, wxT("9"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKey9->Wrap( -1 );
  strEmuKey9->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKey9, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKey9Choices;
  chEmuKey9 = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKey9Choices, 0 );
  chEmuKey9->SetSelection( 0 );
  fgSizer1->Add( chEmuKey9, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKey2;
  strEmuKey2 = new wxStaticText( this, wxID_ANY, wxT("2"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKey2->Wrap( -1 );
  strEmuKey2->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKey2, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKey2Choices;
  chEmuKey2 = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKey2Choices, 0 );
  chEmuKey2->SetSelection( 0 );
  fgSizer1->Add( chEmuKey2, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKeyA;
  strEmuKeyA = new wxStaticText( this, wxID_ANY, wxT("A"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKeyA->Wrap( -1 );
  strEmuKeyA->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKeyA, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKeyAChoices;
  chEmuKeyA = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKeyAChoices, 0 );
  chEmuKeyA->SetSelection( 0 );
  fgSizer1->Add( chEmuKeyA, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKey3;
  strEmuKey3 = new wxStaticText( this, wxID_ANY, wxT("3"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKey3->Wrap( -1 );
  strEmuKey3->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKey3, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKey3Choices;
  chEmuKey3 = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKey3Choices, 0 );
  chEmuKey3->SetSelection( 0 );
  fgSizer1->Add( chEmuKey3, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKeyB;
  strEmuKeyB = new wxStaticText( this, wxID_ANY, wxT("B"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKeyB->Wrap( -1 );
  strEmuKeyB->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKeyB, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKeyBChoices;
  chEmuKeyB = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKeyBChoices, 0 );
  chEmuKeyB->SetSelection( 0 );
  fgSizer1->Add( chEmuKeyB, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKey4;
  strEmuKey4 = new wxStaticText( this, wxID_ANY, wxT("4"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKey4->Wrap( -1 );
  strEmuKey4->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKey4, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKey4Choices;
  chEmuKey4 = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKey4Choices, 0 );
  chEmuKey4->SetSelection( 0 );
  fgSizer1->Add( chEmuKey4, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKeyC;
  strEmuKeyC = new wxStaticText( this, wxID_ANY, wxT("C"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKeyC->Wrap( -1 );
  strEmuKeyC->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKeyC, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKeyCChoices;
  chEmuKeyC = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKeyCChoices, 0 );
  chEmuKeyC->SetSelection( 0 );
  fgSizer1->Add( chEmuKeyC, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKey5;
  strEmuKey5 = new wxStaticText( this, wxID_ANY, wxT("5"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKey5->Wrap( -1 );
  strEmuKey5->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKey5, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKey5Choices;
  chEmuKey5 = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKey5Choices, 0 );
  chEmuKey5->SetSelection( 0 );
  fgSizer1->Add( chEmuKey5, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKeyD;
  strEmuKeyD = new wxStaticText( this, wxID_ANY, wxT("D"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKeyD->Wrap( -1 );
  strEmuKeyD->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKeyD, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKeyDChoices;
  chEmuKeyD = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKeyDChoices, 0 );
  chEmuKeyD->SetSelection( 0 );
  fgSizer1->Add( chEmuKeyD, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKey6;
  strEmuKey6 = new wxStaticText( this, wxID_ANY, wxT("6"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKey6->Wrap( -1 );
  strEmuKey6->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKey6, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKey6Choices;
  chEmuKey6 = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKey6Choices, 0 );
  chEmuKey6->SetSelection( 0 );
  fgSizer1->Add( chEmuKey6, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKeyE;
  strEmuKeyE = new wxStaticText( this, wxID_ANY, wxT("E"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKeyE->Wrap( -1 );
  strEmuKeyE->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKeyE, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKeyEChoices;
  chEmuKeyE = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKeyEChoices, 0 );
  chEmuKeyE->SetSelection( 0 );
  fgSizer1->Add( chEmuKeyE, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKey7;
  strEmuKey7 = new wxStaticText( this, wxID_ANY, wxT("7"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKey7->Wrap( -1 );
  strEmuKey7->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKey7, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKey7Choices;
  chEmuKey7 = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKey7Choices, 0 );
  chEmuKey7->SetSelection( 0 );
  fgSizer1->Add( chEmuKey7, 0, wxRIGHT|wxLEFT, 5 );

  wxStaticText* strEmuKeyF;
  strEmuKeyF = new wxStaticText( this, wxID_ANY, wxT("F"), wxDefaultPosition, wxDefaultSize, 0 );
  strEmuKeyF->Wrap( -1 );
  strEmuKeyF->SetFont( wxFont( 15, 70, 90, 92, false, wxEmptyString ) );

  fgSizer1->Add( strEmuKeyF, 0, wxRIGHT|wxLEFT, 5 );

  wxArrayString chEmuKeyFChoices;
  chEmuKeyF = new wxChoice( this, CHOICE_KEYMAP_ANY, wxDefaultPosition, wxDefaultSize, chEmuKeyFChoices, 0 );
  chEmuKeyF->SetSelection( 0 );
  fgSizer1->Add( chEmuKeyF, 0, wxRIGHT|wxLEFT, 5 );


  this->SetSizer( fgSizer1 );
  this->Layout();
  fgSizer1->Fit( this );

  this->Centre( wxBOTH );

  this->vPopulateControls();
}

bool wxWinKeyMap::Show(bool show)
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(show)
  {
    this->vUpdateKeymapControls();
    this->bKeymapChanged=false;
  }

  return(wxFrame::Show(show));
}

void wxWinKeyMap::OnChoiceSelect(wxCommandEvent& WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  this->bKeymapChanged=true;
}

void wxWinKeyMap::OnClose(wxCloseEvent& WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(this->bKeymapChanged)
  {
    switch(wxMessageBox("Do you want to save changes?",
                        "Save Keymap",
                        wxYES_NO|wxCANCEL))
    {
      case wxCANCEL:
        return;
      case wxYES:
        this->vSaveKeymap();
        /* Fall through okay */
      case wxNO:
        break;
      default:
        wxPuts("Unknown return from message box");
    }
  }
  this->Show(false);
}

void wxWinKeyMap::vSaveKeymap()
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  this->ucaKeymapTmp[0x0]=this->chEmuKey0->GetSelection();
  this->ucaKeymapTmp[0x1]=this->chEmuKey1->GetSelection();
  this->ucaKeymapTmp[0x2]=this->chEmuKey2->GetSelection();
  this->ucaKeymapTmp[0x3]=this->chEmuKey3->GetSelection();
  this->ucaKeymapTmp[0x4]=this->chEmuKey4->GetSelection();
  this->ucaKeymapTmp[0x5]=this->chEmuKey5->GetSelection();
  this->ucaKeymapTmp[0x6]=this->chEmuKey6->GetSelection();
  this->ucaKeymapTmp[0x7]=this->chEmuKey7->GetSelection();
  this->ucaKeymapTmp[0x8]=this->chEmuKey8->GetSelection();
  this->ucaKeymapTmp[0x9]=this->chEmuKey9->GetSelection();
  this->ucaKeymapTmp[0xA]=this->chEmuKeyA->GetSelection();
  this->ucaKeymapTmp[0xB]=this->chEmuKeyB->GetSelection();
  this->ucaKeymapTmp[0xC]=this->chEmuKeyC->GetSelection();
  this->ucaKeymapTmp[0xD]=this->chEmuKeyD->GetSelection();
  this->ucaKeymapTmp[0xE]=this->chEmuKeyE->GetSelection();
  this->ucaKeymapTmp[0xF]=this->chEmuKeyF->GetSelection();

  wxGetApp().vApp_SetKeymap(this->ucaKeymapTmp);
}

void wxWinKeyMap::vUpdateKeymapControls()
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  wxGetApp().vApp_GetKeymap(this->ucaKeymapTmp);

  this->chEmuKey0->SetSelection(this->ucaKeymapTmp[0x0]);
  this->chEmuKey1->SetSelection(this->ucaKeymapTmp[0x1]);
  this->chEmuKey2->SetSelection(this->ucaKeymapTmp[0x2]);
  this->chEmuKey3->SetSelection(this->ucaKeymapTmp[0x3]);
  this->chEmuKey4->SetSelection(this->ucaKeymapTmp[0x4]);
  this->chEmuKey5->SetSelection(this->ucaKeymapTmp[0x5]);
  this->chEmuKey6->SetSelection(this->ucaKeymapTmp[0x6]);
  this->chEmuKey7->SetSelection(this->ucaKeymapTmp[0x7]);
  this->chEmuKey8->SetSelection(this->ucaKeymapTmp[0x8]);
  this->chEmuKey9->SetSelection(this->ucaKeymapTmp[0x9]);
  this->chEmuKeyA->SetSelection(this->ucaKeymapTmp[0xA]);
  this->chEmuKeyB->SetSelection(this->ucaKeymapTmp[0xB]);
  this->chEmuKeyC->SetSelection(this->ucaKeymapTmp[0xC]);
  this->chEmuKeyD->SetSelection(this->ucaKeymapTmp[0xD]);
  this->chEmuKeyE->SetSelection(this->ucaKeymapTmp[0xE]);
  this->chEmuKeyF->SetSelection(this->ucaKeymapTmp[0xF]);

}

void wxWinKeyMap::vPopulateControls()
{
  wxArrayString arrItems;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  arrItems.Add("0");
  arrItems.Add("1");
  arrItems.Add("2");
  arrItems.Add("3");
  arrItems.Add("4");
  arrItems.Add("5");
  arrItems.Add("6");
  arrItems.Add("7");
  arrItems.Add("8");
  arrItems.Add("9");
  arrItems.Add("A");
  arrItems.Add("B");
  arrItems.Add("C");
  arrItems.Add("D");
  arrItems.Add("E");
  arrItems.Add("F");
  arrItems.Add("G");
  arrItems.Add("H");
  arrItems.Add("I");
  arrItems.Add("J");
  arrItems.Add("K");
  arrItems.Add("L");
  arrItems.Add("N");
  arrItems.Add("M");
  arrItems.Add("O");
  arrItems.Add("P");
  arrItems.Add("Q");
  arrItems.Add("R");
  arrItems.Add("S");
  arrItems.Add("T");
  arrItems.Add("U");
  arrItems.Add("V");
  arrItems.Add("W");
  arrItems.Add("X");
  arrItems.Add("Y");
  arrItems.Add("Z");
  arrItems.Add("UP");
  arrItems.Add("DOWN");
  arrItems.Add("LEFT");
  arrItems.Add("RIGHT");

  this->chEmuKey0->Clear();
  this->chEmuKey1->Clear();
  this->chEmuKey2->Clear();
  this->chEmuKey3->Clear();
  this->chEmuKey4->Clear();
  this->chEmuKey5->Clear();
  this->chEmuKey6->Clear();
  this->chEmuKey7->Clear();
  this->chEmuKey8->Clear();
  this->chEmuKey9->Clear();
  this->chEmuKeyA->Clear();
  this->chEmuKeyB->Clear();
  this->chEmuKeyC->Clear();
  this->chEmuKeyD->Clear();
  this->chEmuKeyE->Clear();
  this->chEmuKeyF->Clear();

  this->chEmuKey0->Append(arrItems);
  this->chEmuKey1->Append(arrItems);
  this->chEmuKey2->Append(arrItems);
  this->chEmuKey3->Append(arrItems);
  this->chEmuKey4->Append(arrItems);
  this->chEmuKey5->Append(arrItems);
  this->chEmuKey6->Append(arrItems);
  this->chEmuKey7->Append(arrItems);
  this->chEmuKey8->Append(arrItems);
  this->chEmuKey9->Append(arrItems);
  this->chEmuKeyA->Append(arrItems);
  this->chEmuKeyB->Append(arrItems);
  this->chEmuKeyC->Append(arrItems);
  this->chEmuKeyD->Append(arrItems);
  this->chEmuKeyE->Append(arrItems);
  this->chEmuKeyF->Append(arrItems);
}
