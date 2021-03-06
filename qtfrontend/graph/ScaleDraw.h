/***************************************************************************
    File                 : ScaleDraw.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Extension to QwtScaleDraw

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
#ifndef SCALES_H
#define SCALES_H

#include <QDateTime>
#include <QStringList>
#include <QLocale>

#include <qwt_scale_draw.h>

//! Extension to QwtScaleDraw
class ScaleDraw: public QwtScaleDraw
{
public:
	enum TicksStyle{None = 0, Out = 1, Both = 2, In = 3};

	ScaleDraw(const QString& s = QString::null);
	virtual ~ScaleDraw(){};

	QString formulaString() {return formula_string;};
	void setFormulaString(const QString& formula) {formula_string = formula;};

	double transformValue(double value) const;

	virtual QwtText label(double value) const
	{
	return QwtText(QLocale().toString(transformValue(value), m_fmt, m_prec));
	};

	void labelFormat(char &f, int &prec) const;
	void setLabelFormat(char f, int prec);

	int labelNumericPrecision(){return m_prec;};

	int majorTicksStyle(){return m_majTicks;};
	void setMajorTicksStyle(TicksStyle type){m_majTicks = type;};

	int minorTicksStyle(){return m_minTicks;};
	void setMinorTicksStyle(TicksStyle type){m_minTicks = type;};

protected:
	void drawTick(QPainter *p, double value, int len) const;

private:
	QString formula_string;
	char m_fmt;
    int m_prec;
	int m_minTicks, m_majTicks;
};

class QwtTextScaleDraw: public ScaleDraw
{
public:
	QwtTextScaleDraw(const QStringList& list);
	~QwtTextScaleDraw(){};

	QwtText label(double value) const;

	QStringList labelsList(){return labels;};
private:
	QStringList labels;
};

class TimeScaleDraw: public ScaleDraw
{
public:
	TimeScaleDraw(const QTime& t, const QString& format);
	~TimeScaleDraw(){};

	QString origin();
	QString timeFormat() {return t_format;};

	QwtText label(double value) const;

private:
	QTime t_origin;
	QString t_format;
};

class DateScaleDraw: public ScaleDraw
{
public:
	DateScaleDraw(const QDate& t, const QString& format);
	~DateScaleDraw(){};

	QString origin();

	QString format() {return t_format;};
	QwtText label(double value) const;

private:
	QDate t_origin;
	QString t_format;
};

class WeekDayScaleDraw: public ScaleDraw
{
public:
	enum NameFormat{ShortName, LongName, Initial};

	WeekDayScaleDraw(NameFormat format = ShortName);
	~WeekDayScaleDraw(){};

	NameFormat format() {return m_format;};
	QwtText label(double value) const;

private:
	NameFormat m_format;
};

class MonthScaleDraw: public ScaleDraw
{
public:
	enum NameFormat{ShortName, LongName, Initial};

	MonthScaleDraw(NameFormat format = ShortName);
	~MonthScaleDraw(){};

	NameFormat format() {return m_format;};
	QwtText label(double value) const;

private:
	NameFormat m_format;
};

class QwtSupersciptsScaleDraw: public ScaleDraw
{
public:
	QwtSupersciptsScaleDraw(const QString& s = QString::null);
	~QwtSupersciptsScaleDraw(){};

	QwtText label(double value) const;
};

#endif
