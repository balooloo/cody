/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * cody
 * Copyright (C) Émilien Kia 2012 <emilien@emilien-vm-lm-maya>
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

#include <wx/artprov.h>
#include <wx/ribbon/buttonbar.h>
#include <wx/stc/stc.h>
#include <wx/xrc/xmlres.h>

#include "main-frame.hpp"
#include "cody-app.hpp"
#include "text-frame.hpp"
#include "text-document.hpp"
#include "bookmark.hpp"
#include "cody-app.hpp"


#define wxArtIcon(artid, sz) (wxArtProvider::GetBitmap(artid, wxART_OTHER, wxSize(sz,sz)))


MainFrame::MainFrame():
wxFrame(NULL, wxID_ANY, "Cody")
{
	CommonInit();

	_recentFileMenu = new wxMenu;
	wxGetApp().getFileHistory().UseMenu(_recentFileMenu);
	wxGetApp().getFileHistory().AddFilesToMenu(_recentFileMenu);
}

void MainFrame::CommonInit()
{
	wxSizer* gsz = new wxBoxSizer(wxVERTICAL);
	_ribbon = new wxRibbonBar(this, wxID_ANY, wxDefaultPosition, wxSize(800, 600), wxRIBBON_BAR_FLOW_HORIZONTAL
                                | wxRIBBON_BAR_SHOW_PAGE_LABELS
                                | wxRIBBON_BAR_SHOW_PANEL_EXT_BUTTONS
                                | wxRIBBON_BAR_SHOW_TOGGLE_BUTTON
                                //| wxRIBBON_BAR_SHOW_HELP_BUTTON
                                );
	InitRibbon();
	InitAcceleratorTable();
	
	_panel = new wxPanel(this, wxID_ANY);
	_manager.SetManagedWindow(_panel);
	
	gsz->Add(_ribbon, 0, wxEXPAND);
	gsz->Add(_panel, 1, wxEXPAND);
	SetSizer(gsz);

	_notebook = new wxAuiNotebook(_panel, wxID_ANY);
	_manager.AddPane(_notebook, wxAuiPaneInfo().CenterPane().PaneBorder(false));

	_bookmark = new BookmarkPanel(_panel, wxID_ANY);
	_manager.AddPane(_bookmark, wxAuiPaneInfo().Left().Caption("Bookmarks").MinSize(48, 48).Hide());
	
	_manager.Update();

	Layout();

	SetSize(800, 600);
}

