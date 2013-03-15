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
#include <wx/splitter.h>
#include <wx/xrc/xmlres.h>

#include "text-frame.hpp"

#include "bookmark.hpp"
#include "main-frame.hpp"
#include "markbar.hpp"
#include "text-document.hpp"

enum TEXT_INDICATORS
{
	TEXT_INDICATOR_SEARCH = wxSTC_INDIC_CONTAINER
};

BEGIN_EVENT_TABLE(TextFrame, wxPanel)
	EVT_CHILD_FOCUS(TextFrame::onChildFocus)

	EVT_BUTTON(XRCID("FAST_FIND_CLOSE"), TextFrame::onCloseFastFind)
	EVT_BUTTON(wxID_FORWARD, TextFrame::onFastFindNext)
	EVT_BUTTON(wxID_BACKWARD, TextFrame::onFastFindPrev)
	EVT_TEXT(XRCID("FAST_FIND_TEXT"),TextFrame::onFastFindText)
	EVT_TEXT_ENTER(XRCID("FAST_FIND_TEXT"), TextFrame::onFastFindNext)
	EVT_TEXT_ENTER(XRCID("FAST_FIND_LINE"), TextFrame::onFastFindLine)
	EVT_TEXT_ENTER(XRCID("FAST_FIND_LINE_BTN"), TextFrame::onFastFindLine)

	EVT_STC_MODIFIED(wxID_ANY, TextFrame::OnTextModified)
	EVT_STC_UPDATEUI(wxID_ANY, TextFrame::onUpdateUI)

	MARKBAR_CLICK(wxID_ANY, TextFrame::onMarkerActivated)
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
	_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_NOBORDER|wxSP_PERMIT_UNSPLIT|wxSP_LIVE_UPDATE);

	_mainText    = new wxStyledTextCtrl(_splitter, wxID_ANY);
	_secondText  = new wxStyledTextCtrl(_splitter, wxID_ANY);
	_currentText = NULL;

	InitTextCtrl(_mainText);
	InitTextCtrl(_secondText);
	_secondText->SetDocPointer(_mainText->GetDocPointer());

	_splitter->Initialize(_mainText);
	_secondText->Hide();

	_markbar = new wxMarkBar(this, wxID_ANY, 0, 1, wxDefaultPosition, wxSize(14, -1), MB_VERTICAL|wxBORDER_NONE);

	wxSizer* gsz = new wxBoxSizer(wxHORIZONTAL);
	wxSizer* vsz = new wxBoxSizer(wxVERTICAL);
	vsz->Add(_splitter, 1, wxEXPAND);

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
	vsz->Add(_fastFindSizer, 0, wxEXPAND);

	gsz->Add(vsz, 1, wxEXPAND);
	gsz->Add(_markbar, 0, wxEXPAND);
	
	SetSizer(gsz);

	gsz->Hide(_fastFindSizer, true);

	// Initialize markers (Scintilla and MarkBar)
	setMarkerStyle(TEXT_MARKER_BOOKMARK, wxSTC_MARK_ARROW, wxColour(0, 0, 156), wxColour(0, 0, 192));
	setMarkerStyle(TEXT_MARKER_SEARCH,   wxSTC_MARK_ARROW,  wxColour(0, 156, 0), wxColour(0, 192, 0));
}

void TextFrame::InitTextCtrl(wxStyledTextCtrl* txt)
{
	if(!txt)
		return;

	// Initialize indicators
	txt->IndicatorSetStyle(TEXT_INDICATOR_SEARCH, wxSTC_INDIC_STRAIGHTBOX);
	txt->IndicatorSetForeground(TEXT_INDICATOR_SEARCH, wxColour(0, 156, 0));
	txt->IndicatorSetUnder(TEXT_INDICATOR_SEARCH, true);

	// Initialize margins
	txt->SetMarginType(MARGIN_LINE_NB, wxSTC_MARGIN_NUMBER);
	txt->SetMarginType(MARGIN_MARKER, wxSTC_MARGIN_SYMBOL);
	txt->SetMarginMask(MARGIN_MARKER, ~wxSTC_MASK_FOLDERS);
	txt->SetMarginType(MARGIN_FOLD, wxSTC_MARGIN_SYMBOL);
	txt->SetMarginMask(MARGIN_FOLD, wxSTC_MASK_FOLDERS);
	showLineNumbers(/*lineNumbersShown()*/);
	showMarkers(/*markersShown()*/);
	showFolders(/*foldersShown()*/);

	// Initialize long lines
	txt->SetEdgeColumn(80);
	txt->SetEdgeColour(*wxLIGHT_GREY);
	txt->SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_END);
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
	showMargin(MARGIN_LINE_NB, show);
	updateLineNbMargin();
}

bool TextFrame::lineNumbersShown()const
{
	return marginShown(MARGIN_LINE_NB);
}

