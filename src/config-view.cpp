/* -*- Mode: C++; indent-tabs-mode: s; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * cody
 * Copyright (C) 2013 Émilien KIA <emilien.kia@gmail.com>
 *
cody is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cody is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <wx/wx.h>

#include <wx/fileconf.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>

#include "config-view.hpp"

#include "cody-app.hpp"
#include "decls.hpp"
#include "text-document.hpp"
#include "text-frame.hpp"

enum ConfigViewID
{
    ConfigView_ShowCaretLine = 100,
    ConfigView_ShowWhiteSpaces,
    ConfigView_ShowIndentGuides,
    ConfigView_DefaultTabSize,
    ConfigView_DefaultUseTab,
    ConfigView_ShowEndOfLines,
    ConfigView_DefaultEndOfLines,
    ConfigView_WrapLongLines,

    ConfigView_MarginLineNumbers,
    ConfigView_MarginMarkers,
    ConfigView_MarginFolders,
    ConfigView_MarginLongLines


};


wxBEGIN_EVENT_TABLE(ConfigView, wxPanel)
EVT_CHECKBOX(ConfigView_ShowCaretLine, ConfigView::onCheckShowCaretLine)
EVT_CHECKBOX(ConfigView_ShowWhiteSpaces, ConfigView::onCheckShowWhiteSpaces)
EVT_CHECKBOX(ConfigView_ShowIndentGuides, ConfigView::onCheckShowIndentGuides)
EVT_SPINCTRL(ConfigView_DefaultTabSize, ConfigView::onSpinDefaultTabSize)
EVT_TEXT(ConfigView_DefaultTabSize, ConfigView::onSpinTextDefaultTabSize)
EVT_CHECKBOX(ConfigView_DefaultUseTab, ConfigView::onCheckDefaultUseTab)
EVT_CHECKBOX(ConfigView_ShowEndOfLines, ConfigView::onCheckShowEndOfLines)
EVT_CHOICE(ConfigView_DefaultEndOfLines, ConfigView::onChoiceDefaultEndOfLines)
EVT_CHECKBOX(ConfigView_WrapLongLines, ConfigView::onCheckWrapLongLines)

EVT_CHECKBOX(ConfigView_MarginLineNumbers, ConfigView::onCheckMarginLineNumbers)
EVT_CHECKBOX(ConfigView_MarginMarkers, ConfigView::onCheckMarginMarkers)
EVT_CHECKBOX(ConfigView_MarginFolders, ConfigView::onCheckMarginFolders)
EVT_CHECKBOX(ConfigView_MarginLongLines, ConfigView::onCheckMarginLongLines)
wxEND_EVENT_TABLE()

ConfigView::ConfigView(wxWindow* parent, wxWindowID id):
    wxPanel(parent, id)
{
    Initialize();
}

void ConfigView::Initialize()
{
    wxSizer* gsz = new wxBoxSizer(wxVERTICAL);

    wxFileConfig* conf = wxGetApp().getConfig();
    wxCheckBox *cb;
    wxChoice   *ch;

    {
        wxStaticBoxSizer *bsz = new wxStaticBoxSizer(wxVERTICAL, this, "Decorations");
	      wxSizer* sz;

        // Caret line
        cb = new wxCheckBox(bsz->GetStaticBox(), ConfigView_ShowCaretLine, "Caret line");
        cb->SetValue(conf->ReadBool(CONFPATH_EDITOR_SHOWCARETLINE, CONFDEFAULT_EDITOR_SHOWCARETLINE));
        bsz->Add(cb, 0, wxALL, 4);

        // White spaces
        cb = new wxCheckBox(bsz->GetStaticBox(), ConfigView_ShowWhiteSpaces, "White space");
        cb->SetValue(conf->ReadBool(CONFPATH_EDITOR_SHOWWHITESPACES, CONFDEFAULT_EDITOR_SHOWWHITESPACES));
        bsz->Add(cb, 0, wxALL, 4);

        // Indentation guides
        cb = new wxCheckBox(bsz->GetStaticBox(), ConfigView_ShowIndentGuides, "Indentation guides");
        cb->SetValue(conf->ReadBool(CONFPATH_EDITOR_SHOWINDENTGUIDES, CONFDEFAULT_EDITOR_SHOWINDENTGUIDES));
        bsz->Add(cb, 0, wxALL, 4);

        sz = new wxBoxSizer(wxHORIZONTAL);
        sz->Add(new wxStaticText(bsz->GetStaticBox(), wxID_ANY, "Tab size:"), 0, wxALIGN_CENTRE_VERTICAL);
        sz->Add(new wxSpinCtrl(bsz->GetStaticBox(), ConfigView_DefaultTabSize, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
          1, 64, conf->ReadLong(CONFPATH_EDITOR_DEFAULTTABSIZE, CONFDEFAULT_EDITOR_DEFAULTTABSIZE)), 0, wxALIGN_CENTRE_VERTICAL);
        sz->AddSpacer(16);
        cb = new wxCheckBox(bsz->GetStaticBox(), ConfigView_DefaultUseTab, "Use tabs");
        cb->SetValue(conf->ReadBool(CONFPATH_EDITOR_DEFAULTUSETAB, CONFDEFAULT_EDITOR_DEFAULTUSETAB));
        sz->Add(cb, 0, wxALIGN_CENTRE_VERTICAL|wxALL, 4);
        bsz->Add(sz, 0, wxALL, 4);
        
        // EOL
        cb = new wxCheckBox(bsz->GetStaticBox(), ConfigView_ShowEndOfLines, "End of lines");
        cb->SetValue(conf->ReadBool(CONFPATH_EDITOR_SHOWENDOFLINES, CONFDEFAULT_EDITOR_SHOWENDOFLINES));
        bsz->Add(cb, 0, wxALL, 4);

        sz = new wxBoxSizer(wxHORIZONTAL);
        sz->Add(new wxStaticText(bsz->GetStaticBox(), wxID_ANY, "Default ends of line:"), 0, wxALIGN_CENTRE_VERTICAL);
        static const wxString choices[]={/*"Autodetect",*/"CRLF", "CR", "LF"};
        sz->Add(ch = new wxChoice(bsz->GetStaticBox(), ConfigView_DefaultEndOfLines, wxDefaultPosition, wxDefaultSize, 3, choices), 0, wxALIGN_CENTRE_VERTICAL);
        ch->SetSelection(conf->ReadLong(CONFPATH_EDITOR_DEFAULTENDOFLINES, CONFDEFAULT_EDITOR_DEFAULTENDOFLINES));
        bsz->Add(sz, 0, wxALL, 4);

        // Wrap long lines
        cb = new wxCheckBox(bsz->GetStaticBox(), ConfigView_WrapLongLines, "Wrap long lines");
        cb->SetValue(conf->ReadBool(CONFPATH_EDITOR_WRAPLONGLINES, CONFDEFAULT_EDITOR_WRAPLONGLINES));
        bsz->Add(cb, 0, wxALL, 4);

        bsz->RecalcSizes();
        gsz->Add(bsz, 0, wxEXPAND|wxALL, 8);
    }

    {
        wxStaticBoxSizer *bsz = new wxStaticBoxSizer(wxVERTICAL, this, "Margins");

        // Line numbers
        cb = new wxCheckBox(bsz->GetStaticBox(), ConfigView_MarginLineNumbers, "Line numbers");
        cb->SetValue(conf->ReadBool(CONFPATH_EDITOR_MARGINLINENUMBERS, CONFDEFAULT_EDITOR_MARGINLINENUMBERS));
        bsz->Add(cb, 0, wxALL, 4);

        // Marker margin
        cb = new wxCheckBox(bsz->GetStaticBox(), ConfigView_MarginMarkers, "Markers");
        cb->SetValue(conf->ReadBool(CONFPATH_EDITOR_MARGINMARKERS, CONFDEFAULT_EDITOR_MARGINMARKERS));
        bsz->Add(cb, 0, wxALL, 4);

        // Folder margin
        cb = new wxCheckBox(bsz->GetStaticBox(), ConfigView_MarginFolders, "Folders");
        cb->SetValue(conf->ReadBool(CONFPATH_EDITOR_MARGINFOLDERS, CONFDEFAULT_EDITOR_MARGINFOLDERS));
        bsz->Add(cb, 0, wxALL, 4);

        // Long line margin
        cb = new wxCheckBox(bsz->GetStaticBox(), ConfigView_MarginLongLines, "Long lines");
        cb->SetValue(conf->ReadBool(CONFPATH_EDITOR_MARGINLONGLINES, CONFDEFAULT_EDITOR_MARGINLONGLINES));
        bsz->Add(cb, 0, wxALL, 4);

        bsz->RecalcSizes();
        gsz->Add(bsz, 0, wxEXPAND|wxALL, 8);
    }

    SetSizer(gsz);
}