void MainFrame::InitRibbon()
{
	{// File page
		wxRibbonPage* page = new wxRibbonPage(_ribbon, wxID_ANY, "File");
		{
			wxRibbonPanel* panel = new wxRibbonPanel(page, wxID_ANY, "Load");
			wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
			bar->AddButton(wxID_NEW, "New", wxArtIcon("document-new", 24));
			bar->AddHybridButton(wxID_OPEN, "Open", wxArtIcon("document-open", 24));
			bar->AddButton(wxID_REVERT_TO_SAVED, "Revert", wxArtIcon("document-revert", 24));
		}
		{
			wxRibbonPanel* panel = new wxRibbonPanel(page, wxID_ANY, "Save");
			wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
			bar->AddButton(wxID_SAVE, "Save", wxArtIcon("document-save", 24));
			bar->AddButton(wxID_SAVEAS, "Save as", wxArtIcon("document-save-as", 24));
			bar->AddButton(XRCID("Save all"), "Save all", wxArtIcon("document-save", 24));
		}
		{
			wxRibbonPanel* panel = new wxRibbonPanel(page, wxID_ANY, "Close");
			wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
			bar->AddButton(wxID_CLOSE, "Close", wxArtIcon("document-close", 24));
			bar->AddButton(XRCID("Close all"), "Close all", wxArtIcon("document-close", 24));
		}
		{
			wxRibbonPanel* panel = new wxRibbonPanel(page, wxID_ANY, "Help");
			wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
			bar->AddButton(wxID_HELP, "Help", wxArtIcon("help-contents", 24));
			bar->AddButton(wxID_ABOUT, "About Cody", wxArtIcon("help-about", 24));
		}
		{
			wxRibbonPanel* panel = new wxRibbonPanel(page, wxID_ANY, "");
			wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
			bar->AddButton(wxID_EXIT, "Quit", wxArtIcon("application-exit", 24));
		}
	}
	{// Edit page
		wxRibbonPage* page = new wxRibbonPage(_ribbon, wxID_ANY, "Edit");
		{
			wxRibbonPanel* panel = new wxRibbonPanel(page, wxID_ANY, "History");
			wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
			bar->AddButton(wxID_UNDO, "Undo", wxArtIcon("edit-undo", 24));
			bar->AddButton(wxID_REDO, "Redo", wxArtIcon("edit-redo", 24));
		}
		{
			wxRibbonPanel* panel = new wxRibbonPanel(page, wxID_ANY, "Clipboard");
			wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
			bar->AddButton(wxID_DELETE, "Delete", wxArtIcon("edit-delete", 24));
			bar->AddButton(wxID_CUT, "Cut", wxArtIcon("edit-cut", 24));
			bar->AddButton(wxID_COPY, "Copy", wxArtIcon("edit-copy", 24));
			bar->AddButton(wxID_PASTE, "Paste", wxArtIcon("edit-paste", 24));
		}
	}
	{// Navigate page
		wxRibbonPage* page = new wxRibbonPage(_ribbon, wxID_ANY, "Navigate");
		{
			wxRibbonPanel* panel = new wxRibbonPanel(page, XRCID("Search panel"), "Search", wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_DEFAULT_STYLE|wxRIBBON_PANEL_EXT_BUTTON);
			wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
			bar->AddButton(wxID_FIND, "Find", wxArtIcon("edit-find", 24));
			bar->AddButton(wxID_BACKWARD, "Find previous", wxArtIcon("go-previous", 24));
			bar->AddButton(wxID_FORWARD, "Find next", wxArtIcon("go-next", 24));
			bar->AddButton(XRCID("Go to line"), "Go to line", wxArtIcon("go-to-line", 24));
		}
		{
			wxRibbonPanel* panel = new wxRibbonPanel(page, XRCID("Bookmark panel"), "Bookmarks", wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_DEFAULT_STYLE|wxRIBBON_PANEL_EXT_BUTTON);
			wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
			bar->AddButton(XRCID("Toggle bookmark"), "Toggle", wxArtIcon("bookmark-toggle", 24));
			bar->AddButton(wxID_UP, "Previous", wxArtIcon("bookmark-prev", 24));
			bar->AddButton(wxID_DOWN, "Next", wxArtIcon("bookmark-next", 24));
		}
	}
	{// View page
		wxRibbonPage* page = new wxRibbonPage(_ribbon, wxID_ANY, "View");
		{
			wxRibbonPanel* panel = new wxRibbonPanel(page, wxID_ANY, "Decorations");
			wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
			bar->AddToggleButton(XRCID("Display caret line"), "Caret line", wxArtIcon("caret-line", 24));
			bar->AddToggleButton(XRCID("Display white spaces"), "White spaces", wxArtIcon("white-spaces", 24));
			bar->AddToggleButton(XRCID("Display indentation guides"), "Indentation guides", wxArtIcon("indent-lines", 24));
			bar->AddToggleButton(XRCID("Display end of lines"), "End of lines", wxArtIcon("end-of-lines", 24));
		}
		{
			wxRibbonPanel* panel = new wxRibbonPanel(page, wxID_ANY, "Margins");
			wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
			bar->AddToggleButton(XRCID("Display line numbers"), "Line numbers", wxArtIcon("line-numbers", 24));
			bar->AddToggleButton(XRCID("Display markers"), "Markers", wxArtIcon("marker-margin", 24));
			bar->AddToggleButton(XRCID("Display folders"), "Folders", wxArtIcon("folder-margin", 24));
		}
	}
	_ribbon->Realise();
}

