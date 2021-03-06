/***************************************************************************
    File                 : ErrorCurve.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Error bars curve

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
#include "ErrorCurve.h"
#include "BarCurve.h"
#include "../Layer.h"
#include "table/Table.h"

#include <qwt_painter.h>
#include <qwt_symbol.h>

#include <QPainter>
#include <QLocale>

ErrorCurve::ErrorCurve(int orientation, Table *t, const char *name):
	DataCurve(t, QString(), name),
	d_master_curve(NULL)
{
	cap = 10;
	type = orientation;
	plus = true;
	minus = true;
	through = false;
	setType(Layer::ErrorBars);
}

ErrorCurve::ErrorCurve(Table *t, const char *name):
	DataCurve(t, QString(), name),
	d_master_curve(NULL)
{
	cap=10;
	type = Vertical;
	plus = true;
	minus = true;
	through = false;
	setType(Layer::ErrorBars);
}

void ErrorCurve::copy(const ErrorCurve *e)
{
	cap = e->cap;
	type = e->type;
	plus = e->plus;
	minus = e->minus;
	through = e->through;
	setPen(e->pen());
	err = e->err;
}

void ErrorCurve::draw(QPainter *painter,
		const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
	if ( !painter || dataSize() <= 0 )
		return;

	if (to < 0)
		to = dataSize() - 1;

  painter->save();
  painter->setPen(pen());
  drawErrorBars(painter, xMap, yMap, from, to);
  painter->restore();
}

void ErrorCurve::drawErrorBars(QPainter *painter,
		const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
    int sh = 0, sw =0;
    const QwtSymbol symbol = d_master_curve->symbol();
    if (symbol.style() != QwtSymbol::NoSymbol){
        sh = symbol.size().height();
        sw = symbol.size().width();
    }

	double d_xOffset = 0.0;
	double d_yOffset = 0.0;
	if (d_master_curve->type() == Layer::VerticalBars)
		d_xOffset = ((BarCurve *)d_master_curve)->dataOffset();
	else if (d_master_curve->type() == Layer::HorizontalBars)
		d_yOffset = ((BarCurve *)d_master_curve)->dataOffset();

	for (int i = from; i <= to; i++){
		const int xi = xMap.transform(x(i) + d_xOffset);
		const int yi = yMap.transform(y(i) + d_yOffset);

		if (type == Vertical){
			const int yh = yMap.transform(y(i)+err[i]);
			const int yl = yMap.transform(y(i)-err[i]);
			const int yhl = yi - sh/2;
			const int ylh = yi + sh/2;

			if (plus){
				QwtPainter::drawLine(painter, xi, yhl, xi, yh);
				QwtPainter::drawLine(painter, xi-cap/2, yh, xi+cap/2, yh);
			}
			if (minus){
				QwtPainter::drawLine(painter, xi, ylh, xi, yl);
				QwtPainter::drawLine(painter, xi-cap/2, yl, xi+cap/2, yl);
			}
			if (through)
				QwtPainter::drawLine(painter, xi, yhl, xi, ylh);
		} else if (type == Horizontal) {
			const int xp = xMap.transform(x(i)+err[i]);
			const int xm = xMap.transform(x(i)-err[i]);
  			const int xpm = xi + sw/2;
  	        const int xmp = xi - sw/2;

			if (plus){
				QwtPainter::drawLine(painter, xp, yi, xpm, yi);
				QwtPainter::drawLine(painter, xp, yi-cap/2, xp, yi+cap/2);
			}
			if (minus){
				QwtPainter::drawLine(painter, xm, yi, xmp, yi);
				QwtPainter::drawLine(painter, xm, yi-cap/2, xm, yi+cap/2);
			}
			if (through)
				QwtPainter::drawLine(painter, xmp, yi, xpm, yi);
		}
	}
}

double ErrorCurve::errorValue(int i)
{
	if (i >= 0 && i < dataSize())
		return err[i];
	else
		return 0.0;
}

bool ErrorCurve::xErrors()
{
	bool x = false;
	if (type == Horizontal)
		x = true;

	return x;
}

void ErrorCurve::setXErrors(bool yes)
{
	if (yes)
		type = Horizontal;
	else
		type = Vertical;
}

void ErrorCurve::setWidth(int w)
{
	QPen p = pen();
 	p.setWidth (w);
  	setPen(p);
}

void ErrorCurve::setColor(const QColor& c)
{
	QPen p = pen();
  	p.setColor(c);
  	setPen(p);
}

QwtDoubleRect ErrorCurve::boundingRect() const
{
	QwtDoubleRect rect = QwtPlotCurve::boundingRect();

	int size = dataSize();

	QwtArray <double> X(size), Y(size), min(size), max(size);
	for (int i=0; i<size; i++){
		X[i]=x(i);
		Y[i]=y(i);
		if (type == Vertical){
			min[i] = y(i) - err[i];
			max[i] = y(i) + err[i];
		} else {
			min[i] = x(i) - err[i];
			max[i] = x(i) + err[i];
		}
	}

	QwtArrayData *erMin, *erMax;
	if (type == Vertical) {
		erMin=new QwtArrayData(X, min);
		erMax=new QwtArrayData(X, max);
	} else {
		erMin=new QwtArrayData(min, Y);
		erMax=new QwtArrayData(max, Y);
	}

	QwtDoubleRect minrect = erMin->boundingRect();
	QwtDoubleRect maxrect = erMax->boundingRect();

	rect.setTop(QMIN(minrect.top(), maxrect.top()));
	rect.setBottom(QMAX(minrect.bottom(), maxrect.bottom()));
	rect.setLeft(QMIN(minrect.left(), maxrect.left()));
	rect.setRight(QMAX(minrect.right(), maxrect.right()));

	delete erMin;
	delete erMax;

	return rect;
}

void ErrorCurve::setMasterCurve(DataCurve *c)
{
	if (!c || d_master_curve == c)
		return;

	d_master_curve = c;
	setAxis(c->xAxis(), c->yAxis());
	d_start_row = c->startRow();
	d_end_row = c->endRow();
	c->addErrorBars(this);

	loadData();
}

void ErrorCurve::loadData()
{
	if (!d_master_curve)
		return;

	Table *mt = d_master_curve->table();
	if (!mt)
		return;

	int xcol = mt->colIndex(d_master_curve->xColumnName());
	int ycol = mt->colIndex(d_master_curve->title().text());
	int errcol = d_table->colIndex(title().text());
	if (xcol<0 || ycol<0 || errcol<0)
		return;

	int xColType = mt->columnType(xcol);
	int yColType = mt->columnType(ycol);

	d_start_row = d_master_curve->startRow();
	d_end_row = d_master_curve->endRow();
    int r = abs(d_end_row - d_start_row) + 1;
	QVector<double> X(r), Y(r), err(r);
    int data_size = 0;
	for (int i = d_start_row; i <= d_end_row; i++){
		QString xval = mt->text(i, xcol);
		QString yval = mt->text(i, ycol);
		QString errval = d_table->text(i, errcol);
		if (!xval.isEmpty() && !yval.isEmpty() && !errval.isEmpty()){
		    bool ok = true;
			if (xColType == SciDAVis::Text)
				X[data_size] = (double)(data_size + 1);
			else
				X[data_size] = QLocale().toDouble(xval, &ok);

			if (yColType == SciDAVis::Text)
				Y[data_size] = (double)(data_size + 1);
			else
				Y[data_size] = QLocale().toDouble(yval, &ok);

            if (!ok)
                continue;

			err[data_size] = QLocale().toDouble(errval, &ok);
			if (ok)
                data_size++;
		}
	}

	if (!data_size)
		remove();

    X.resize(data_size);
	Y.resize(data_size);
	err.resize(data_size);

	setData(X.data(), Y.data(), data_size);
	setErrors(err);
}

QString ErrorCurve::plotAssociation()
{
    if (!d_master_curve)
		return QString();

    QString base = d_master_curve->xColumnName() + "(X)," + d_master_curve->title().text() + "(Y)," + title().text();
    if (type == Horizontal)
        return base + "(xErr)";
    else
        return base + "(yErr)";
}

bool ErrorCurve::updateData(Table *t, const QString& colName)
{
	if (d_table != t || colName != title().text())
		return false;

	loadData();
	return true;
}