void ConfigView::onCheckShowCaretLine(wxCommandEvent& event)
{
    bool checked = event.IsChecked();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_SHOWCARETLINE, checked);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->getFrame()->showCaretLine(checked);
    }
}

void ConfigView::onCheckShowWhiteSpaces(wxCommandEvent& event)
{
    bool checked = event.IsChecked();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_SHOWWHITESPACES, checked);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->getFrame()->showWhiteSpaces(checked);
    }
}

void ConfigView::onCheckShowIndentGuides(wxCommandEvent& event)
{
    bool checked = event.IsChecked();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_SHOWINDENTGUIDES, checked);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->getFrame()->showIndentationGuides(checked);
    }
}

void ConfigView::onSpinDefaultTabSize(wxSpinEvent& event)
{
	  int size = event.GetPosition();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_DEFAULTTABSIZE, size);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->setIndent(size);
    }
}

void ConfigView::onSpinTextDefaultTabSize(wxCommandEvent& event)
{
	  int size = event.GetInt();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_DEFAULTTABSIZE, size);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->setIndent(size);
    }
}

void ConfigView::onCheckDefaultUseTab(wxCommandEvent& event)
{
    bool checked = event.IsChecked();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_DEFAULTUSETAB, checked);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->useTabs(checked);
    }
}