void MainFrame::InitAcceleratorTable()
{
	static const size_t number = 17;
	wxAcceleratorEntry entries[number];
	
	// File/Load
	entries[0].Set(wxACCEL_CTRL, (int) 'N', wxID_NEW);
	entries[1].Set(wxACCEL_CTRL, (int) 'O', wxID_OPEN);
	entries[2].Set(wxACCEL_CTRL, (int) 'R', wxID_REVERT_TO_SAVED);
	// File/Save
	entries[3].Set(wxACCEL_CTRL, (int) 'S', wxID_SAVE);
	entries[4].Set(wxACCEL_CTRL|wxACCEL_SHIFT, (int) 'S', XRCID("Save all"));
	// File/Close
	entries[5].Set(wxACCEL_CTRL, (int) 'W', wxID_CLOSE);
	entries[6].Set(wxACCEL_CTRL|wxACCEL_SHIFT, (int) 'W', XRCID("Close all"));
	// File/Help
	entries[7].Set(wxACCEL_NORMAL, WXK_F1, wxID_HELP);
	entries[8].Set(wxACCEL_SHIFT, WXK_F1, wxID_ABOUT);
	// File / ""
	entries[9].Set(wxACCEL_CTRL, (int) 'Q', wxID_EXIT);

	// Navigate / Search
	entries[10].Set(wxACCEL_CTRL, (int) 'F', wxID_FIND);
	entries[11].Set(wxACCEL_NORMAL, WXK_F3, wxID_FORWARD);
	entries[12].Set(wxACCEL_SHIFT, WXK_F3, wxID_BACKWARD);
	entries[13].Set(wxACCEL_CTRL, (int) 'G', XRCID("Go to line"));
	// Navigate / Bookmarks
	entries[14].Set(wxACCEL_CTRL, WXK_F2, XRCID("Toggle bookmark"));
	entries[15].Set(wxACCEL_NORMAL, WXK_F2, wxID_DOWN);
	entries[16].Set(wxACCEL_SHIFT, WXK_F2, wxID_UP);
	
	wxAcceleratorTable accel(number, entries);
	SetAcceleratorTable(accel);
}

bool MainFrame::Destroy()
{
	if(!wxFrame::Destroy())
		return false;

	wxGetApp().getFileHistory().RemoveMenu(_recentFileMenu);
	delete _recentFileMenu;

	_manager.UnInit();

	return true;
}


wxStyledTextCtrl* MainFrame::getCurrentTextCtrl()
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame!=NULL)
	{
		return frame->getCurrentTextCtrl();
	}
	return NULL;
}

TextFrame* MainFrame::getCurrentTextFrame()
{
	wxWindow* window = _notebook->GetCurrentPage();
	if(!window)
		return NULL;
	return dynamic_cast<TextFrame*>(window);
}

TextDocument* MainFrame::getCurrentDocument()
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		return frame->getDocument();
	}
	return NULL;
}

void MainFrame::toggleBookmarkPanel()
{
	wxAuiPaneInfo& paneinfo = _manager.GetPane(_bookmark);
	paneinfo.Show(!paneinfo.IsShown());
	_manager.Update();
}


