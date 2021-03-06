/***************************************************************************
    File                 : Interpolation.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Numerical interpolation of data sets

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
#include "Interpolation.h"

#include <QMessageBox>

#include <qwt_plot_curve.h>

#include <gsl/gsl_sort.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_interp.h>

Interpolation::Interpolation(ApplicationWindow *parent, Layer *layer, const QString& curveTitle, int m)
: Filter(parent, layer)
{
	init(m);
	setDataFromCurve(curveTitle);
}

Interpolation::Interpolation(ApplicationWindow *parent, Layer *layer, const QString& curveTitle,
                             double start, double end, int m)
: Filter(parent, layer)
{
	init(m);
	setDataFromCurve(curveTitle, start, end);
}

void Interpolation::init(int m)
{
    if (m < 0 || m > 2)
    {
        QMessageBox::critical((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
        tr("Unknown interpolation method. Valid values are: 0 - Linear, 1 - Cubic, 2 - Akima."));
        m_init_err = true;
        return;
    }
    m_method = m;
	switch(m_method)
	{
		case 0:
			setName(tr("Linear") + tr("Int"));
			m_explanation = tr("Linear") + " " + tr("Interpolation");
			break;
		case 1:
			setName(tr("Cubic") + tr("Int"));
			m_explanation = tr("Cubic") + " " + tr("Interpolation");
			break;
		case 2:
			setName(tr("Akima") + tr("Int"));
			m_explanation = tr("Akima") + " " + tr("Interpolation");
			break;
	}
    m_sort_data = true;
    m_min_points = m_method + 3;
}


void Interpolation::setMethod(int m)
{
if (m < 0 || m > 2)
    {
        QMessageBox::critical((ApplicationWindow *)parent(), tr("Error"),
        tr("Unknown interpolation method, valid values are: 0 - Linear, 1 - Cubic, 2 - Akima."));
        m_init_err = true;
        return;
    }
int min_points = m + 3;
if (m_n < min_points)
	{
		QMessageBox::critical((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
				tr("You need at least %1 points in order to perform this operation!").arg(min_points));
        m_init_err = true;
        return;
	}
m_method = m;
m_min_points = min_points;
}

void Interpolation::calculateOutputData(double *x, double *y)
{
	gsl_interp_accel *acc = gsl_interp_accel_alloc ();
	const gsl_interp_type *method;
	switch(m_method)
	{
		case 0:
			method = gsl_interp_linear;
			break;
		case 1:
			method = gsl_interp_cspline;
			break;
		case 2:
			method = gsl_interp_akima;
			break;
	}

	gsl_spline *interp = gsl_spline_alloc (method, m_n);
	gsl_spline_init (interp, m_x, m_y, m_n);

    double step = (m_to - m_from)/(double)(m_points - 1);
    for (int j = 0; j < m_points; j++)
	{
	   x[j] = m_from + j*step;
	   y[j] = gsl_spline_eval (interp, x[j], acc);
	}

	gsl_spline_free (interp);
	gsl_interp_accel_free (acc);
}

int Interpolation::sortedCurveData(QwtPlotCurve *c, double start, double end, double **x, double **y)
{
    if (!c || c->rtti() != QwtPlotItem::Rtti_PlotCurve)
        return 0;

    int i_start = 0, i_end = c->dataSize();
    for (int i = 0; i < i_end; i++)
  	    if (c->x(i) > start && i)
        {
  	      i_start = i - 1;
          break;
        }
    for (int i = i_end-1; i >= 0; i--)
  	    if (c->x(i) < end && i < c->dataSize())
        {
  	      i_end = i + 1;
          break;
        }
    int n = i_end - i_start + 1;
    (*x) = new double[n];
    (*y) = new double[n];
    double *xtemp = new double[n];
    double *ytemp = new double[n];

	double pr_x;
  	int j=0;
    for (int i = i_start; i <= i_end; i++)
    {
        xtemp[j] = c->x(i);
        if (xtemp[j] == pr_x)
        {
            delete (*x);
            delete (*y);
            return -1;//this kind of data causes division by zero in GSL interpolation routines
        }
        pr_x = xtemp[j];
        ytemp[j++] = c->y(i);
    }
    size_t *p = new size_t[n];
    gsl_sort_index(p, xtemp, 1, n);
    for (int i=0; i<n; i++)
    {
        (*x)[i] = xtemp[p[i]];
  	    (*y)[i] = ytemp[p[i]];
    }
    delete[] xtemp;
    delete[] ytemp;
    delete[] p;
    return n;
}
