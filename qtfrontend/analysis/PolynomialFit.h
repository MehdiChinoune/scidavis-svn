/***************************************************************************
    File                 : PolynomialFit.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Polynomial Fit and Linear Fit classes

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
#ifndef POLYNOMIALFIT_H
#define POLYNOMIALFIT_H

#include "core/Fit.h"

class PolynomialFit : public Fit
{
	Q_OBJECT

	public:
		PolynomialFit(ApplicationWindow *parent, Layer *layer, int order = 2, bool legend = false);
		PolynomialFit(ApplicationWindow *parent, Layer *layer, QString& curveTitle, int order = 2, bool legend = false);
		PolynomialFit(ApplicationWindow *parent, Layer *layer, QString& curveTitle, double start, double end, int order = 2, bool legend = false);

		virtual QString legendInfo();
		void fit();

		static QString generateFormula(int order);
		static QStringList generateParameterList(int order);

	private:
		void init();
		void calculateFitCurveData(double *par, double *X, double *Y);

		int m_order;
		bool show_legend;
};

class LinearFit : public Fit
{
	Q_OBJECT

	public:
		LinearFit(ApplicationWindow *parent, Layer *layer);
		LinearFit(ApplicationWindow *parent, Layer *layer, const QString& curveTitle);
		LinearFit(ApplicationWindow *parent, Layer *layer, const QString& curveTitle, double start, double end);

		void fit();

	private:
		void init();
		void calculateFitCurveData(double *par, double *X, double *Y);
};
#endif