void MainFrame::onRibbonButtonClicked(wxEvent/*wxRibbonButtonBarEvent*/& event)
{
	wxRibbonButtonBarEvent *ribbonevent = dynamic_cast<wxRibbonButtonBarEvent*>(&event);
	
	// Process an equivalent wxEVT_COMMAND_MENU_SELECTED event for simulating menu events.
	// Used to unify RibbonButton click and accelerator entries.
	
	wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, event.GetId());
	if(ribbonevent)
		evt.SetInt(ribbonevent->GetInt());
	ProcessEvent(evt);

	event.Skip();
	
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_CUSTOM(wxEVT_COMMAND_RIBBONBUTTON_CLICKED, wxID_ANY, MainFrame::onRibbonButtonClicked)

	EVT_UPDATE_UI(wxID_SAVE, MainFrame::onUpdateHasOpenDocument)
	EVT_UPDATE_UI(wxID_SAVEAS, MainFrame::onUpdateHasOpenDocument)
	EVT_UPDATE_UI(XRCID("Save all"), MainFrame::onUpdateHasOpenDocument)
	EVT_UPDATE_UI(wxID_CLOSE, MainFrame::onUpdateHasOpenDocument)
	EVT_UPDATE_UI(XRCID("Close all"), MainFrame::onUpdateHasOpenDocument)

	EVT_UPDATE_UI(wxID_UNDO, MainFrame::onUpdateCanUndo)
	EVT_UPDATE_UI(wxID_REDO, MainFrame::onUpdateCanRedo)
	EVT_UPDATE_UI(wxID_DELETE, MainFrame::onUpdateHasOpenDocument)

	EVT_UPDATE_UI(wxID_CUT, MainFrame::onUpdateHasSelection)
	EVT_UPDATE_UI(wxID_COPY, MainFrame::onUpdateHasSelection)
	EVT_UPDATE_UI(wxID_PASTE, MainFrame::onUpdateCanPaste)

	EVT_MENU(wxID_NEW, MainFrame::onNewDocument)
	EVT_MENU(wxID_OPEN, MainFrame::onOpenDocument)
	EVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED(wxID_OPEN, MainFrame::onRecentDocumentMenu)
	EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, MainFrame::onRecentDocument)
	EVT_MENU(wxID_REVERT_TO_SAVED, MainFrame::onRevertDocument)
	EVT_MENU(wxID_SAVE, MainFrame::onSaveDocument)
	EVT_MENU(wxID_SAVEAS, MainFrame::onSaveDocumentAs)
	EVT_MENU(wxID_CLOSE, MainFrame::onCloseDocument)
	EVT_MENU(XRCID("Close all"), MainFrame::onCloseAllDocuments)

	EVT_MENU(wxID_UNDO, MainFrame::onUndo)
	EVT_MENU(wxID_REDO, MainFrame::onRedo)
	EVT_MENU(wxID_DELETE, MainFrame::onDelete)
	EVT_MENU(wxID_CUT, MainFrame::onCut)
	EVT_MENU(wxID_COPY, MainFrame::onCopy)
	EVT_MENU(wxID_PASTE, MainFrame::onPaste)

	EVT_RIBBONPANEL_EXTBUTTON_ACTIVATED(XRCID("Search panel"), MainFrame::onFindRibbonBarExtActivated)
	EVT_MENU(wxID_FIND, MainFrame::onFind)
	EVT_MENU(wxID_BACKWARD, MainFrame::onFindPrev)
	EVT_MENU(wxID_FORWARD, MainFrame::onFindNext)
	EVT_MENU(XRCID("Go to line"), MainFrame::onGoToLine)

	EVT_RIBBONPANEL_EXTBUTTON_ACTIVATED(XRCID("Bookmark panel"), MainFrame::onBookmarkRibbonBarExtActivated)
	EVT_MENU(XRCID("Toggle bookmark"), MainFrame::onToggleBookmark)
	EVT_MENU(wxID_UP, MainFrame::onPreviousBookmark)
	EVT_MENU(wxID_DOWN, MainFrame::onNextBookmark)

	EVT_MENU(XRCID("Display line numbers"), MainFrame::onDisplayLineNumber)
	EVT_UPDATE_UI(XRCID("Display line numbers"), MainFrame::onUpdateDisplayLineNumber)
	EVT_MENU(XRCID("Display markers"), MainFrame::onDisplayMarkers)
	EVT_UPDATE_UI(XRCID("Display markers"), MainFrame::onUpdateDisplayMarkers)
	EVT_MENU(XRCID("Display folders"), MainFrame::onDisplayFolders)
	EVT_UPDATE_UI(XRCID("Display folderes"), MainFrame::onUpdateDisplayFolders)

	EVT_MENU(XRCID("Display caret line"), MainFrame::onDisplayCaretLine)
	EVT_UPDATE_UI(XRCID("Display caret line"), MainFrame::onUpdateDisplayCaretLine)
	EVT_MENU(XRCID("Display white spaces"), MainFrame::onDisplayWhiteSpaces)
	EVT_UPDATE_UI(XRCID("Display white spaces"), MainFrame::onUpdateDisplayWhiteSpaces)
	EVT_MENU(XRCID("Display indentation guides"), MainFrame::onDisplayIndentationGuides)
	EVT_UPDATE_UI(XRCID("Display indentation guides"), MainFrame::onUpdateDisplayIndentationGuides)
	EVT_MENU(XRCID("Display end of lines"), MainFrame::onDisplayEOL)
	EVT_UPDATE_UI(XRCID("Display end of lines"), MainFrame::onUpdateDisplayEOL)

	EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, MainFrame::onPageClosing)
	EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, MainFrame::onPageChanged)
