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

#ifndef _CODY_APP_HPP_
#define _CODY_APP_HPP_

#include <set>

class MainFrame;
class TextDocument;

class wxRibbonButtonBarEvent;

class CodyApp : public wxApp
{
	DECLARE_EVENT_TABLE();
public:
    virtual bool OnInit();

	TextDocument* createEmptyDocument(MainFrame* mainFrame = NULL);
	TextDocument* loadDocument(const wxString& path, MainFrame* mainFrame = NULL);

	TextDocument* queryLoadFile(MainFrame* mainFrame = NULL);

	bool saveDocumentAs(TextDocument* doc, MainFrame* mainFrame = NULL);

	void closeDocument(TextDocument* doc);
	void closeAllFrameDocuments(MainFrame* mainFrame = NULL);
	
protected:
	MainFrame* _frame;

	std::set<TextDocument*> _documents;

private:
	void onAbout(wxRibbonButtonBarEvent& event);
	void onExit(wxRibbonButtonBarEvent& event);
};

DECLARE_APP(CodyApp)

#endif // _CODY_APP_HPP_
