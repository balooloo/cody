/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * cody
 * Copyright (C) Émilien Kia 2012 <emilien.kia@gmail.com>
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

#include <wx/aui/auibook.h>
#include <wx/srchctrl.h>
#include <wx/spinctrl.h>
#include <wx/xrc/xmlres.h>

#include "text-frame.hpp"

#include "bookmark.hpp"
#include "main-frame.hpp"
#include "text-document.hpp"

enum TEXT_MARKERS
{
	TEXT_MARKER_BOOKMARK = 0
};

BEGIN_EVENT_TABLE(TextFrame, wxPanel)
	EVT_BUTTON(XRCID("FAST_FIND_CLOSE"), TextFrame::onCloseFastFind)
	EVT_BUTTON(wxID_FORWARD, TextFrame::onFastFindNext)
	EVT_BUTTON(wxID_BACKWARD, TextFrame::onFastFindPrev)
	EVT_TEXT(XRCID("FAST_FIND_TEXT"),TextFrame::onFastFindText)
	EVT_TEXT_ENTER(XRCID("FAST_FIND_TEXT"), TextFrame::onFastFindNext)
	EVT_TEXT_ENTER(XRCID("FAST_FIND_LINE"), TextFrame::onFastFindLine)
	EVT_TEXT_ENTER(XRCID("FAST_FIND_LINE_BTN"), TextFrame::onFastFindLine)

	EVT_STC_MODIFIED(wxID_ANY, TextFrame::OnTextModified)
END_EVENT_TABLE()

TextFrame::TextFrame(wxWindow* parent, TextDocument* doc):
wxPanel(parent, wxID_ANY),
_document(doc),
_fastFindShown(false)
{
	CommonInit();
}

void TextFrame::CommonInit()
{
	_mainText = new wxStyledTextCtrl(this, wxID_ANY);
	InitTextCtrl(_mainText);
	
	updateLineNbMargin();

	wxSizer* gsz = new wxBoxSizer(wxVERTICAL);
	gsz->Add(_mainText, 1, wxEXPAND);

	// Fast find
	_fastFindLine  = new wxSpinCtrl(this, XRCID("FAST_FIND_LINE"), "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_PROCESS_ENTER);
	_fastFindText  = new wxSearchCtrl(this, XRCID("FAST_FIND_TEXT"), "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	_fastFindClose = new wxBitmapButton(this, XRCID("FAST_FIND_CLOSE"), wxArtProvider::GetBitmap(wxART_CLOSE, wxART_BUTTON, wxSize(16,16)));
	_fastFindSizer = new wxBoxSizer(wxHORIZONTAL);
	_fastFindSizer->Add(_fastFindLine, 0, wxEXPAND|wxALL, 2);
	_fastFindSizer->Add(new wxBitmapButton(this, XRCID("FAST_FIND_LINE_BTN"), wxArtProvider::GetBitmap(wxART_GOTO_LAST, wxART_BUTTON, wxSize(16, 16))), 0, wxEXPAND|wxALL, 2);
	_fastFindSizer->AddSpacer(8);
	_fastFindSizer->Add(_fastFindText, 1, wxEXPAND|wxALL, 2);
	_fastFindSizer->Add(new wxBitmapButton(this, wxID_BACKWARD, wxArtProvider::GetBitmap(wxART_GO_BACK, wxART_BUTTON, wxSize(16, 16))), 0, wxEXPAND|wxALL, 2);
	_fastFindSizer->Add(new wxBitmapButton(this, wxID_FORWARD, wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_BUTTON, wxSize(16, 16))), 0, wxEXPAND|wxALL, 2);
	_fastFindSizer->AddSpacer(8);
	_fastFindSizer->Add(_fastFindClose, 0, wxEXPAND|wxALL, 2);
	gsz->Add(_fastFindSizer, 0, wxEXPAND);
	
	SetSizer(gsz);

	gsz->Hide(_fastFindSizer, true);
}

void TextFrame::InitTextCtrl(wxStyledTextCtrl* txt)
{
	if(!txt)
		return;

	txt->MarkerDefine(TEXT_MARKER_BOOKMARK, wxSTC_MARK_ARROW, wxColour(0, 0, 192), wxColour(0, 0, 255));
}

void TextFrame::initAfterLoading()
{
	addBookmarksFromProvider();
}

MainFrame* TextFrame::getMainFrame()
{
	wxWindow* win = this;
	while(win!=NULL)
	{
		MainFrame* frame = dynamic_cast<MainFrame*>(win);
		if(frame!=NULL)
			return frame;
		win = win->GetParent();
	}
	return NULL;
}

void TextFrame::showLineNumbers(bool show)
{
	_lineNrMarginShown = show;
	updateLineNbMargin();

}

void TextFrame::updateLineNbMargin()
{
	wxStyledTextCtrl* txt = getMainTextCtrl();
	if(txt)
	{
		if(_lineNrMarginShown)
		{
			int c = txt->GetLineCount();
			int sz = 0;
			if(c<=999)
				sz = txt->TextWidth (wxSTC_STYLE_LINENUMBER, _T("_999"));
			else if(c<=9999)
				sz = txt->TextWidth (wxSTC_STYLE_LINENUMBER, _T("_9999"));
			else if(c<=99999)
				sz = txt->TextWidth (wxSTC_STYLE_LINENUMBER, _T("_99999"));
			else if(c<=99999)
				sz = txt->TextWidth (wxSTC_STYLE_LINENUMBER, _T("_99999"));
			else
				sz = txt->TextWidth (wxSTC_STYLE_LINENUMBER, _T("_999999"));
			txt->SetMarginWidth(LineNrID, sz);
		}
		else
		{
			txt->SetMarginWidth(LineNrID, 0);
		}
	}
}

void TextFrame::setTitle(const wxString& title)
{
	wxAuiNotebook* notebook = getNotebook();
	if(notebook)
	{
		int idx = notebook->GetPageIndex(this);
		if(idx!=wxNOT_FOUND)
		{
			notebook->SetPageText(idx, title);
		}
	}
}

wxAuiNotebook* TextFrame::getNotebook()
{
	return dynamic_cast<wxAuiNotebook*>(GetParent());
}

void TextFrame::showFastFind(bool shown)
{
	GetSizer()->Show(_fastFindSizer, shown, true);
	_fastFindShown = shown;
	Layout();
}

void TextFrame::onCloseFastFind(wxCommandEvent& event)
{
	showFastFind(false);
}

void TextFrame::showFastFindText()
{
	showFastFind(true);
	
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		wxString sel = txt->GetSelectedText();
		if(!sel.IsEmpty())
		{
			_fastFindText->ChangeValue(sel);
			_fastFindText->SelectAll();
		}
		_fastFindText->SetFocus();
	}
}