END_EVENT_TABLE()

void MainFrame::onNewDocument(wxCommandEvent& WXUNUSED(event))
{
	wxGetApp().createEmptyDocument(this);
}

void MainFrame::onOpenDocument(wxCommandEvent& event)
{
	wxGetApp().queryLoadFile(this);
}

void MainFrame::onRecentDocumentMenu(wxRibbonButtonBarEvent& event)
{
	event.PopupMenu(_recentFileMenu);
}

void MainFrame::onRecentDocument(wxCommandEvent& event)
{
	int id = event.GetId() - wxID_FILE1;
	if(id>=0 && id<wxGetApp().getFileHistory().GetCount())
	{
		wxString name = wxGetApp().getFileHistory().GetHistoryFile(id);
		if(!name.IsEmpty())
		{
			wxGetApp().loadDocument(name, this);
		}
	}
}

void MainFrame::onRevertDocument(wxCommandEvent& event)
{
	TextDocument* doc = getCurrentDocument();
	if(doc)
	{
		// TODO message box
		doc->reloadFile();
	}
}

void MainFrame::onSaveDocument(wxCommandEvent& event)
{
	TextDocument* doc = getCurrentDocument();
	if(doc)
	{
		if(!doc->getFile().IsEmpty())
			doc->saveFile();
		else
		{
			wxGetApp().saveDocumentAs(doc, this);
		}
	}
}

void MainFrame::onSaveDocumentAs(wxCommandEvent& event)
{
	TextDocument* doc = getCurrentDocument();
	if(doc)
	{
		wxGetApp().saveDocumentAs(doc, this);
	}
}

void MainFrame::onCloseDocument(wxCommandEvent& event)
{
	TextDocument* doc = getCurrentDocument();
	if(doc)
	{
		wxGetApp().closeDocument(doc);
	}
}

void MainFrame::onCloseAllDocuments(wxCommandEvent& event)
{
	wxGetApp().closeAllFrameDocuments(this);
}


void MainFrame::onPageClosing(wxAuiNotebookEvent& event)
{
	TextDocument* doc = getCurrentDocument();
	if(doc)
	{
		wxGetApp().closeDocument(doc);
		event.Veto();
	}
}

void MainFrame::onPageChanged(wxAuiNotebookEvent& event)
{
	if(getBookmarkPanel())
	{
		getBookmarkPanel()->setView(getCurrentTextFrame());
	}
}

void MainFrame::onUndo(wxCommandEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		txt->Undo();
	}
}

void MainFrame::onRedo(wxCommandEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		txt->Redo();
	}
}

void MainFrame::onDelete(wxCommandEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		txt->Clear();
	}
}