void TextFrame::showMarkers(bool show)
{
	showMargin(MARGIN_MARKER, show);
	_mainText  ->SetMarginWidth(MARGIN_MARKER, show?16:0);
	_secondText->SetMarginWidth(MARGIN_MARKER, show?16:0);
}

bool TextFrame::markersShown()const
{
	return marginShown(MARGIN_MARKER);
}

void TextFrame::showFolders(bool show)
{
	showMargin(MARGIN_FOLD, show);
	_mainText  ->SetMarginWidth(MARGIN_FOLD, show?16:0);
	_secondText->SetMarginWidth(MARGIN_FOLD, show?16:0);
}

bool TextFrame::foldersShown()const
{
	return marginShown(MARGIN_FOLD);
}

void TextFrame::showCaretLine(bool show)
{
	_mainText  ->SetCaretLineVisible(show);
	_secondText->SetCaretLineVisible(show);
}

bool TextFrame::caretLineShown()const
{
	return _mainText->GetCaretLineVisible();
}

void TextFrame::showWhiteSpaces(bool show)
{
	_mainText  ->SetViewWhiteSpace(show?wxSTC_WS_VISIBLEALWAYS:wxSTC_WS_INVISIBLE);
	_secondText->SetViewWhiteSpace(show?wxSTC_WS_VISIBLEALWAYS:wxSTC_WS_INVISIBLE);
}

bool TextFrame::whiteSpacesShown()const
{
	return _mainText->GetViewWhiteSpace()!=wxSTC_WS_INVISIBLE;
}

void TextFrame::showIndentationGuides(bool show)
{
	_mainText  ->SetIndentationGuides(show?wxSTC_IV_LOOKFORWARD:wxSTC_IV_NONE);
	_secondText->SetIndentationGuides(show?wxSTC_IV_LOOKFORWARD:wxSTC_IV_NONE);
}

bool TextFrame::indentationGuidesShown()const
{
	return _mainText->GetIndentationGuides()!=wxSTC_IV_NONE;
}

void TextFrame::showEOL(bool show)
{
	_mainText  ->SetViewEOL(show);
	_secondText->SetViewEOL(show);
}

bool TextFrame::EOLShown()const
{
	return _mainText->GetViewEOL();
}

void TextFrame::showLongLines(bool show)
{
	_mainText  ->SetEdgeMode(show?wxSTC_EDGE_LINE:wxSTC_EDGE_NONE);
	_secondText->SetEdgeMode(show?wxSTC_EDGE_LINE:wxSTC_EDGE_NONE);
}

bool TextFrame::longLinesShown()const
{
	return _mainText->GetEdgeMode()!=wxSTC_EDGE_NONE;
}

void TextFrame::wrapLongLines(bool wrap)
{
	_mainText  ->SetWrapMode(wrap?wxSTC_WRAP_WORD:wxSTC_WRAP_NONE);
	_secondText->SetWrapMode(wrap?wxSTC_WRAP_WORD:wxSTC_WRAP_NONE);
}

bool TextFrame::longLinesWrapped()const
{
	return _mainText->GetEdgeMode()!=wxSTC_WRAP_NONE;
}

void TextFrame::setZoom(int scale)
{
	_mainText  ->SetZoom(scale);
	_secondText->SetZoom(scale);
}

int TextFrame::getScale()const
{
	return _mainText->GetZoom();
}

void TextFrame::zoomIn()
{
	_mainText  ->ZoomIn();
	_secondText->ZoomIn();
}

void TextFrame::zoomOut()
{
	_mainText  ->ZoomOut();
	_secondText->ZoomOut();
}

void TextFrame::updateLineNbMargin()
{
	if(marginShown(MARGIN_LINE_NB))
	{
		int c = _mainText->GetLineCount();
		int sz = 0;
		if(c<=999)
			sz = _mainText->TextWidth (wxSTC_STYLE_LINENUMBER, _T("_999"));
		else if(c<=9999)
			sz = _mainText->TextWidth (wxSTC_STYLE_LINENUMBER, _T("_9999"));
		else if(c<=99999)
			sz = _mainText->TextWidth (wxSTC_STYLE_LINENUMBER, _T("_99999"));
		else if(c<=99999)
			sz = _mainText->TextWidth (wxSTC_STYLE_LINENUMBER, _T("_99999"));
		else
			sz = _mainText->TextWidth (wxSTC_STYLE_LINENUMBER, _T("_999999"));
		_mainText  ->SetMarginWidth(MARGIN_LINE_NB, sz);
		_secondText->SetMarginWidth(MARGIN_LINE_NB, sz);
	}
	else
	{
		_mainText  ->SetMarginWidth(MARGIN_LINE_NB, 0);
		_secondText->SetMarginWidth(MARGIN_LINE_NB, 0);
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
			lines = event.GetLinesAdded(),
			linecount = txt->GetLineCount();

		
		if(lines!=0)
		{
			// Add or remove lines
			updateLineNbMargin();
			_fastFindLine->SetRange(1, linecount);
		
			// Bookmarks
			if(getDocument()->getBookmarks().addLines(line, lines))
				UpdateBookmarkPanel();

			// Markers
			_markbar->SetMax(linecount);
			_markbar->MoveMarkers(line, lines);
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

			// Add Bookmark marker
			addMarker(TEXT_MARKER_BOOKMARK, name, line);
		}
	}
}

