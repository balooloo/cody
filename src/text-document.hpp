/* -*- Mode: C++; indent-tabs-mode: s; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * cody
 * Copyright (C) 2012-2013 Émilien KIA <emilien.kia@gmail.com>
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
class FileType;

class wxAuiNotebook;
class wxStyledTextCtrl;

#include "file-type.hpp"

class TextDocument: public wxObject
{
public:
    TextDocument(const wxString& title = "");

    bool loadFile(const wxString& file);
    bool reloadFile();
    bool saveFile();
    bool saveFileAs(const wxString& file);

    wxString getFile()const
    {
        return _file;
    }
    wxString getTitle()const
    {
        return _title;
    }

    void setTitle(const wxString& title = "");

    bool isModified()const
    {
        return _modified;
    }
    void setModified(bool modified = true);

    TextFrame* getFrame()
    {
        return _frame;
    }
    void setFrame(TextFrame* frame)
    {
        _frame = frame;
    }

    wxAuiNotebook* getParent()
    {
        return _parent;
    }
    void setParent(wxAuiNotebook* parent)
    {
        _parent = parent;
    }

    TextFrame* createFrame(wxAuiNotebook* parent);

    wxStyledTextCtrl* getMainCtrl() const;

    BookmarkList& getBookmarks();

    int getDocumentType()const
    {
        return _docType;
    }
    void setDocumentType(int doctype);
    void setDocumentType(const FileType& type);
    const FileType& getDocFileType()const;

    enum EOLMode {
      EOL_CRLF = wxSTC_EOL_CRLF,
      EOL_CR   = wxSTC_EOL_CR,
      EOL_LF   = wxSTC_EOL_LF,

#ifdef __WINDOWS__
      EOL_DEFAULT = EOL_CRLF,
#else
      EOL_DEFAULT = EOL_LF
#endif

    };

    EOLMode getEOLMode() const;
    void setEOLMode(EOLMode mode);
    void convertEOL(EOLMode mode);

    bool useTabs() const;
    void useTabs(bool use);
    int getIndent() const;
    void setIndent(int indent);
    void convertIndent();


protected:
    wxString _title, _file;
    bool _modified;

    int _docType;

    TextFrame* _frame;
    wxAuiNotebook* _parent;

    void setTitleFromFile(const wxString& file);
    void updateTitle();

private:
    static wxString createIndentation(int indent, int tabSize, bool insertSpaces);

};

#endif // _TEXT_DOCUMENT_HPP_
