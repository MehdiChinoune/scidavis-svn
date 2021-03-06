/***************************************************************************
    File                 : Correlation.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Numerical correlation of data sets

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
#include "Correlation.h"
#include "graph/Graph.h"
#include "graph/Plot.h"
#include "graph/PlotCurve.h"
#include "lib/ColorBox.h"
#include "table/Table.h"

#include <QMessageBox>
#include <QLocale>

#include <gsl/gsl_fft_halfcomplex.h>

Correlation::Correlation(ApplicationWindow *parent, Table *t, const QString& colName1, const QString& colName2)
: Filter(parent, t)
{
	setName(tr("Correlation"));
    setDataFromTable(t, colName1, colName2);
}

void Correlation::setDataFromTable(Table *t, const QString& colName1, const QString& colName2)
{
    if (t && m_table != t)
        m_table = t;

    int col1 = m_table->colIndex(colName1);
	int col2 = m_table->colIndex(colName2);

	if (col1 < 0)
	{
		QMessageBox::warning((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
		tr("The data set %1 does not exist!").arg(colName1));
		m_init_err = true;
		return;
	}
	else if (col2 < 0)
	{
		QMessageBox::warning((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
		tr("The data set %1 does not exist!").arg(colName2));
		m_init_err = true;
		return;
	}

    if (m_n > 0)
	{//delete previousely allocated memory
		delete[] m_x;
		delete[] m_y;
	}

	int rows = m_table->rowCount();
	m_n = 16; // tmp number of points
	while (m_n < rows)
		m_n *= 2;

    m_x = new double[m_n];
	m_y = new double[m_n];

    if(m_y && m_x)
	{
		memset( m_x, 0, m_n * sizeof( double ) ); // zero-pad the two arrays...
		memset( m_y, 0, m_n * sizeof( double ) );
		for(int i=0; i<rows; i++)
		{
			m_x[i] = m_table->cell(i, col1);
			m_y[i] = m_table->cell(i, col2);
		}
	}
	else
	{
		QMessageBox::critical((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
                        tr("Could not allocate memory, operation aborted!"));
        m_init_err = true;
		m_n = 0;
	}
}

void Correlation::output()
{
    // calculate the FFTs of the two functions
	if( gsl_fft_real_radix2_transform( m_x, 1, m_n ) == 0 &&
        gsl_fft_real_radix2_transform( m_y, 1, m_n ) == 0)
	{
		// multiply the FFT by its complex conjugate
		for(int i=0; i<m_n/2; i++ )
		{
			if( i==0 || i==(m_n/2)-1 )
				m_x[i] *= m_x[i];
			else
			{
				int ni = m_n-i;
				double dReal = m_x[i] * m_y[i] + m_x[ni] * m_y[ni];
				double dImag = m_x[i] * m_y[ni] - m_x[ni] * m_y[i];
				m_x[i] = dReal;
				m_x[ni] = dImag;
			}
		}
	}
	else
	{
		QMessageBox::warning((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
                             tr("Error in GSL forward FFT operation!"));
		return;
	}

	gsl_fft_halfcomplex_radix2_inverse(m_x, 1, m_n );	//inverse FFT

	addResultCurve();
}

void Correlation::addResultCurve()
{
    ApplicationWindow *app = (ApplicationWindow *)parent();
    if (!app)
        return;

	int rows = m_table->rowCount();
	int cols = m_table->columnCount();
	int cols2 = cols+1;
	m_table->addCol();
	m_table->addCol();
	int n = rows/2;

	double x_temp[rows], y_temp[rows];
	for (int i = 0; i<rows; i++)
	{
	    double x = i - n;
        x_temp[i] = x;

        double y;
        if(i < n)
			y_temp[i] = m_x[m_n - n + i];
		else
			y_temp[i] = m_x[i-n];

		m_table->setText(i, cols, QString::number(x));
		m_table->setText(i, cols2, QLocale().toString(y, 'g', app->m_decimal_digits));
	}

	QStringList l = m_table->colNames().grep(tr("Lag"));
	QString id = QString::number((int)l.size()+1);
	QString label = name() + id;

	m_table->setColName(cols, tr("Lag") + id);
	m_table->setColName(cols2, label);
	m_table->setColPlotDesignation(cols, SciDAVis::X);
	// TODO
	//m_table->setHeaderColType();

	Graph *graph = app->newGraph(name() + tr("Plot"));
	if (!graph)
        return;

	DataCurve *c = new DataCurve(m_table, m_table->colName(cols), m_table->colName(cols2));
	c->setData(x_temp, y_temp, rows);
	c->setPen(QPen(ColorBox::color(m_curveColorIndex), 1));
	graph->activeLayer()->insertPlotItem(c, Layer::Line);
	graph->activeLayer()->updatePlot();
}
