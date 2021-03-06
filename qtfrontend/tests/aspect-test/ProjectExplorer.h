/***************************************************************************
    File                 : ProjectExplorer.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Description          : A tree view for displaying and editing an
                           AspectTreeModel.

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
#ifndef PROJECT_EXPLORER_H
#define PROJECT_EXPLORER_H

#include <QTreeView>

//! A tree view for displaying and editing an AspectTreeModel.
/**
 * Currently, the only functionality provided in addition to that of QTreeView
 * is usage of the context menus provided by AspectTreeModel.
 */
class ProjectExplorer : public QTreeView
{
	Q_OBJECT

	public:
		ProjectExplorer(QWidget *parent = 0) : QTreeView(parent) {}

	protected:
		virtual void contextMenuEvent(QContextMenuEvent *event);
};

#endif // ifndef PROJECT_EXPLORER_H