void TextFrame::remBookmark(int line)
{
	getDocument()->getBookmarks().remove(line);
	UpdateBookmarkPanel();
	remMarker(TEXT_MARKER_BOOKMARK, line);
}

void TextFrame::clearBookmarks()
{
	getDocument()->getBookmarks().clear();
	UpdateBookmarkPanel();
	remMarkers(TEXT_MARKER_BOOKMARK);
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
	BookmarkList& list = getDocument()->getBookmarks();

	// Remove existing markers then add from provider
	remMarkers(TEXT_MARKER_BOOKMARK);
	for(BookmarkList::iterator it=list.begin(); it!=list.end(); ++it)
	{
		addMarker(TEXT_MARKER_BOOKMARK, it->name, it->line);
	}

	// Update Bookmark panel
	UpdateBookmarkPanel();
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

void TextFrame::onUpdateUI(wxStyledTextEvent& event)
{
	if(event.GetUpdated()==wxSTC_UPDATE_SELECTION)
	{
		onSelectionChanged();
	}
}

void TextFrame::onSelectionChanged()
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	int len = txt->GetLength();

	// Rem Bookmark markers
	remMarkers(TEXT_MARKER_SEARCH);

	// Rem indicators (Scintilla)
	txt->SetIndicatorCurrent(TEXT_INDICATOR_SEARCH);
	txt->IndicatorClearRange(0, len);
	
	int start = txt->GetSelectionStart();
	int end = txt->GetSelectionEnd();

	wxString sel = txt->GetSelectedText();
	int sellen = sel.Length(); 
	if(sellen>0)
	{
		int pos = -1;
		while(pos = txt->FindText(pos+1, len, sel), pos>=0 && pos<len )
		{
			// Add markers and indicators
			addMarker(TEXT_MARKER_SEARCH, sel, txt->LineFromPosition(pos));
			txt->IndicatorFillRange(pos, sellen);
		}
	}
	
}

void TextFrame::setMarkerStyle(int id, const wxBitmap &bmp, const wxColour& fore, const wxColour& back)
{
	_mainText  ->MarkerDefineBitmap(id, bmp);
	_secondText->MarkerDefineBitmap(id, bmp);
	_markbar->SetCategory(id, fore, back);
}

void TextFrame::setMarkerStyle(int id, int predefStyle, const wxColour& fore, const wxColour& back)
{
	_mainText  ->MarkerDefine(id, predefStyle, fore, back);
	_secondText->MarkerDefine(id, predefStyle, fore, back);
	_markbar->SetCategory(id, fore, back);
}

void TextFrame::addMarker(int id, const wxString& name, int line)
{
	_mainText->MarkerAdd(line, id);
	_markbar->AddMarker(line, name, id);
}

void TextFrame::remMarker(int id, int line)
{
	_mainText->MarkerDelete(line, id);
	_markbar->RemoveMarker(line, id);
}

void TextFrame::remMarkers(int id)
{
	_mainText->MarkerDeleteAll(id);
	_markbar->RemoveCategoryMarker(id);
}

void TextFrame::onMarkerActivated(wxMarkBarEvent& event)
{
	wxStyledTextCtrl* txt = getCurrentTextCtrl();
	txt->GotoLine(event.getPosition());
	setFocusToTextCtrl();
}

void TextFrame::showMargin(unsigned id, bool shown)
{
	if(shown)
	{
		_marginShown |= (1 << id);
	}
	else
	{
		_marginShown &= ~ (1 << id);
	}
}

bool TextFrame::marginShown(unsigned id)const
{
	return (_marginShown & (1 << id)) != 0;
}

void TextFrame::splitView(bool split)
{
	if(viewSplitted()==split)
		return; // If already in the wanted state, do nothing.

	if(split)
	{
		_splitter->Freeze();
		_secondText->Show();
		_splitter->SplitHorizontally(_mainText, _secondText);
		_splitter->Thaw();
	}
	else
	{
		_splitter->Freeze();
		_splitter->Unsplit(_secondText);
		_secondText->Hide();
		_splitter->Thaw();		
	}
}

bool TextFrame::viewSplitted()const
{
	return _splitter->IsSplit();
}

void TextFrame::onChildFocus(wxChildFocusEvent& event)
{
	if(event.GetWindow()==_mainText)
	{
		_currentText = _mainText;
	}
	else if(event.GetWindow()==_secondText)
	{
		_currentText = _secondText;
	}
}