void MainFrame::onCut(wxCommandEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		txt->Cut();
	}
}

void MainFrame::onCopy(wxCommandEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		txt->Copy();
	}
}

void MainFrame::onPaste(wxCommandEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		txt->Paste();
	}
}

void MainFrame::onFindRibbonBarExtActivated(wxRibbonPanelEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->showFastFind(!frame->fastFindShown());
	}
}

void MainFrame::onFind(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->showFastFindText();
	}
}

void MainFrame::onFindNext(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->findNext();
	}
}

void MainFrame::onFindPrev(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->findPrev();
	}
}

void MainFrame::onGoToLine(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->showFastFindGoToLine();
	}
}

void MainFrame::onUpdateHasOpenDocument(wxUpdateUIEvent& event)
{
	event.Enable(getCurrentDocument()!=NULL);
}

void MainFrame::onUpdateCanUndo(wxUpdateUIEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt!=NULL)
	{
		event.Enable(txt->CanUndo());
	}
	else
	{
		event.Enable(false);
	}
}

void MainFrame::onUpdateCanRedo(wxUpdateUIEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt!=NULL)
	{
		event.Enable(txt->CanRedo());
	}
	else
	{
		event.Enable(false);
	}
}

void MainFrame::onUpdateCanPaste(wxUpdateUIEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt!=NULL)
	{
		event.Enable(txt->CanPaste());
	}
	else
	{
		event.Enable(false);
	}
}

void MainFrame::onUpdateHasSelection(wxUpdateUIEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt!=NULL)
	{
		int begin, end;
		txt->GetSelection(&begin, &end);
		event.Enable(end > begin);
	}
	else
	{
		event.Enable(false);
	}	
}

void MainFrame::onBookmarkRibbonBarExtActivated(wxRibbonPanelEvent& event)
{
	toggleBookmarkPanel();
}

void MainFrame::onToggleBookmark(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->toggleBookmark();
	}
}

void MainFrame::onPreviousBookmark(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->gotoPrevBookmark();
	}
}

void MainFrame::onNextBookmark(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->gotoNextBookmark();
	}
}

void MainFrame::onDisplayLineNumber(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->showLineNumbers(event.IsChecked());
	}	
}

void MainFrame::onUpdateDisplayLineNumber(wxUpdateUIEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		event.Check(frame->lineNumbersShown());
	}
}

void MainFrame::onDisplayMarkers(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->showMarkers(event.IsChecked());
	}	
}

void MainFrame::onUpdateDisplayMarkers(wxUpdateUIEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		event.Check(frame->markersShown());
	}
}

void MainFrame::onDisplayFolders(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->showFolders(event.IsChecked());
	}	
}

void MainFrame::onUpdateDisplayFolders(wxUpdateUIEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		event.Check(frame->foldersShown());
	}
}

void MainFrame::onDisplayCaretLine(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->showCaretLine(event.IsChecked());
	}
}

void MainFrame::onUpdateDisplayCaretLine(wxUpdateUIEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		event.Check(frame->caretLineShown());
	}
}

void MainFrame::onDisplayWhiteSpaces(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->showWhiteSpaces(event.IsChecked());
	}	
}

void MainFrame::onUpdateDisplayWhiteSpaces(wxUpdateUIEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		event.Check(frame->whiteSpacesShown());
	}
}

void MainFrame::onDisplayIndentationGuides(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->showIndentationGuides(event.IsChecked());
	}	
}

void MainFrame::onUpdateDisplayIndentationGuides(wxUpdateUIEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		event.Check(frame->indentationGuidesShown());
	}
}

void MainFrame::onDisplayEOL(wxCommandEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		frame->showEOL(event.IsChecked());
	}	
}

void MainFrame::onUpdateDisplayEOL(wxUpdateUIEvent& event)
{
	TextFrame* frame = getCurrentTextFrame();
	if(frame)
	{
		event.Check(frame->EOLShown());
	}
}