void ConfigView::onCheckShowEndOfLines(wxCommandEvent& event)
{
    bool checked = event.IsChecked();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_SHOWENDOFLINES, checked);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->getFrame()->showEOL(checked);
    }
}

void ConfigView::onChoiceDefaultEndOfLines(wxCommandEvent& event)
{
	  int sel = event.GetInt();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_DEFAULTENDOFLINES, sel);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->setEOLMode((TextDocument::EOLMode)sel);
    }
}

void ConfigView::onCheckWrapLongLines(wxCommandEvent& event)
{
    bool checked = event.IsChecked();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_WRAPLONGLINES, checked);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->getFrame()->wrapLongLines(checked);
    }
}

void ConfigView::onCheckMarginLineNumbers(wxCommandEvent& event)
{
    bool checked = event.IsChecked();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_MARGINLINENUMBERS, checked);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->getFrame()->showLineNumbers(checked);
    }
}

void ConfigView::onCheckMarginMarkers(wxCommandEvent& event)
{
    bool checked = event.IsChecked();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_MARGINMARKERS, checked);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->getFrame()->showMarkers(checked);
    }
}

void ConfigView::onCheckMarginFolders(wxCommandEvent& event)
{
    bool checked = event.IsChecked();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_MARGINFOLDERS, checked);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->getFrame()->showFolders(checked);
    }
}

void ConfigView::onCheckMarginLongLines(wxCommandEvent& event)
{
    bool checked = event.IsChecked();
    wxGetApp().getConfig()->Write(CONFPATH_EDITOR_MARGINLONGLINES, checked);
    for(std::set<TextDocument*>::iterator it=wxGetApp().getDocuments().begin(); it!=wxGetApp().getDocuments().end(); ++it)
    {
        TextDocument* txt = *it;
        txt->getFrame()->showLongLines(checked);
    }
}