void TextFrame::showFastFindGoToLine()
{
	showFastFind(true);
	
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		_fastFindLine->SetFocus();
	}
}

void TextFrame::onFastFindText(wxCommandEvent& event)
{
	findNext(0);
}

void TextFrame::onFastFindNext(wxCommandEvent& event)
{
	findNext();
}

void TextFrame::onFastFindPrev(wxCommandEvent& event)
{
	findPrev();
}

void TextFrame::findNext(int offset)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		wxString str = _fastFindText->GetValue();
		int curpos = txt->GetCurrentPos();
		if(txt->GetSelectionStart()>=0 && txt->GetSelectionStart()<curpos)
			curpos = txt->GetSelectionStart();
		if(!str.IsEmpty())
		{
			int pos = txt->FindText(curpos+offset, txt->GetTextLength(), str);
			if(pos>=0)
			{
				txt->SetSelection(pos, pos+str.Length());
			}
			else
			{
				pos = txt->FindText(0, txt->GetTextLength(), str);
				if(pos>=0)
				{
					txt->SetSelection(pos, pos+str.Length());
				}
			}
			txt->EnsureCaretVisible();
		}
	}	
}

void TextFrame::findPrev(int offset)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		wxString str = _fastFindText->GetValue();
		int curpos = txt->GetCurrentPos();
		if(txt->GetSelectionStart()>=0 && txt->GetSelectionStart()<curpos)
			curpos = txt->GetSelectionStart();
		if(!str.IsEmpty())
		{
			int pos = txt->FindText(0, curpos+offset, str);
			if(pos>=0)
			{
				txt->SetSelection(pos, pos+str.Length());
			}
			else
			{
				pos = txt->FindText(curpos+offset, txt->GetTextLength(), str);
				if(pos>=0)
				{
					txt->SetSelection(pos, pos+str.Length());
				}
			}
			txt->EnsureCaretVisible();
		}
	}
}

