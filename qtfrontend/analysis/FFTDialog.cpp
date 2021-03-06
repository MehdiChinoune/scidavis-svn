/***************************************************************************
    File                 : FFTDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Fast Fourier transform options dialog

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
#include "FFTDialog.h"
#include "FFT.h"
#include "core/MyParser.h"
#include "core/ApplicationWindow.h"
#include "graph/Layer.h"
#include "graph/Graph.h"
#include "graph/Plot.h"
#include "table/Table.h"

#include <QRadioButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QLayout>

FFTDialog::FFTDialog(int type, QWidget* parent, Qt::WFlags fl )
: QDialog( parent, fl )
{
	setWindowTitle(tr("FFT Options"));

	m_table = 0;
	m_layer = 0;
	m_type = type;

	forwardBtn = new QRadioButton(tr("&Forward"));
	forwardBtn->setChecked( true );
	backwardBtn = new QRadioButton(tr("&Inverse"));

	QHBoxLayout *hbox1 = new QHBoxLayout();
    hbox1->addWidget(forwardBtn);
    hbox1->addWidget(backwardBtn);

	QGroupBox *gb1 = new QGroupBox();
    gb1->setLayout(hbox1);

	QGridLayout *gl1 = new QGridLayout();
	if (m_type == onGraph)
	    gl1->addWidget(new QLabel(tr("Curve")), 0, 0);
	else
		gl1->addWidget(new QLabel(tr("Sampling")), 0, 0);

	boxName = new QComboBox();
	gl1->addWidget(boxName, 0, 1);

    boxSampling = new QLineEdit();
	if (m_type == onTable)
       {
		gl1->addWidget(new QLabel(tr("Real")), 1, 0);
		boxReal = new QComboBox();
		gl1->addWidget(boxReal, 1, 1);

		gl1->addWidget(new QLabel(tr("Imaginary")), 2, 0);
		boxImaginary = new QComboBox();
		gl1->addWidget(boxImaginary, 2, 1);

		gl1->addWidget(new QLabel(tr("Sampling Interval")), 3, 0);
		gl1->addWidget(boxSampling, 3, 1);
	   }
    else
       {
        gl1->addWidget(new QLabel(tr("Sampling Interval")), 1, 0);
		gl1->addWidget(boxSampling, 1, 1);
       }
 	QGroupBox *gb2 = new QGroupBox();
    gb2->setLayout(gl1);

	boxNormalize = new QCheckBox(tr( "&Normalize Amplitude" ));
	boxNormalize->setChecked(true);

    boxOrder = new QCheckBox(tr( "&Shift Results" ));
	boxOrder->setChecked(true);

    QVBoxLayout *vbox1 = new QVBoxLayout();
    vbox1->addWidget(gb1);
    vbox1->addWidget(gb2);
    vbox1->addWidget(boxNormalize);
    vbox1->addWidget(boxOrder);
	vbox1->addStretch();

    buttonOK = new QPushButton(tr("&OK"));
	buttonOK->setDefault( true );
	buttonCancel = new QPushButton(tr("&Close"));

	QVBoxLayout *vbox2 = new QVBoxLayout();
    vbox2->addWidget(buttonOK);
    vbox2->addWidget(buttonCancel);
    vbox2->addStretch();

    QHBoxLayout *hbox2 = new QHBoxLayout(this);
    hbox2->addLayout(vbox1);
    hbox2->addLayout(vbox2);

	setFocusProxy(boxName);

	// signals and slots connections
	connect( boxName, SIGNAL( activated(const QString&) ), this, SLOT( activateCurve(const QString&) ) );
	connect( buttonOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

void FFTDialog::accept()
{
	double sampling;
	try
	{
		MyParser parser;
		parser.SetExpr(boxSampling->text().toAscii().constData());
		sampling=parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(this, tr("Sampling value error"), QString::fromStdString(e.GetMsg()));
		boxSampling->setFocus();
		return;
	}

	ApplicationWindow *app = (ApplicationWindow *)parent();
	FFT *fft = 0;
	if (m_layer)
	{
        fft = new FFT(app, m_layer, boxName->currentText());
	} else if (m_table) {
		if (boxReal->currentText().isEmpty())
		{
			QMessageBox::critical(this, tr("Error"), tr("Please choose a column for the real part of the data!"));
			boxReal->setFocus();
			return;
		}
        fft = new FFT(app, m_table, boxReal->currentText(), boxImaginary->currentText());
	}
	if (!fft) {
		close();
		return;
	}
	fft->setInverseFFT(backwardBtn->isChecked());
	fft->setSampling(sampling);
	fft->normalizeAmplitudes(boxNormalize->isChecked());
	fft->shiftFrequencies(boxOrder->isChecked());
	fft->run();
	delete fft;
	close();
}

void FFTDialog::setLayer(Layer *layer)
{
	m_layer = layer;
	boxName->insertStringList (layer->analysableCurvesList());
	activateCurve(boxName->currentText());
};

void FFTDialog::activateCurve(const QString& curveName)
{
	if (m_layer)
	{
		QwtPlotCurve *c = m_layer->curve(curveName);
		if (!c)
			return;

		boxSampling->setText(QString::number(c->x(1) - c->x(0)));
	}
	else if (m_table)
	{
	    int col = m_table->colIndex(curveName);
		double x0 = m_table->text(0, col).toDouble();
		double x1 = m_table->text(1, col).toDouble();
		boxSampling->setText(QString::number(x1 - x0));
	}
};

void FFTDialog::setTable(Table *t)
{
	m_table = t;
	QStringList l = t->columnsList();
	boxName->insertStringList (l);
	boxReal->insertStringList (l);
	boxImaginary->insertStringList (l);

	int xcol = t->firstXCol();
	if (xcol >= 0)
	{
		boxName->setCurrentItem(xcol);

		double x0 = t->text(0, xcol).toDouble();
		double x1 = t->text(1, xcol).toDouble();
		boxSampling->setText(QString::number(x1 - x0));
	}

	l = t->selectedColumnsOld();
	int selected = (int)l.size();
	if (!selected)
	{
		boxReal->setCurrentText(QString());
		boxImaginary->setCurrentText(QString());
	}
	else if (selected == 1)
	{
		boxReal->setCurrentItem(t->colIndex(l[0]));
		boxImaginary->setCurrentText(QString());
	}
	else
	{
		boxReal->setCurrentItem(t->colIndex(l[0]));
		boxImaginary->setCurrentItem(t->colIndex(l[1]));
	}
};
