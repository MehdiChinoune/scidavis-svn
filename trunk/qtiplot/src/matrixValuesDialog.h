/***************************************************************************
    File                 : matrixValuesDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, 
                           Tilman Hoener zu Siederdissen,
                           Knut Franke
    Email                : ion_vasilief@yahoo.fr, thzs@gmx.net
                           knut.franke@gmx.de
    Description          : Set matrix values dialog
                           
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
#ifndef MVALUESDIALOG_H
#define MVALUESDIALOG_H

#include "Scripting.h"

#include <qvariant.h>
#include <qdialog.h>

class QComboBox;
class Q3TextEdit;
class ScriptEdit;
class QSpinBox;
class QPushButton;
class Matrix;
	
//! Set matrix values dialog
class MatrixValuesDialog : public QDialog, public scripted
{ 
    Q_OBJECT

public:
    MatrixValuesDialog( ScriptingEnv *env, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~MatrixValuesDialog();
	
	QSize sizeHint() const ;

	void customEvent( QEvent *e);

    QComboBox* functions;
    QPushButton* PushButton3; 
    QPushButton* btnOk, *btnAddCell;
    QPushButton* btnCancel;
    ScriptEdit* commands;
    Q3TextEdit* explain;
	QSpinBox *startRow, *endRow, *startCol, *endCol;
	QPushButton *btnApply;

public slots:
	void accept();
	bool apply();
	void setFunctions();
	void addCell();
	void insertFunction();
	void insertExplain(int index);
	void setMatrix(Matrix *m);

private:
	Matrix *matrix;
};

#endif //