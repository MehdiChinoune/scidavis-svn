/***************************************************************************
    File                 : LineEnrichmentDialog.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Line options dialog

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
#ifndef LINE_ENRICHMENT_DIALOG_H
#define LINE_ENRICHMENT_DIALOG_H

#include <qdialog.h>

class QCheckBox;
class QComboBox;
class QPushButton;
class QTabWidget;
class QWidget;
class QSpinBox;
class QLineEdit;
class ColorButton;
class LineEnrichment;

//! Line options dialog
class LineEnrichmentDialog : public QDialog
{
    Q_OBJECT

public:
    LineEnrichmentDialog(LineEnrichment *line, QWidget* parent = 0, Qt::WFlags fl = 0);

    enum Unit{ScaleCoordinates, Pixels};

	void initGeometryTab();
	void enableHeadTab();
	void setCoordinates(int unit);

public slots:
	void enableButtonDefault(QWidget *w);
	void setDefaultValues();
	void displayCoordinates(int unit);
	void setLineStyle(Qt::PenStyle style);
	void pickColor();
	void accept();
	void apply();

private:
	LineEnrichment *lm;

    ColorButton* colorBox;
    QComboBox* styleBox;
    QComboBox* widthBox;
	QComboBox* unitBox;
    QPushButton* btnOk;
    QPushButton* btnApply;
	QPushButton* buttonDefault;
    QCheckBox* endBox;
    QCheckBox* startBox,  *filledBox;
	QTabWidget* tw;
	QWidget *options, *geometry, *head;
	QLineEdit *xStartBox, *yStartBox, *xEndBox, *yEndBox;
	QSpinBox *boxHeadAngle, *boxHeadLength;
};

#endif // ifndef LINE_ENRICHMENT_DIALOG_H
