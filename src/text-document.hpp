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

#ifndef _TEXT_DOCUMENT_HPP_
#define _TEXT_DOCUMENT_HPP_

class TextFrame;
class BookmarkList;

class wxAuiNotebook;
class wxStyledTextCtrl;

class TextDocument: public wxObject 
{
public:
	TextDocument(const wxString& title = "");

	bool loadFile(const wxString& file);
	bool reloadFile();
	bool saveFile();
	bool saveFileAs(const wxString& file);

	wxString getFile()const{return _file;}
	wxString getTitle()const{return _title;}

	void setTitle(const wxString& title = "");

	bool isMosified()const{return _modified;}
	void setModifed(bool modified = true){_modified = modified;}

	TextFrame* getFrame(){return _frame;}
	void setFrame(TextFrame* frame){_frame = frame;}

	wxAuiNotebook* getParent(){return _parent;}
	void setParent(wxAuiNotebook* parent){_parent = parent;}

	TextFrame* createFrame(wxAuiNotebook* parent);

	wxStyledTextCtrl* getMainCtrl();

	BookmarkList& getBookmarks();
	
protected:
	wxString _title, _file;
	bool _modified;
	
	TextFrame* _frame;
	wxAuiNotebook* _parent;

	void setTitleFromFile(const wxString& file);

};

#endif // _TEXT_DOCUMENT_HPP_