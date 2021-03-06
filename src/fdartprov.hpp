/* -*- Mode: C++; indent-tabs-mode: s; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * cody
 * Copyright (C) 2013 Émilien Kia <emilien.kia@gmail.com>
 *
cody is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cody is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
 */

#ifndef _FDARTPROV_HPP_
#define _FDARTPROV_HPP_

#include <wx/artprov.h>
#include <list>



class wxFreedesktopArtProvider: public wxArtProvider
{
public:
    wxFreedesktopArtProvider();
    wxFreedesktopArtProvider(const wxString& path);

    void Load(const wxString& path);

    virtual wxBitmap     CreateBitmap(const wxArtID &id, const wxArtClient &client, const wxSize &size);
    virtual wxIconBundle CreateIconBundle(const wxArtID &id, const wxArtClient &client);

protected:
    wxString m_path, m_WWHHPath, m_WWPath;

    struct IconDirDescription
    {
        wxString path;
        int      size;
        // TODO Context & Type
    };

    std::list<IconDirDescription> m_dirList;
};

#endif // _FDARTPROV_HPP_

