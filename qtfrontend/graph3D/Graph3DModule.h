/***************************************************************************
    File                 : Graph3DModule.h
    Project              : SciDAVis
    Description          : Module providing the 3D graph Part and support classes.
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email address)

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

#ifndef GRAPH3D_MODULE_H
#define GRAPH3D_MODULE_H

#include "core/interfaces.h"
#include "Graph3D.h"
#include <QMenu>

class Graph3DModule : public QObject, public PartMaker, public ActionManagerOwner, public ConfigPageMaker
{
	Q_OBJECT
	Q_INTERFACES(PartMaker ActionManagerOwner ConfigPageMaker)

	public:
		virtual AbstractPart * makePart();
		virtual QAction * makeAction(QObject *parent);
		virtual ActionManager * actionManager() { return Graph3D::actionManager(); }
		virtual void initActionManager();
		virtual ConfigPageWidget * makeConfigPage();
		virtual QString configPageLabel();
		virtual void loadSettings();
		virtual void saveSettings();
};

class Ui_Graph3DConfigPage;

//! Helper class for Graph3DModule
class Graph3DConfigPage : public ConfigPageWidget
{
	Q_OBJECT

	public:
		Graph3DConfigPage();
		~Graph3DConfigPage();

	public slots:
		virtual void apply();

	private:
		Ui_Graph3DConfigPage *ui;
};

#endif // ifndef GRAPH3D_MODULE_H