void TextFrame::onFastFindLine(wxCommandEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		int pos = txt->GetLineIndentPosition(_fastFindLine->GetValue()-1);
		txt->SetSelection(pos, pos);
		txt->EnsureCaretVisible();
		txt->SetFocus();
	}
}

void TextFrame::OnTextModified(wxStyledTextEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		int line  = txt->LineFromPosition(event.GetPosition()),
			lines = event.GetLinesAdded();

		if(lines!=0)
		{
			// Add or remove lines
			updateLineNbMargin();
			_fastFindLine->SetRange(1, getCurrentTextCtrl()->GetLineCount());
		
			// Bookmarks
			if(getDocument()->getBookmarks().addLines(line, lines))
				UpdateBookmarkPanel();
		}
	}
	
	event.Skip();
}

void TextFrame::toggleBookmark()
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		int line = txt->GetCurrentLine();
		BookmarkList& list = getDocument()->getBookmarks();
		if(list.has(line))
			remBookmark(line);
		else
			addBookmark(line);
	}
}

void TextFrame::addBookmark(int line, wxString name)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		// Define bookmark line
		if(line==wxNOT_FOUND)
			line = txt->GetCurrentLine();

		// Define bookmark name
		if(name.IsEmpty())
		{
			name = txt->GetCurLine(NULL);
			name.Trim();
			if(name.IsEmpty())
				name = wxGetTextFromUser(_("Name of the new bookmark"), _("New bookmark"), name);
		}
	
		if(!name.IsEmpty())
		{
			// Set bookmark for bookmark panel
			BookmarkList& list = getDocument()->getBookmarks();
			Bookmark bm = {line, name};
			list.insert(bm);
			UpdateBookmarkPanel();

			// Add Bookmark marker (Scintilla)
			txt->MarkerAdd(line, TEXT_MARKER_BOOKMARK);
		}
	}
}

void TextFrame::remBookmark(int line)
{
	getDocument()->getBookmarks().remove(line);
	UpdateBookmarkPanel();

	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		// Rem Bookmark marker (Scintilla)
		txt->MarkerDelete(line, TEXT_MARKER_BOOKMARK);
	}	
}

void TextFrame::clearBookmarks()
{
	getDocument()->getBookmarks().clear();
	UpdateBookmarkPanel();	

	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		// Rem Bookmark markers (Scintilla)
		txt->MarkerDeleteAll(TEXT_MARKER_BOOKMARK);
	}
}

void TextFrame::UpdateBookmarkPanel()
{
	MainFrame* frame = getMainFrame();
	if(frame)
	{
		BookmarkPanel* panel = frame->getBookmarkPanel();
		if(panel)
		{
			panel->update();
		}
	}
}

void TextFrame::addBookmarksFromProvider()
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		BookmarkList& list = getDocument()->getBookmarks();

		// Remove existing markers (Scintilla) then add from provider
		txt->MarkerDeleteAll(TEXT_MARKER_BOOKMARK);
		for(BookmarkList::iterator it=list.begin(); it!=list.end(); ++it)
		{
			txt->MarkerAdd(it->line, TEXT_MARKER_BOOKMARK);			
		}

		// Update Bookmark panel
		UpdateBookmarkPanel();
	}
}

void TextFrame::gotoPrevBookmark()
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		BookmarkList& list = getDocument()->getBookmarks();
		int line = list.getPrev(txt->GetCurrentLine()-1);
		if(line!=wxNOT_FOUND)
			gotoLine(line);
		else
		{
			line = list.getPrev(txt->GetLineCount());
			if(line!=wxNOT_FOUND)
				gotoLine(line);
		}
	}
}

void TextFrame::gotoNextBookmark()
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		BookmarkList& list = getDocument()->getBookmarks();
		int line = list.getNext(txt->GetCurrentLine()+1);
		if(line!=wxNOT_FOUND)
			gotoLine(line);
		else
		{
			line = list.getNext(-1);
			if(line!=wxNOT_FOUND)
				gotoLine(line);			
		}
	}
}

void TextFrame::gotoLine(int line)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
	{
		int pos = txt->PositionFromLine(line);
		txt->SetSelection(pos, pos);
	}
}

void TextFrame::setFocusToTextCtrl()
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	if(txt)
		txt->SetFocus();
}
