/***************************************************************************
    File                 : InterpolationDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Interpolation options dialog

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
#include "InterpolationDialog.h"
#include "Interpolation.h"
#include "core/MyParser.h"
#include "lib/ColorBox.h"
#include "graph/Layer.h"

#include <QGroupBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QLayout>

InterpolationDialog::InterpolationDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
	setWindowTitle(tr("Interpolation Options"));

    QGroupBox *gb1 = new QGroupBox();
	QGridLayout *gl1 = new QGridLayout(gb1);
	gl1->addWidget(new QLabel(tr("Make curve from")), 0, 0);

	boxName = new QComboBox();
	gl1->addWidget(boxName, 0, 1);

	gl1->addWidget(new QLabel(tr("Spline")), 1, 0);
	boxMethod = new QComboBox();
	boxMethod->insertItem(tr("Linear"));
    boxMethod->insertItem(tr("Cubic"));
    boxMethod->insertItem(tr("Non-rounded Akima"));
	gl1->addWidget(boxMethod, 1, 1);

	gl1->addWidget(new QLabel(tr("Points")), 2, 0);
	boxPoints = new QSpinBox();
	boxPoints->setRange(3,100000);
	boxPoints->setSingleStep(10);
	boxPoints->setValue(1000);
	gl1->addWidget(boxPoints, 2, 1);

	gl1->addWidget(new QLabel(tr("From Xmin")), 3, 0);
	boxStart = new QLineEdit();
	boxStart->setText(tr("0"));
	gl1->addWidget(boxStart, 3, 1);

	gl1->addWidget(new QLabel(tr("To Xmax")), 4, 0);
	boxEnd = new QLineEdit();
	gl1->addWidget(boxEnd, 4, 1);

	gl1->addWidget(new QLabel(tr("Color")), 5, 0);

	boxColor = new ColorBox(false);
	boxColor->setColor(QColor(Qt::red));
	gl1->addWidget(boxColor, 5, 1);
	gl1->setRowStretch(6, 1);

	buttonFit = new QPushButton(tr( "&Make" ));
    buttonFit->setDefault( true );
    buttonCancel = new QPushButton(tr( "&Close" ));

	QVBoxLayout *vl = new QVBoxLayout();
 	vl->addWidget(buttonFit);
	vl->addWidget(buttonCancel);
    vl->addStretch();

    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->addWidget(gb1);
    hb->addLayout(vl);

	connect( boxName, SIGNAL(activated(const QString&)), this, SLOT( activateCurve(const QString&)));
	connect( buttonFit, SIGNAL( clicked() ), this, SLOT( interpolate() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

void InterpolationDialog::interpolate()
{
QString curve = boxName->currentText();
QStringList curvesList = m_layer->analysableCurvesList();
if (!curvesList.contains(curve))
	{
	QMessageBox::critical(this,tr("Warning"),
		tr("The curve <b> %1 </b> doesn't exist anymore! Operation aborted!").arg(curve));
	boxName->clear();
	boxName->addItems(curvesList);
	return;
	}

double from, to;
try
	{
	MyParser parser;
	parser.SetExpr(boxStart->text().replace(",", ".").toAscii().constData());
	from = parser.Eval();
	}
catch(mu::ParserError &e)
	{
	QMessageBox::critical(this, tr("Start limit error"), QString::fromStdString(e.GetMsg()));
	boxStart->setFocus();
	return;
	}

try
	{
	MyParser parser;
	parser.SetExpr(boxEnd->text().replace(",", ".").toAscii().constData());
	to = parser.Eval();
	}
catch(mu::ParserError &e)
	{
	QMessageBox::critical(this, tr("End limit error"), QString::fromStdString(e.GetMsg()));
	boxEnd->setFocus();
	return;
	}

if (from >= to)
	{
	QMessageBox::critical(this, tr("Input error"), tr("Please enter x limits that satisfy: from < to!"));
	boxEnd->setFocus();
	return;
	}

Interpolation *i = new Interpolation((ApplicationWindow *)this->parent(), m_layer, curve,
                                      from, to, boxMethod->currentIndex());
i->setOutputPoints(boxPoints->value());
i->setColor(boxColor->currentIndex());
i->run();
delete i;
}

void InterpolationDialog::setLayer(Layer *layer)
{
	m_layer = layer;
	boxName->addItems(m_layer->analysableCurvesList());

	QString selectedCurve = m_layer->selectedCurveTitle();
	if (!selectedCurve.isEmpty())
	{
		int index = boxName->findText (selectedCurve);
		boxName->setCurrentItem(index);
	}

	activateCurve(boxName->currentText());

	connect (m_layer, SIGNAL(closed()), this, SLOT(close()));
	connect (m_layer, SIGNAL(dataRangeChanged()), this, SLOT(changeDataRange()));
};

void InterpolationDialog::activateCurve(const QString& curveName)
{
	QwtPlotCurve *c = m_layer->curve(curveName);
	if (!c)
		return;

    ApplicationWindow *app = (ApplicationWindow *)parent();
    if(!app)
        return;

	double start, end;
	m_layer->range(m_layer->curveIndex(curveName), &start, &end);
	boxStart->setText(QString::number(QMIN(start, end), 'g', app->m_decimal_digits));
	boxEnd->setText(QString::number(QMAX(start, end), 'g', app->m_decimal_digits));
};

void InterpolationDialog::changeDataRange()
{
ApplicationWindow *app = (ApplicationWindow *)parent();
if(!app)
    return;

double start = m_layer->selectedXStartValue();
double end = m_layer->selectedXEndValue();
boxStart->setText(QString::number(QMIN(start, end), 'g', app->m_decimal_digits));
boxEnd->setText(QString::number(QMAX(start, end), 'g', app->m_decimal_digits));
}
