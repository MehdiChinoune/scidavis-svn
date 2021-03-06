/***************************************************************************
    File                 : OpjImporter.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2007 by Ion Vasilief, Alex Kargovsky, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, kargovsky*yumr.phys.msu.su, thzs*gmx.net
    Description          : Origin project import class

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#ifndef IMPORTOPJ_H
#define IMPORTOPJ_H

#include "core/ApplicationWindow.h"

class OPJFile;

//! Origin project import class
class OpjImporter
{
public:
	OpjImporter(ApplicationWindow *app, const QString& filename);

	bool importTables (OPJFile opj);
	bool importGraphs (OPJFile opj);
	bool importNotes (OPJFile opj);
	int error(){return parse_error;};

private:
	int translateOrigin2SciDAVisLineStyle(int linestyle);
	QString parseOriginText(const QString &str);
	QString parseOriginTags(const QString &str);
	int parse_error;
	int xoffset;
	ApplicationWindow *mw;
};

#endif //IMPORTOPJ_H
