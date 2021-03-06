/***************************************************************************
    File                 : Layer.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Layer widget

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

#include <QVarLengthArray>

#include "Layer.h"
#include "CanvasPicker.h"
#include "types/ErrorCurve.h"
#include "TextEnrichment.h"
#include "enrichments/LineEnrichment.h"
#include "ScalePicker.h"
#include "TitlePicker.h"
#include "types/PieCurve.h"
#include "enrichments/ImageEnrichment.h"
#include "types/BarCurve.h"
#include "types/BoxCurve.h"
#include "types/HistogramCurve.h"
#include "types/VectorCurve.h"
#include "ScaleDraw.h"
#include "lib/ColorBox.h"
#include "lib/PatternBox.h"
#include "SymbolBox.h"
#include "FunctionCurve.h"
#include "types/Spectrogram.h"
#include "SelectionMoveResizer.h"
#include "tools/RangeSelectorTool.h"
#include "PlotCurve.h"
#include "core/ApplicationWindow.h"
#include "core/AbstractDataSource.h"
#include "table/Table.h"

#include <QApplication>
#include <QBitmap>
#include <QClipboard>
#include <QCursor>
#include <QImage>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>
#include <QMenu>
#include <QTextStream>
#include <QLocale>
#include <QPrintDialog>
#include <QImageWriter>
#include <QFileInfo>

#if QT_VERSION >= 0x040300
	#include <QSvgGenerator>
#endif

#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <qwt_text.h>
#include <qwt_text_label.h>
#include <qwt_color_map.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

Layer::Layer(QWidget* parent, const char* name, Qt::WFlags f)
: QWidget(parent,f)
{
	if ( !name )
		setObjectName( "graph" );
	else
		setObjectName( name );

	n_curves=0;
	m_active_tool = NULL;
	widthLine=1;mrkX=-1;mrkY=-1;;
	selectedMarker=-1;
	drawTextOn=false;
	drawLineOn=false;
	drawArrowOn=false;
	ignoreResize = true;
	drawAxesBackbone = true;
	autoscale = true;
	autoScaleFonts = false;
	m_antialiasing = true;
	m_scale_on_print = true;
	m_print_cropmarks = false;

	defaultArrowLineWidth = 1;
	defaultArrowColor = QColor(Qt::black);
	defaultArrowLineStyle = Qt::SolidLine;
	defaultArrowHeadLength = 4;
	defaultArrowHeadAngle = 45;
	defaultArrowHeadFill = true;

	defaultMarkerFont = QFont();
	defaultMarkerFrame = 1;
	defaultTextMarkerColor = QColor(Qt::black);
	defaultTextMarkerBackground = QColor(Qt::white);

	m_user_step = QVector<double>(QwtPlot::axisCnt);
	for (int i=0; i<QwtPlot::axisCnt; i++)
	{
		axisType << Numeric;
		axesFormatInfo << QString::null;
		axesFormulas << QString::null;
		m_user_step[i] = 0.0;
	}

	m_plot = new Plot(this);
	cp = new CanvasPicker(this);

	titlePicker = new TitlePicker(m_plot);
	scalePicker = new ScalePicker(m_plot);

	m_zoomer[0]= new QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft,
			QwtPicker::DragSelection | QwtPicker::CornerToCorner, QwtPicker::AlwaysOff, m_plot->canvas());
	m_zoomer[0]->setRubberBandPen(QPen(Qt::black));
	m_zoomer[1] = new QwtPlotZoomer(QwtPlot::xTop, QwtPlot::yRight,
			QwtPicker::DragSelection | QwtPicker::CornerToCorner,
			QwtPicker::AlwaysOff, m_plot->canvas());
	zoom(false);

	setGeometry(0, 0, 500, 400);
	setFocusPolicy(Qt::StrongFocus);
	setFocusProxy(m_plot);
	setMouseTracking(true );

	grid.majorOnX=0;
	grid.majorOnY=0;
	grid.minorOnX=0;
	grid.minorOnY=0;
	grid.majorCol=3;
	grid.majorStyle=0;
	grid.majorWidth=1;
	grid.minorCol=18;
	grid.minorStyle=2;
	grid.minorWidth=1;
	grid.xZeroOn=0;
	grid.yZeroOn=0;
	setGridOptions(grid);
	grid.xAxis = QwtPlot::xBottom;
	grid.yAxis = QwtPlot::yLeft;

	legendMarkerID = -1; // no legend for an empty graph
	m_texts = QVector<int>();
	c_type = QVector<int>();
	c_keys = QVector<int>();

	connect (cp,SIGNAL(selectPlot()),this,SLOT(activate()));
	connect (cp,SIGNAL(drawTextOff()),this,SIGNAL(drawTextOff()));
	connect (cp,SIGNAL(viewImageDialog()),this,SIGNAL(viewImageDialog()));
	connect (cp,SIGNAL(viewTextDialog()),this,SIGNAL(viewTextDialog()));
	connect (cp,SIGNAL(viewLineDialog()),this,SIGNAL(viewLineDialog()));
	connect (cp,SIGNAL(showPlotDialog(int)),this,SIGNAL(showPlotDialog(int)));
	connect (cp,SIGNAL(showMarkerPopupMenu()),this,SIGNAL(showMarkerPopupMenu()));
	connect (cp,SIGNAL(modified()), this, SIGNAL(modified()));

	connect (titlePicker,SIGNAL(showTitleMenu()),this,SLOT(showTitleContextMenu()));
	connect (titlePicker,SIGNAL(doubleClicked()),this,SIGNAL(viewTitleDialog()));
	connect (titlePicker,SIGNAL(removeTitle()),this,SLOT(removeTitle()));
	connect (titlePicker,SIGNAL(clicked()), this,SLOT(selectTitle()));

	connect (scalePicker,SIGNAL(clicked()),this,SLOT(activate()));
	connect (scalePicker,SIGNAL(clicked()),this,SLOT(deselectMarker()));
	connect (scalePicker,SIGNAL(axisDblClicked(int)),this,SIGNAL(axisDblClicked(int)));
	connect (scalePicker,SIGNAL(axisTitleRightClicked(int)),this,SLOT(showAxisTitleMenu(int)));
	connect (scalePicker,SIGNAL(axisRightClicked(int)),this,SLOT(showAxisContextMenu(int)));
	connect (scalePicker,SIGNAL(xAxisTitleDblClicked()),this,SIGNAL(xAxisTitleDblClicked()));
	connect (scalePicker,SIGNAL(yAxisTitleDblClicked()),this,SIGNAL(yAxisTitleDblClicked()));
	connect (scalePicker,SIGNAL(rightAxisTitleDblClicked()),this,SIGNAL(rightAxisTitleDblClicked()));
	connect (scalePicker,SIGNAL(topAxisTitleDblClicked()),this,SIGNAL(topAxisTitleDblClicked()));

	connect (m_zoomer[0],SIGNAL(zoomed (const QwtDoubleRect &)),this,SLOT(zoomed (const QwtDoubleRect &)));
}

void Layer::notifyChanges()
{
	emit modified();
}

void Layer::activate()
{
	emit selected(this);
	setFocus();
}

void Layer::deselectMarker()
{
	selectedMarker = -1;
	if (m_markers_selector)
		delete m_markers_selector;
}

long Layer::selectedMarkerKey()
{
	return selectedMarker;
}

QwtPlotMarker* Layer::selectedMarkerPtr()
{
	return m_plot->marker(selectedMarker);
}

void Layer::setSelectedMarker(long mrk, bool add)
{
	selectedMarker = mrk;
	if (add) {
		if (m_markers_selector) {
			if (m_texts.contains(mrk))
				m_markers_selector->add((TextEnrichment*)m_plot->marker(mrk));
			else if (m_lines.contains(mrk))
				m_markers_selector->add((LineEnrichment*)m_plot->marker(mrk));
			else if (m_images.contains(mrk))
				m_markers_selector->add((ImageEnrichment*)m_plot->marker(mrk));
		} else {
			if (m_texts.contains(mrk))
				m_markers_selector = new SelectionMoveResizer((TextEnrichment*)m_plot->marker(mrk));
			else if (m_lines.contains(mrk))
				m_markers_selector = new SelectionMoveResizer((LineEnrichment*)m_plot->marker(mrk));
			else if (m_images.contains(mrk))
				m_markers_selector = new SelectionMoveResizer((ImageEnrichment*)m_plot->marker(mrk));
			else
				return;
			connect(m_markers_selector, SIGNAL(targetsChanged()), this, SIGNAL(modified()));
		}
	} else {
		if (m_texts.contains(mrk)) {
			if (m_markers_selector) {
				if (m_markers_selector->contains((TextEnrichment*)m_plot->marker(mrk)))
					return;
				delete m_markers_selector;
			}
			m_markers_selector = new SelectionMoveResizer((TextEnrichment*)m_plot->marker(mrk));
		} else if (m_lines.contains(mrk)) {
			if (m_markers_selector) {
				if (m_markers_selector->contains((LineEnrichment*)m_plot->marker(mrk)))
					return;
				delete m_markers_selector;
			}
			m_markers_selector = new SelectionMoveResizer((LineEnrichment*)m_plot->marker(mrk));
		} else if (m_images.contains(mrk)) {
			if (m_markers_selector) {
				if (m_markers_selector->contains((ImageEnrichment*)m_plot->marker(mrk)))
					return;
				delete m_markers_selector;
			}
			m_markers_selector = new SelectionMoveResizer((ImageEnrichment*)m_plot->marker(mrk));
		} else
			return;
		connect(m_markers_selector, SIGNAL(targetsChanged()), this, SIGNAL(modified()));
	}
}

void Layer::initFonts(const QFont &scaleTitleFnt, const QFont &numbersFnt)
{
	for (int i = 0;i<QwtPlot::axisCnt;i++)
	{
		m_plot->setAxisFont (i,numbersFnt);
		QwtText t = m_plot->axisTitle (i);
		t.setFont (scaleTitleFnt);
		m_plot->setAxisTitle(i, t);
	}
}

void Layer::setAxisFont(int axis,const QFont &fnt)
{
	m_plot->setAxisFont (axis, fnt);
	m_plot->replot();
	emit modified();
}

QFont Layer::axisFont(int axis)
{
	return m_plot->axisFont (axis);
}

void Layer::enableAxis(int axis, bool on)
{
	m_plot->enableAxis(axis, on);
	QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(axis);
	if (scale)
		scale->setMargin(0);

	scalePicker->refresh();
}

void Layer::enableAxes(const QStringList& list)
{
	int i;
	for (i = 0;i<QwtPlot::axisCnt;i++)
	{
		bool ok=list[i+1].toInt();
		m_plot->enableAxis(i,ok);
	}

	for (i = 0;i<QwtPlot::axisCnt;i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
		if (scale)
			scale->setMargin(0);
	}
	scalePicker->refresh();
}

void Layer::enableAxes(QVector<bool> axesOn)
{
	for (int i = 0; i<QwtPlot::axisCnt; i++)
		m_plot->enableAxis(i, axesOn[i]);

	for (int i = 0;i<QwtPlot::axisCnt;i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
		if (scale)
			scale->setMargin(0);
	}
	scalePicker->refresh();
}

QVector<bool> Layer::enabledAxes()
{
	QVector<bool> axesOn(4);
	for (int i = 0; i<QwtPlot::axisCnt; i++)
		axesOn[i]=m_plot->axisEnabled (i);
	return axesOn;
}

QList<int> Layer::axesBaseline()
{
	QList<int> baselineDist;
	for (int i = 0; i<QwtPlot::axisCnt; i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
		if (scale)
			baselineDist << scale->margin();
		else
			baselineDist << 0;
	}
	return baselineDist;
}

void Layer::setAxesBaseline(const QList<int> &lst)
{
	for (int i = 0; i<QwtPlot::axisCnt; i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
		if (scale)
			scale->setMargin(lst[i]);
	}
}

void Layer::setAxesBaseline(QStringList &lst)
{
	lst.remove(lst.first());
	for (int i = 0; i<QwtPlot::axisCnt; i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
		if (scale)
			scale->setMargin((lst[i]).toInt());
	}
}

QList<int> Layer::axesType()
{
	return axisType;
}

void Layer::setLabelsNumericFormat(int axis, int format, int prec, const QString& formula)
{
	axisType[axis] = Numeric;
	axesFormulas[axis] = formula;

	ScaleDraw *sd_old = (ScaleDraw *)m_plot->axisScaleDraw (axis);
	const QwtScaleDiv div = sd_old->scaleDiv ();

	if (format == Plot::Superscripts){
		QwtSupersciptsScaleDraw *sd = new QwtSupersciptsScaleDraw(formula.toAscii().constData());
		sd->setLabelFormat('s', prec);
		sd->setScaleDiv(div);
		m_plot->setAxisScaleDraw (axis, sd);
	} else {
		ScaleDraw *sd = new ScaleDraw(formula.toAscii().constData());
		sd->setScaleDiv(div);

		if (format == Plot::Automatic)
			sd->setLabelFormat ('g', prec);
		else if (format == Plot::Scientific )
			sd->setLabelFormat ('e', prec);
		else if (format == Plot::Decimal)
			sd->setLabelFormat ('f', prec);

		m_plot->setAxisScaleDraw (axis, sd);
	}
}

void Layer::setLabelsNumericFormat(int axis, const QStringList& l)
{
	QwtScaleDraw *sd = m_plot->axisScaleDraw (axis);
	if (!sd->hasComponent(QwtAbstractScaleDraw::Labels) ||
			axisType[axis] != Numeric)	return;

	int format=l[2*axis].toInt();
	int prec=l[2*axis+1].toInt();
	setLabelsNumericFormat(axis, format, prec, axesFormulas[axis]);
}

void Layer::setLabelsNumericFormat(const QStringList& l)
{
	for (int axis = 0; axis<4; axis++)
		setLabelsNumericFormat (axis, l);
}

void Layer::setAxesType(const QList<int> tl)
{
	axisType = tl;
}

QString Layer::saveAxesLabelsType()
{
	QString s="AxisType\t";
	for (int i=0; i<4; i++)
	{
		int type = axisType[i];
		s+=QString::number(type);
		if (type == Time || type == Date || type == Txt ||
				type == ColHeader || type == Day || type == Month)
			s += ";" + axesFormatInfo[i];
		s+="\t";
	};

	return s+"\n";
}

QString Layer::saveTicksType()
{
	QList<int> ticksTypeList=m_plot->getMajorTicksType();
	QString s="MajorTicks\t";
	int i;
	for (i=0; i<4; i++)
		s+=QString::number(ticksTypeList[i])+"\t";
	s += "\n";

	ticksTypeList=m_plot->getMinorTicksType();
	s += "MinorTicks\t";
	for (i=0; i<4; i++)
		s+=QString::number(ticksTypeList[i])+"\t";

	return s+"\n";
}

QStringList Layer::enabledTickLabels()
{
	QStringList lst;
	for (int axis=0; axis<QwtPlot::axisCnt; axis++)
	{
		const QwtScaleDraw *sd = m_plot->axisScaleDraw (axis);
		lst << QString::number(sd->hasComponent(QwtAbstractScaleDraw::Labels));
	}
	return lst;
}

QString Layer::saveEnabledTickLabels()
{
	QString s="EnabledTickLabels\t";
	for (int axis=0; axis<QwtPlot::axisCnt; axis++)
	{
		const QwtScaleDraw *sd = m_plot->axisScaleDraw (axis);
		s += QString::number(sd->hasComponent(QwtAbstractScaleDraw::Labels))+"\t";
	}
	return s+"\n";
}

QString Layer::saveLabelsFormat()
{
	QString s="LabelsFormat\t";
	for (int axis=0; axis<QwtPlot::axisCnt; axis++)
	{
		s += QString::number(m_plot->axisLabelFormat(axis))+"\t";
		s += QString::number(m_plot->axisLabelPrecision(axis))+"\t";
	}
	return s+"\n";
}

QString Layer::saveAxesBaseline()
{
	QString s="AxesBaseline\t";
	for (int i = 0; i<QwtPlot::axisCnt; i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
		if (scale)
			s+= QString::number(scale->margin()) + "\t";
		else
			s+= "0\t";
	}
	return s+"\n";
}

QString Layer::saveLabelsRotation()
{
	QString s="LabelsRotation\t";
	s+=QString::number(labelsRotation(QwtPlot::xBottom))+"\t";
	s+=QString::number(labelsRotation(QwtPlot::xTop))+"\n";
	return s;
}

void Layer::setEnabledTickLabels(const QStringList& labelsOn)
{
	for (int axis=0; axis<QwtPlot::axisCnt; axis++)
	{
		QwtScaleWidget *sc = m_plot->axisWidget(axis);
		if (sc)
		{
			QwtScaleDraw *sd = m_plot->axisScaleDraw (axis);
			sd->enableComponent (QwtAbstractScaleDraw::Labels, labelsOn[axis] == "1");
		}
	}
}

void Layer::setMajorTicksType(const QList<int>& lst)
{
	if (m_plot->getMajorTicksType() == lst)
		return;

	for (int i=0;i<(int)lst.count();i++)
	{
		ScaleDraw *sd = (ScaleDraw *)m_plot->axisScaleDraw (i);
		if (lst[i]==ScaleDraw::None || lst[i]==ScaleDraw::In)
			sd->enableComponent (QwtAbstractScaleDraw::Ticks, false);
		else
		{
			sd->enableComponent (QwtAbstractScaleDraw::Ticks);
			sd->setTickLength  	(QwtScaleDiv::MinorTick, m_plot->minorTickLength());
			sd->setTickLength  	(QwtScaleDiv::MediumTick, m_plot->minorTickLength());
			sd->setTickLength  	(QwtScaleDiv::MajorTick, m_plot->majorTickLength());
		}
		sd->setMajorTicksStyle((ScaleDraw::TicksStyle)lst[i]);
	}
}

void Layer::setMajorTicksType(const QStringList& lst)
{
	for (int i=0; i<(int)lst.count(); i++)
		m_plot->setMajorTicksType(i, lst[i].toInt());
}

void Layer::setMinorTicksType(const QList<int>& lst)
{
	if (m_plot->getMinorTicksType() == lst)
		return;

	for (int i=0;i<(int)lst.count();i++)
		m_plot->setMinorTicksType(i, lst[i]);
}

void Layer::setMinorTicksType(const QStringList& lst)
{
	for (int i=0;i<(int)lst.count();i++)
		m_plot->setMinorTicksType(i,lst[i].toInt());
}

int Layer::minorTickLength()
{
	return m_plot->minorTickLength();
}

int Layer::majorTickLength()
{
	return m_plot->majorTickLength();
}

void Layer::setAxisTicksLength(int axis, int majTicksType, int minTicksType,
		int minLength, int majLength)
{
	QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(axis);
	if (!scale)
		return;

	m_plot->setTickLength(minLength, majLength);

	ScaleDraw *sd = (ScaleDraw *)m_plot->axisScaleDraw (axis);
	sd->setMajorTicksStyle((ScaleDraw::TicksStyle)majTicksType);
	sd->setMinorTicksStyle((ScaleDraw::TicksStyle)minTicksType);

	if (majTicksType == ScaleDraw::None && minTicksType == ScaleDraw::None)
		sd->enableComponent (QwtAbstractScaleDraw::Ticks, false);
	else
		sd->enableComponent (QwtAbstractScaleDraw::Ticks);

	if (majTicksType == ScaleDraw::None || majTicksType == ScaleDraw::In)
		majLength = minLength;
	if (minTicksType == ScaleDraw::None || minTicksType == ScaleDraw::In)
		minLength = 0;

	sd->setTickLength (QwtScaleDiv::MinorTick, minLength);
	sd->setTickLength (QwtScaleDiv::MediumTick, minLength);
	sd->setTickLength (QwtScaleDiv::MajorTick, majLength);
}

void Layer::setTicksLength(int minLength, int majLength)
{
	QList<int> majTicksType = m_plot->getMajorTicksType();
	QList<int> minTicksType = m_plot->getMinorTicksType();

	for (int i=0; i<4; i++)
		setAxisTicksLength (i, majTicksType[i], minTicksType[i], minLength, majLength);
}

void Layer::changeTicksLength(int minLength, int majLength)
{
	if (m_plot->minorTickLength() == minLength &&
			m_plot->majorTickLength() == majLength)
		return;

	setTicksLength(minLength, majLength);

	m_plot->hide();
	for (int i=0; i<4; i++)
	{
		if (m_plot->axisEnabled(i))
		{
			m_plot->enableAxis (i,false);
			m_plot->enableAxis (i,true);
		}
	}
	m_plot->replot();
	m_plot->show();

	emit modified();
}

void Layer::showAxis(int axis, int type, const QString& formatInfo, Table *table,
		bool axisOn, int majTicksType, int minTicksType, bool labelsOn,
		const QColor& c,  int format, int prec, int rotation, int baselineDist,
		const QString& formula, const QColor& labelsColor)
{
	m_plot->enableAxis(axis, axisOn);
	if (!axisOn)
		return;

	QList<int> majTicksTypeList = m_plot->getMajorTicksType();
	QList<int> minTicksTypeList = m_plot->getMinorTicksType();

	QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(axis);
	ScaleDraw *sclDraw = (ScaleDraw *)m_plot->axisScaleDraw (axis);

	if (m_plot->axisEnabled (axis) == axisOn &&
			majTicksTypeList[axis] == majTicksType &&
			minTicksTypeList[axis] == minTicksType &&
			axesColors()[axis] == c.name() &&
            axesNumColors()[axis] == labelsColor.name() &&
			prec == m_plot->axisLabelPrecision (axis) &&
			format == m_plot->axisLabelFormat (axis) &&
			labelsRotation(axis) == rotation &&
			axisType[axis] == type &&
			axesFormatInfo[axis] == formatInfo &&
			axesFormulas[axis] == formula &&
			scale->margin() == baselineDist &&
			sclDraw->hasComponent (QwtAbstractScaleDraw::Labels) == labelsOn)
		return;

	scale->setMargin(baselineDist);
	QPalette pal = scale->palette();
	if (pal.color(QPalette::Active, QPalette::WindowText) != c)
		pal.setColor(QPalette::WindowText, c);
    if (pal.color(QPalette::Active, QPalette::Text) != labelsColor)
		pal.setColor(QPalette::Text, labelsColor);
    scale->setPalette(pal);

	if (!labelsOn)
		sclDraw->enableComponent (QwtAbstractScaleDraw::Labels, false);
	else
	{
		if (type == Numeric)
			setLabelsNumericFormat(axis, format, prec, formula);
		else if (type == Day)
			setLabelsDayFormat (axis, format);
		else if (type == Month)
			setLabelsMonthFormat (axis, format);
		else if (type == Time || type == Date)
			setLabelsDateTimeFormat (axis, type, formatInfo);
		else
			setLabelsTextFormat(axis, type, formatInfo, table);

		setAxisLabelRotation(axis, rotation);
	}

	sclDraw = (ScaleDraw *)m_plot->axisScaleDraw (axis);
	sclDraw->enableComponent(QwtAbstractScaleDraw::Backbone, drawAxesBackbone);

	setAxisTicksLength(axis, majTicksType, minTicksType,
			m_plot->minorTickLength(), m_plot->majorTickLength());

	if (axisOn && (axis == QwtPlot::xTop || axis == QwtPlot::yRight))
		updateSecondaryAxis(axis);//synchronize scale divisions

	scalePicker->refresh();
	m_plot->updateLayout();	//This is necessary in order to enable/disable tick labels
	scale->repaint();
	m_plot->replot();
	emit modified();
}

void Layer::setLabelsDayFormat(int axis, int format)
{
	axisType[axis] = Day;
	axesFormatInfo[axis] = QString::number(format);

	ScaleDraw *sd_old = (ScaleDraw *)m_plot->axisScaleDraw (axis);
	const QwtScaleDiv div = sd_old->scaleDiv ();

	WeekDayScaleDraw *sd = new WeekDayScaleDraw((WeekDayScaleDraw::NameFormat)format);
	sd->setScaleDiv(div);
	m_plot->setAxisScaleDraw (axis, sd);
}

void Layer::setLabelsMonthFormat(int axis, int format)
{
	axisType[axis] = Month;
	axesFormatInfo[axis] = QString::number(format);

	ScaleDraw *sd_old = (ScaleDraw *)m_plot->axisScaleDraw (axis);
	const QwtScaleDiv div = sd_old->scaleDiv ();

	MonthScaleDraw *sd = new MonthScaleDraw((MonthScaleDraw::NameFormat)format);
	sd->setScaleDiv(div);
	m_plot->setAxisScaleDraw (axis, sd);
}

void Layer::setLabelsTextFormat(int axis, int type, const QString& name, const QStringList& lst)
{
	if (type != Txt && type != ColHeader)
		return;

    axesFormatInfo[axis] = name;
	m_plot->setAxisScaleDraw (axis, new QwtTextScaleDraw(lst));
	axisType[axis] = type;
}

void Layer::setLabelsTextFormat(int axis, int type, const QString& labelsColName, Table *table)
{
	if (type != Txt && type != ColHeader)
		return;

	QStringList list;
	if (type == Txt)
	{
		if (!table)
			return;

		axesFormatInfo[axis] = labelsColName;
		int r = table->rowCount();
		int col = table->colIndex(labelsColName);

		for (int i=0; i < r; i++)
			list<<table->text(i, col);
	}
	else if (type == ColHeader)
	{
		if (!table)
			return;

		axesFormatInfo[axis] = table->name();
		for (int i=0; i<table->columnCount(); i++)
		{
			if (table->plotDesignation(i) == SciDAVis::Y)
				list<<table->colLabel(i);
		}
	}
	m_plot->setAxisScaleDraw (axis, new QwtTextScaleDraw(list));
	axisType[axis] = type;
}

void Layer::setLabelsDateTimeFormat(int axis, int type, const QString& formatInfo)
{
	if (type < Time)
		return;

	QStringList list = formatInfo.split(";", QString::KeepEmptyParts);
	if ((int)list.count() < 2)
	{
        QMessageBox::critical(this, tr("Error"), "Couldn't change the axis type to the requested format!");
        return;
    }
    if (list[0].isEmpty() || list[1].isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), "Couldn't change the axis type to the requested format!");
        return;
    }

	if (type == Time)
	{
		TimeScaleDraw *sd = new TimeScaleDraw (QTime::fromString (list[0]), list[1]);
		sd->enableComponent (QwtAbstractScaleDraw::Backbone, drawAxesBackbone);
		m_plot->setAxisScaleDraw (axis, sd);
	}
	else if (type == Date)
	{
		DateScaleDraw *sd = new DateScaleDraw (QDate::fromString (list[0], Qt::ISODate), list[1]);
		sd->enableComponent (QwtAbstractScaleDraw::Backbone, drawAxesBackbone);
		m_plot->setAxisScaleDraw (axis, sd);
	}

	axisType[axis] = type;
	axesFormatInfo[axis] = formatInfo;
}

void Layer::setAxisLabelRotation(int axis, int rotation)
{
	if (axis==QwtPlot::xBottom)
	{
		if (rotation > 0)
			m_plot->setAxisLabelAlignment(axis, Qt::AlignRight|Qt::AlignVCenter);
		else if (rotation < 0)
			m_plot->setAxisLabelAlignment(axis, Qt::AlignLeft|Qt::AlignVCenter);
		else if (rotation == 0)
			m_plot->setAxisLabelAlignment(axis, Qt::AlignHCenter|Qt::AlignBottom);
	}
	else if (axis==QwtPlot::xTop)
	{
		if (rotation > 0)
			m_plot->setAxisLabelAlignment(axis, Qt::AlignLeft|Qt::AlignVCenter);
		else if (rotation < 0)
			m_plot->setAxisLabelAlignment(axis, Qt::AlignRight|Qt::AlignVCenter);
		else if (rotation == 0)
			m_plot->setAxisLabelAlignment(axis, Qt::AlignHCenter|Qt::AlignTop);
	}
	m_plot->setAxisLabelRotation (axis, (double)rotation);
}

int Layer::labelsRotation(int axis)
{
	ScaleDraw *sclDraw = (ScaleDraw *)m_plot->axisScaleDraw (axis);
	return (int)sclDraw->labelRotation();
}

void Layer::setYAxisTitleFont(const QFont &fnt)
{
	QwtText t = m_plot->axisTitle (QwtPlot::yLeft);
	t.setFont (fnt);
	m_plot->setAxisTitle (QwtPlot::yLeft, t);
	m_plot->replot();
	emit modified();
}

void Layer::setXAxisTitleFont(const QFont &fnt)
{
	QwtText t = m_plot->axisTitle (QwtPlot::xBottom);
	t.setFont (fnt);
	m_plot->setAxisTitle (QwtPlot::xBottom, t);
	m_plot->replot();
	emit modified();
}

void Layer::setRightAxisTitleFont(const QFont &fnt)
{
	QwtText t = m_plot->axisTitle (QwtPlot::yRight);
	t.setFont (fnt);
	m_plot->setAxisTitle (QwtPlot::yRight, t);
	m_plot->replot();
	emit modified();
}

void Layer::setTopAxisTitleFont(const QFont &fnt)
{
	QwtText t = m_plot->axisTitle (QwtPlot::xTop);
	t.setFont (fnt);
	m_plot->setAxisTitle (QwtPlot::xTop, t);
	m_plot->replot();
	emit modified();
}

void Layer::setAxisTitleFont(int axis,const QFont &fnt)
{
	QwtText t = m_plot->axisTitle (axis);
	t.setFont (fnt);
	m_plot->setAxisTitle(axis, t);
	m_plot->replot();
	emit modified();
}

QFont Layer::axisTitleFont(int axis)
{
	return m_plot->axisTitle(axis).font();
}

QColor Layer::axisTitleColor(int axis)
{
	QColor c;
	QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(axis);
	if (scale)
		c = scale->title().color();
	return c;
}

void Layer::setAxesNumColors(const QStringList& colors)
{
  	for (int i=0;i<4;i++)
  	{
  	     QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
  	     if (scale)
  	     {
  	         QPalette pal = scale->palette();
  	         pal.setColor(QPalette::Text, QColor(colors[i]));
  	         scale->setPalette(pal);
  	     }
  	}
}

void Layer::setAxesColors(const QStringList& colors)
{
	for (int i=0;i<4;i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
		if (scale)
		{
			QPalette pal =scale->palette();
			pal.setColor(QPalette::WindowText,QColor(colors[i]));
			scale->setPalette(pal);
		}
	}
}

QString Layer::saveAxesColors()
{
	QString s="AxesColors\t";
	QStringList colors, numColors;
	QPalette pal;
	int i;
	for (i=0;i<4;i++)
    {
		colors<<QColor(Qt::black).name();
        numColors<<QColor(Qt::black).name();
    }

	for (i=0;i<4;i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
		if (scale)
		{
			pal=scale->palette();
			colors[i]=pal.color(QPalette::Active, QPalette::WindowText).name();
            numColors[i]=pal.color(QPalette::Active, QPalette::Text).name();
		}
	}
	s+=colors.join ("\t")+"\n";
    s+="AxesNumberColors\t"+numColors.join ("\t")+"\n";
	return s;
}

QStringList Layer::axesColors()
{
	QStringList colors;
	QPalette pal;
	int i;
	for (i=0;i<4;i++)
		colors<<QColor(Qt::black).name();

	for (i=0;i<4;i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
		if (scale)
		{
			pal=scale->palette();
			colors[i]=pal.color(QPalette::Active, QPalette::WindowText).name();
		}
	}
	return colors;
}

QColor Layer::axisColor(int axis)
{
    QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(axis);
    if (scale)
  	     return scale->palette().color(QPalette::Active, QPalette::WindowText);
  	else
  	     return QColor(Qt::black);
}

QColor Layer::axisNumbersColor(int axis)
{
    QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(axis);
 	if (scale)
  	     return scale->palette().color(QPalette::Active, QPalette::Text);
  	else
  	     return QColor(Qt::black);
}

QStringList Layer::axesNumColors()
{
  	QStringList colors;
  	QPalette pal;
  	int i;
  	for (i=0;i<4;i++)
  	     colors << QColor(Qt::black).name();

  	for (i=0;i<4;i++)
  	{
  	     QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
  	     if (scale)
  	     {
  	         pal=scale->palette();
  	         colors[i]=pal.color(QPalette::Active, QPalette::Text).name();
  	     }
  	}
  	return colors;
}

void Layer::setTitleColor(const QColor & c)
{
	QwtText t = m_plot->title();
	t.setColor(c);
	m_plot->setTitle (t);
	m_plot->replot();
	emit modified();
}

void Layer::setTitleAlignment(int align)
{
	QwtText t = m_plot->title();
	t.setRenderFlags(align);
	m_plot->setTitle (t);
	m_plot->replot();
	emit modified();
}

void Layer::setTitleFont(const QFont &fnt)
{
	QwtText t = m_plot->title();
	t.setFont(fnt);
	m_plot->setTitle (t);
	m_plot->replot();
	emit modified();
}

void Layer::setYAxisTitle(const QString& text)
{
	m_plot->setAxisTitle(0,text);
	m_plot->replot();
	emit modified();
}

void Layer::setXAxisTitle(const QString& text)
{
	m_plot->setAxisTitle(2,text);
	m_plot->replot();
	emit modified();
}

void Layer::setRightAxisTitle(const QString& text)
{
	m_plot->setAxisTitle(1,text);
	m_plot->replot();
	emit modified();
}

void Layer::setTopAxisTitle(const QString& text)
{
	m_plot->setAxisTitle(3,text);
	m_plot->replot();
	emit modified();
}

int Layer::axisTitleAlignment (int axis)
{
	return m_plot->axisTitle(axis).renderFlags();
}

void Layer::setAxesTitlesAlignment(const QStringList& align)
{
	for (int i=0;i<4;i++)
	{
		QwtText t = m_plot->axisTitle(i);
		t.setRenderFlags(align[i+1].toInt());
		m_plot->setAxisTitle (i, t);
	}
}

void Layer::setXAxisTitleAlignment(int align)
{
	QwtText t = m_plot->axisTitle(QwtPlot::xBottom);
	t.setRenderFlags(align);
	m_plot->setAxisTitle (QwtPlot::xBottom, t);

	m_plot->replot();
	emit modified();
}

void Layer::setYAxisTitleAlignment(int align)
{
	QwtText t = m_plot->axisTitle(QwtPlot::yLeft);
	t.setRenderFlags(align);
	m_plot->setAxisTitle (QwtPlot::yLeft, t);

	m_plot->replot();
	emit modified();
}

void Layer::setTopAxisTitleAlignment(int align)
{
	QwtText t = m_plot->axisTitle(QwtPlot::xTop);
	t.setRenderFlags(align);
	m_plot->setAxisTitle (QwtPlot::xTop, t);
	m_plot->replot();
	emit modified();
}

void Layer::setRightAxisTitleAlignment(int align)
{
	QwtText t = m_plot->axisTitle(QwtPlot::yRight);
	t.setRenderFlags(align);
	m_plot->setAxisTitle (QwtPlot::yRight, t);

	m_plot->replot();
	emit modified();
}

void Layer::setAxisTitle(int axis, const QString& text)
{
	int a;
	switch (axis)
	{
		case 0:
			a=2;
        break;
		case 1:
			a=0;
        break;
		case 2:
			a=3;
        break;
		case 3:
			a=1;
        break;
	}
	m_plot->setAxisTitle(a, text);
	m_plot->replot();
	emit modified();
}

void Layer::setGridOptions(const GridOptions& o)
{
	if (grid.majorCol == o.majorCol && grid.majorOnX == o.majorOnX &&
			grid.majorOnY == o.majorOnY && grid.majorStyle == o.majorStyle &&
			grid.majorWidth == o.majorWidth && grid.minorCol == o.minorCol &&
			grid.minorOnX == o.minorOnX && grid.minorOnY == o.minorOnY &&
			grid.minorStyle == o.minorStyle && grid.minorWidth == o.minorWidth &&
			grid.xAxis == o.xAxis && grid.yAxis == o.yAxis &&
			grid.xZeroOn == o.xZeroOn && grid.yZeroOn == o.yZeroOn) return;

	grid=o;

	QColor minColor = ColorBox::color(grid.minorCol);
	QColor majColor = ColorBox::color(grid.majorCol);

	Qt::PenStyle majStyle = getPenStyle(grid.majorStyle);
	Qt::PenStyle minStyle = getPenStyle(grid.minorStyle);

	QPen majPen=QPen (majColor,grid.majorWidth,majStyle);
	m_plot->grid()->setMajPen (majPen);

	QPen minPen=QPen (minColor,grid.minorWidth,minStyle);
	m_plot->grid()->setMinPen(minPen);

	if (grid.majorOnX) m_plot->grid()->enableX (true);
	else if (grid.majorOnX==0) m_plot->grid()->enableX (false);

	if (grid.minorOnX) m_plot->grid()->enableXMin (true);
	else if (grid.minorOnX==0) m_plot->grid()->enableXMin (false);

	if (grid.majorOnY) m_plot->grid()->enableY (true);
	else if (grid.majorOnY==0) m_plot->grid()->enableY (false);

	if (grid.minorOnY) m_plot->grid()->enableYMin (true);
	else m_plot->grid()->enableYMin (false);

	m_plot->grid()->setAxis(grid.xAxis, grid.yAxis);

	if (mrkX<0 && grid.xZeroOn)
	{
		QwtPlotMarker *m = new QwtPlotMarker();
		mrkX = m_plot->insertMarker(m);
		m->setAxis(grid.xAxis, grid.yAxis);
		m->setLineStyle(QwtPlotMarker::VLine);
		m->setValue(0.0,0.0);
		m->setLinePen(QPen(Qt::black, 2,Qt::SolidLine));
	}
	else if (mrkX>=0 && !grid.xZeroOn)
	{
		m_plot->removeMarker(mrkX);
		mrkX=-1;
	}

	if (mrkY<0 && grid.yZeroOn)
	{
		QwtPlotMarker *m = new QwtPlotMarker();
		mrkY = m_plot->insertMarker(m);
		m->setAxis(grid.xAxis, grid.yAxis);
		m->setLineStyle(QwtPlotMarker::HLine);
		m->setValue(0.0,0.0);
		m->setLinePen(QPen(Qt::black, 2,Qt::SolidLine));
	}
	else if (mrkY>=0 && !grid.yZeroOn)
	{
		m_plot->removeMarker(mrkY);
		mrkY=-1;
	}

	emit modified();
}

QStringList Layer::scalesTitles()
{
	QStringList scaleTitles;
	int axis;
	for (int i=0;i<QwtPlot::axisCnt;i++)
	{
		switch (i)
		{
			case 0:
				axis=2;
				scaleTitles<<m_plot->axisTitle(axis).text();
				break;

			case 1:
				axis=0;
				scaleTitles<<m_plot->axisTitle(axis).text();
				break;

			case 2:
				axis=3;
				scaleTitles<<m_plot->axisTitle(axis).text();
				break;

			case 3:
				axis=1;
				scaleTitles<<m_plot->axisTitle(axis).text();
				break;
		}
	}
	return scaleTitles;
}

void Layer::updateSecondaryAxis(int axis)
{
	for (int i=0; i<n_curves; i++)
	{
		QwtPlotItem *it = plotItem(i);
		if (!it)
			continue;

		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
  	         {
  	         Spectrogram *sp = (Spectrogram *)it;
  	         if (sp->colorScaleAxis() == axis)
  	              return;
  	         }

		if ((axis == QwtPlot::yRight && it->yAxis() == QwtPlot::yRight) ||
            (axis == QwtPlot::xTop && it->xAxis () == QwtPlot::xTop))
			return;
	}

	int a = QwtPlot::xBottom;
	if (axis == QwtPlot::yRight)
		a = QwtPlot::yLeft;

	if (!m_plot->axisEnabled(a))
		return;

	QwtScaleEngine *se = m_plot->axisScaleEngine(a);
	const QwtScaleDiv *sd = m_plot->axisScaleDiv(a);

	QwtScaleEngine *sc_engine = 0;
	if (se->transformation()->type() == QwtScaleTransformation::Log10)
		sc_engine = new QwtLog10ScaleEngine();
	else if (se->transformation()->type() == QwtScaleTransformation::Linear)
		sc_engine = new QwtLinearScaleEngine();

	if (se->testAttribute(QwtScaleEngine::Inverted))
		sc_engine->setAttribute(QwtScaleEngine::Inverted);

	m_plot->setAxisScaleEngine (axis, sc_engine);
	m_plot->setAxisScaleDiv (axis, *sd);

	m_user_step[axis] = m_user_step[a];

	QwtScaleWidget *scale = m_plot->axisWidget(a);
	int start = scale->startBorderDist();
	int end = scale->endBorderDist();

	scale = m_plot->axisWidget(axis);
	scale->setMinBorderDist (start, end);
}

void Layer::setAutoScale()
{
	for (int i = 0; i < QwtPlot::axisCnt; i++)
		m_plot->setAxisAutoScale(i);

	m_plot->replot();
	m_zoomer[0]->setZoomBase();
	m_zoomer[1]->setZoomBase();
	updateScale();

	emit modified();
}

void Layer::setScale(int axis, double start, double end, double step, int majorTicks, int minorTicks, int type, bool inverted)
{
	QwtScaleEngine *sc_engine = 0;
	if (type)
		sc_engine = new QwtLog10ScaleEngine();
	else
		sc_engine = new QwtLinearScaleEngine();

	int max_min_intervals = minorTicks;
	if (minorTicks == 1)
		max_min_intervals = 3;
	if (minorTicks > 1)
		max_min_intervals = minorTicks + 1;

	QwtScaleDiv div = sc_engine->divideScale (qMin(start, end), qMax(start, end), majorTicks, max_min_intervals, step);
	m_plot->setAxisMaxMajor (axis, majorTicks);
	m_plot->setAxisMaxMinor (axis, minorTicks);

	if (inverted){
		sc_engine->setAttribute(QwtScaleEngine::Inverted);
		div.invert();
	}

	m_plot->setAxisScaleEngine (axis, sc_engine);
	m_plot->setAxisScaleDiv (axis, div);

	m_zoomer[0]->setZoomBase();
	m_zoomer[1]->setZoomBase();

	m_user_step[axis] = step;

	if (axis == QwtPlot::xBottom || axis == QwtPlot::yLeft){
  		updateSecondaryAxis(QwtPlot::xTop);
  	    updateSecondaryAxis(QwtPlot::yRight);
  	}

	m_plot->replot();
	//keep markers on canvas area
	updateMarkersBoundingRect();
	m_plot->replot();
}

QStringList Layer::analysableCurvesList()
{
	QStringList cList;
	QList<int> keys = m_plot->curveKeys();
	for (int i=0; i<(int)keys.count(); i++)
	{
		QwtPlotCurve *c = m_plot->curve(keys[i]);
  	    if (c && c_type[i] != ErrorBars)
  	        cList << c->title().text();
  	 }
return cList;
}

QStringList Layer::curvesList()
{
	QStringList cList;
	QList<int> keys = m_plot->curveKeys();
	for (int i=0; i<(int)keys.count(); i++)
	{
		QwtPlotCurve *c = m_plot->curve(keys[i]);
  	    if (c)
  	        cList << c->title().text();
  	 }
return cList;
}

QStringList Layer::plotItemsList()
{
  	QStringList cList;
  	QList<int> keys = m_plot->curveKeys();
  	for (int i=0; i<(int)keys.count(); i++)
  	{
  		QwtPlotItem *it = m_plot->plotItem(keys[i]);
		if (it)
			cList << it->title().text();
	}
	return cList;
}

void Layer::copyImage()
{
	QApplication::clipboard()->setPixmap(graphPixmap(), QClipboard::Clipboard);
}

QPixmap Layer::graphPixmap()
{
	return QPixmap::grabWidget(this);
}

void Layer::exportToFile(const QString& fileName)
{
	if ( fileName.isEmpty() ){
		QMessageBox::critical(this, tr("Error"), tr("Please provide a valid file name!"));
        return;
	}

	if (fileName.contains(".eps") || fileName.contains(".pdf") || fileName.contains(".ps")){
		exportVector(fileName);
		return;
	} else if(fileName.contains(".svg")){
		exportSVG(fileName);
		return;
	} else {
		QList<QByteArray> list = QImageWriter::supportedImageFormats();
    	for(int i=0 ; i<list.count() ; i++){
			if (fileName.contains( "." + list[i].toLower())){
				exportImage(fileName);
				return;
			}
		}
    	QMessageBox::critical(this, tr("Error"), tr("File format not handled, operation aborted!"));
	}
}

void Layer::exportImage(const QString& fileName, int quality, bool transparent)
{
	QPixmap pic = graphPixmap();

	if (transparent){
		QBitmap mask(pic.size());
		mask.fill(Qt::color1);
		QPainter p;
		p.begin(&mask);
		p.setPen(Qt::color0);

        QColor background = QColor (Qt::white);
		QRgb backgroundPixel = background.rgb ();
		QImage image = pic.convertToImage();
		for (int y=0; y<image.height(); y++){
			for ( int x=0; x<image.width(); x++ ){
				QRgb rgb = image.pixel(x, y);
				if (rgb == backgroundPixel) // we want the frame transparent
					p.drawPoint(x, y);
			}
		}
		p.end();
		pic.setMask(mask);
	}

	pic.save(fileName, 0, quality);
}

void Layer::exportVector(const QString& fileName, int res, bool color, bool keepAspect, QPrinter::PageSize pageSize)
{
	if ( fileName.isEmpty() ){
		QMessageBox::critical(this, tr("Error"), tr("Please provide a valid file name!"));
        return;
	}

	QPrinter printer;
    printer.setCreator("SciDAVis");
	printer.setFullPage(true);
	if (res)
		printer.setResolution(res);

    printer.setOutputFileName(fileName);
    if (fileName.contains(".eps"))
    	printer.setOutputFormat(QPrinter::PostScriptFormat);

    if (color)
		printer.setColorMode(QPrinter::Color);
	else
		printer.setColorMode(QPrinter::GrayScale);

    QRect plotRect = m_plot->rect();
    if (pageSize == QPrinter::Custom)
        printer.setPageSize(minPageSize(printer, plotRect));
    else
        printer.setPageSize(pageSize);

    double plot_aspect = double(m_plot->frameGeometry().width())/double(m_plot->frameGeometry().height());
	if (plot_aspect < 1)
		printer.setOrientation(QPrinter::Portrait);
	else
		printer.setOrientation(QPrinter::Landscape);

    if (keepAspect){// export should preserve plot aspect ratio
        double page_aspect = double(printer.width())/double(printer.height());
        if (page_aspect > plot_aspect){
            int margin = (int) ((0.1/2.54)*printer.logicalDpiY()); // 1 mm margins
            int height = printer.height() - 2*margin;
            int width = height*plot_aspect;
            int x = (printer.width()- width)/2;
            plotRect = QRect(x, margin, width, height);
        } else if (plot_aspect >= page_aspect){
            int margin = (int) ((0.1/2.54)*printer.logicalDpiX()); // 1 mm margins
            int width = printer.width() - 2*margin;
            int height = width/plot_aspect;
            int y = (printer.height()- height)/2;
            plotRect = QRect(margin, y, width, height);
        }
	} else {
	    int x_margin = (int) ((0.1/2.54)*printer.logicalDpiX()); // 1 mm margins
        int y_margin = (int) ((0.1/2.54)*printer.logicalDpiY()); // 1 mm margins
        int width = printer.width() - 2*x_margin;
        int height = printer.height() - 2*y_margin;
        plotRect = QRect(x_margin, y_margin, width, height);
	}

    QPainter paint(&printer);
	m_plot->print(&paint, plotRect);
}

void Layer::print()
{
	QPrinter printer;
	printer.setColorMode (QPrinter::Color);
	printer.setFullPage(true);

	//printing should preserve plot aspect ratio, if possible
	double aspect = double(m_plot->width())/double(m_plot->height());
	if (aspect < 1)
		printer.setOrientation(QPrinter::Portrait);
	else
		printer.setOrientation(QPrinter::Landscape);

	QPrintDialog printDialog(&printer);
    if (printDialog.exec() == QDialog::Accepted)
	{
		QRect plotRect = m_plot->rect();
		QRect paperRect = printer.paperRect();
		if (m_scale_on_print)
		{
			int dpiy = printer.logicalDpiY();
			int margin = (int) ((2/2.54)*dpiy ); // 2 cm margins

			int width = qRound(aspect*printer.height()) - 2*margin;
			int x=qRound(abs(printer.width()- width)*0.5);

			plotRect = QRect(x, margin, width, printer.height() - 2*margin);
			if (x < margin)
			{
				plotRect.setLeft(margin);
				plotRect.setWidth(printer.width() - 2*margin);
			}
		}
		else
		{
    		int x_margin = (paperRect.width() - plotRect.width())/2;
    		int y_margin = (paperRect.height() - plotRect.height())/2;
    		plotRect.moveTo(x_margin, y_margin);
		}

        QPainter paint(&printer);
        if (m_print_cropmarks)
        {
			QRect cr = plotRect; // cropmarks rectangle
			cr.addCoords(-1, -1, 2, 2);
            paint.save();
            paint.setPen(QPen(QColor(Qt::black), 0.5, Qt::DashLine));
            paint.drawLine(paperRect.left(), cr.top(), paperRect.right(), cr.top());
            paint.drawLine(paperRect.left(), cr.bottom(), paperRect.right(), cr.bottom());
            paint.drawLine(cr.left(), paperRect.top(), cr.left(), paperRect.bottom());
            paint.drawLine(cr.right(), paperRect.top(), cr.right(), paperRect.bottom());
            paint.restore();
        }

		m_plot->print(&paint, plotRect);
	}
}

void Layer::exportSVG(const QString& fname)
{
	#if QT_VERSION >= 0x040300
		QSvgGenerator svg;
        svg.setFileName(fname);
        svg.setSize(m_plot->size());

		QPainter p(&svg);
		m_plot->print(&p, m_plot->rect());
		p.end();
	#endif
}

int Layer::selectedCurveID()
{
	if (m_range_selector)
		return curveKey(curveIndex(m_range_selector->selectedCurve()));
	else
		return -1;
}

QString Layer::selectedCurveTitle()
{
	if (m_range_selector)
		return m_range_selector->selectedCurve()->title().text();
	else
		return QString::null;
}

bool Layer::markerSelected()
{
	return (selectedMarker>=0);
}

void Layer::removeMarker()
{
	if (selectedMarker>=0)
	{
		if (m_markers_selector) {
			if (m_texts.contains(selectedMarker))
				m_markers_selector->removeAll((TextEnrichment*)m_plot->marker(selectedMarker));
			else if (m_lines.contains(selectedMarker))
				m_markers_selector->removeAll((LineEnrichment*)m_plot->marker(selectedMarker));
			else if (m_images.contains(selectedMarker))
				m_markers_selector->removeAll((ImageEnrichment*)m_plot->marker(selectedMarker));
		}
		m_plot->removeMarker(selectedMarker);
		m_plot->replot();
		emit modified();

		if (selectedMarker==legendMarkerID)
			legendMarkerID=-1;

		if (m_lines.contains(selectedMarker)>0)
		{
			int index = m_lines.indexOf(selectedMarker);
            int last_line_marker = (int)m_lines.size() - 1;
			for (int i=index; i < last_line_marker; i++)
				m_lines[i]=m_lines[i+1];
			m_lines.resize(last_line_marker);
		}
		else if(m_texts.contains(selectedMarker)>0)
		{
			int index=m_texts.indexOf(selectedMarker);
			int last_text_marker = m_texts.size() - 1;
			for (int i=index; i < last_text_marker; i++)
				m_texts[i]=m_texts[i+1];
			m_texts.resize(last_text_marker);
		}
		else if(m_images.contains(selectedMarker)>0)
		{
			int index=m_images.indexOf(selectedMarker);
			int last_image_marker = m_images.size() - 1;
			for (int i=index; i < last_image_marker; i++)
				m_images[i]=m_images[i+1];
			m_images.resize(last_image_marker);
		}
		selectedMarker=-1;
	}
}

void Layer::cutMarker()
{
	copyMarker();
	removeMarker();
}

bool Layer::arrowMarkerSelected()
{
	return (m_lines.contains(selectedMarker));
}

bool Layer::imageMarkerSelected()
{
	return (m_images.contains(selectedMarker));
}

void Layer::copyMarker()
{
	if (selectedMarker<0){
		selectedMarkerType=None;
		return ;
	}

	if (m_lines.contains(selectedMarker)){
		LineEnrichment* mrkL=(LineEnrichment*) m_plot->marker(selectedMarker);
		auxMrkStart=mrkL->startPoint();
		auxMrkEnd=mrkL->endPoint();
		selectedMarkerType = Arrow;
	} else if (m_images.contains(selectedMarker)){
		ImageEnrichment* mrkI=(ImageEnrichment*) m_plot->marker(selectedMarker);
		auxMrkStart=mrkI->origin();
		QRect rect=mrkI->rect();
		auxMrkEnd=rect.bottomRight();
		auxMrkFileName=mrkI->fileName();
		selectedMarkerType=Image;
	} else
		selectedMarkerType=Text;
}

void Layer::pasteMarker()
{
	if (selectedMarkerType == Arrow){
		LineEnrichment* mrkL = new LineEnrichment();
        int linesOnPlot = (int)m_lines.size();
  	    m_lines.resize(++linesOnPlot);
  	    m_lines[linesOnPlot-1] = m_plot->insertMarker(mrkL);

		mrkL->setColor(auxMrkColor);
		mrkL->setWidth(auxMrkWidth);
		mrkL->setStyle(auxMrkStyle);
		mrkL->setStartPoint(QPoint(auxMrkStart.x()+10,auxMrkStart.y()+10));
		mrkL->setEndPoint(QPoint(auxMrkEnd.x()+10,auxMrkEnd.y()+10));
		mrkL->drawStartArrow(startArrowOn);
		mrkL->drawEndArrow(endArrowOn);
		mrkL->setHeadLength(auxArrowHeadLength);
		mrkL->setHeadAngle(auxArrowHeadAngle);
		mrkL->fillArrowHead(auxFilledArrowHead);
	} else if (selectedMarkerType==Image){
		ImageEnrichment* mrk = new ImageEnrichment(auxMrkFileName);
		int imagesOnPlot=m_images.size();
  	    m_images.resize(++imagesOnPlot);
  	    m_images[imagesOnPlot-1] = m_plot->insertMarker(mrk);

		QPoint o = m_plot->canvas()->mapFromGlobal(QCursor::pos());
		if (!m_plot->canvas()->contentsRect().contains(o))
			o = QPoint(auxMrkStart.x()+20, auxMrkStart.y()+20);
		mrk->setOrigin(o);
		mrk->setSize(QRect(auxMrkStart,auxMrkEnd).size());
	} else {
		TextEnrichment* mrk=new TextEnrichment(m_plot);
        int texts = m_texts.size();
  	    m_texts.resize(++texts);
  	    m_texts[texts-1] = m_plot->insertMarker(mrk);

		QPoint o=m_plot->canvas()->mapFromGlobal(QCursor::pos());
		if (!m_plot->canvas()->contentsRect().contains(o))
			o=QPoint(auxMrkStart.x()+20,auxMrkStart.y()+20);
		mrk->setOrigin(o);
		mrk->setAngle(auxMrkAngle);
		mrk->setFrameStyle(auxMrkBkg);
		mrk->setFont(auxMrkFont);
		mrk->setText(auxMrkText);
		mrk->setTextColor(auxMrkColor);
		mrk->setBackgroundColor(auxMrkBkgColor);
	}
	
	m_plot->replot();
	deselectMarker();
}

void Layer::setCopiedMarkerEnds(const QPoint& start, const QPoint& end)
{
	auxMrkStart=start;
	auxMrkEnd=end;
}

void Layer::setCopiedTextOptions(int bkg, const QString& text, const QFont& font,
		const QColor& color, const QColor& bkgColor)
{
	auxMrkBkg=bkg;
	auxMrkText=text;
	auxMrkFont=font;
	auxMrkColor=color;
	auxMrkBkgColor = bkgColor;
}

void Layer::setCopiedArrowOptions(int width, Qt::PenStyle style, const QColor& color,
		bool start, bool end, int headLength,
		int headAngle, bool filledHead)
{
	auxMrkWidth=width;
	auxMrkStyle=style;
	auxMrkColor=color;
	startArrowOn=start;
	endArrowOn=end;
	auxArrowHeadLength=headLength;
	auxArrowHeadAngle=headAngle;
	auxFilledArrowHead=filledHead;
}

bool Layer::titleSelected()
{
	return m_plot->titleLabel()->hasFocus();
}

void Layer::selectTitle()
{
	if (!m_plot->hasFocus())
	{
		emit selected(this);
		QwtTextLabel *title = m_plot->titleLabel();
		title->setFocus();
	}

	deselectMarker();
}

void Layer::setTitle(const QString& t)
{
	m_plot->setTitle (t);
	emit modified();
}

void Layer::removeTitle()
{
	if (m_plot->titleLabel()->hasFocus())
	{
		m_plot->setTitle(" ");
		emit modified();
	}
}

void Layer::initTitle(bool on, const QFont& fnt)
{
	if (on)
	{
		QwtText t = m_plot->title();
		t.setFont(fnt);
		t.setText(tr("Title"));
		m_plot->setTitle (t);
	}
}

void Layer::removeLegend()
{
	if (legendMarkerID >= 0)
	{
		int index = m_texts.indexOf(legendMarkerID);
		int texts = m_texts.size();
		for (int i=index; i<texts; i++)
			m_texts[i]=m_texts[i+1];
		m_texts.resize(--texts);

		m_plot->removeMarker(legendMarkerID);
		legendMarkerID=-1;
	}
}

void Layer::updateImageEnrichment(int x, int y, int w, int h)
{
	ImageEnrichment* mrk =(ImageEnrichment*) m_plot->marker(selectedMarker);
	mrk->setRect(x, y, w, h);
	m_plot->replot();
	emit modified();
}

void Layer::updateTextMarker(const QString& text,int angle, int bkg,const QFont& fnt,
		const QColor& textColor, const QColor& backgroundColor)
{
	TextEnrichment* mrkL=(TextEnrichment*) m_plot->marker(selectedMarker);
	mrkL->setText(text);
	mrkL->setAngle(angle);
	mrkL->setTextColor(textColor);
	mrkL->setBackgroundColor(backgroundColor);
	mrkL->setFont(fnt);
	mrkL->setFrameStyle(bkg);

	m_plot->replot();
	emit modified();
}

TextEnrichment* Layer::legend()
{
	if (legendMarkerID >=0 )
		return (TextEnrichment*) m_plot->marker(legendMarkerID);
	else
		return 0;
}

QString Layer::legendText()
{
	QString text="";
	for (int i=0; i<n_curves; i++)
	{
		const QwtPlotCurve *c = curve(i);
		if (c && c->rtti() != QwtPlotItem::Rtti_PlotSpectrogram && c_type[i] != ErrorBars )
		{
			text+="\\c{";
			text+=QString::number(i+1);
			text+="}";
			text+=c->title().text();
			text+="\n";
		}
	}
	return text;
}

QString Layer::pieLegendText()
{
	QString text="";
	QList<int> keys= m_plot->curveKeys();
	const QwtPlotCurve *curve = (QwtPlotCurve *)m_plot->curve(keys[0]);
	if (curve)
	{
		for (int i=0;i<int(curve->dataSize());i++)
		{
			text+="\\p{";
			text+=QString::number(i+1);
			text+="} ";
			text+=QString::number(i+1);
			text+="\n";
		}
	}
	return text;
}

void Layer::updateCurvesData(Table* w, const QString& yColName)
{
    QList<int> keys = m_plot->curveKeys();
    int updated_curves = 0;
	for (int i=0; i<(int)keys.count(); i++)
	{
		QwtPlotItem *it = m_plot->plotItem(keys[i]);
		if (!it)
            continue;
        if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
            continue;
        if (((PlotCurve *)it)->type() == Function)
            continue;

        if(((DataCurve *)it)->updateData(w, yColName))
            updated_curves++;
	}
    if (updated_curves)
        updatePlot();
}

QString Layer::saveEnabledAxes()
{
	QString list="EnabledAxes\t";
	for (int i = 0;i<QwtPlot::axisCnt;i++)
		list+=QString::number(m_plot->axisEnabled (i))+"\t";

	list+="\n";
	return list;
}

bool Layer::framed()
{
	bool frameOn=false;

	QwtPlotCanvas* canvas=(QwtPlotCanvas*) m_plot->canvas();
	if (canvas->lineWidth()>0)
		frameOn=true;

	return frameOn;
}

QColor Layer::canvasFrameColor()
{
	QwtPlotCanvas* canvas=(QwtPlotCanvas*) m_plot->canvas();
	QPalette pal =canvas->palette();
	return pal.color(QPalette::Active, QPalette::WindowText);
}

int Layer::canvasFrameWidth()
{
	QwtPlotCanvas* canvas=(QwtPlotCanvas*) m_plot->canvas();
	return canvas->lineWidth();
}

void Layer::drawCanvasFrame(const QStringList& frame)
{
	QwtPlotCanvas* canvas=(QwtPlotCanvas*) m_plot->canvas();
	canvas->setLineWidth((frame[1]).toInt());

	QPalette pal = canvas->palette();
	pal.setColor(QPalette::WindowText,QColor(frame[2]));
	canvas->setPalette(pal);
}

void Layer::drawCanvasFrame(bool frameOn, int width, const QColor& color)
{
	QwtPlotCanvas* canvas=(QwtPlotCanvas*) m_plot->canvas();
	QPalette pal = canvas->palette();

	if (frameOn && canvas->lineWidth() == width &&
			pal.color(QPalette::Active, QPalette::WindowText) == color)
		return;

	if (frameOn)
	{
		canvas->setLineWidth(width);
		pal.setColor(QPalette::WindowText,color);
		canvas->setPalette(pal);
	}
	else
	{
		canvas->setLineWidth(0);
		pal.setColor(QPalette::WindowText,QColor(Qt::black));
		canvas->setPalette(pal);
	}
	emit modified();
}

void Layer::drawCanvasFrame(bool frameOn, int width)
{
	if (frameOn)
	{
		QwtPlotCanvas* canvas=(QwtPlotCanvas*) m_plot->canvas();
		canvas->setLineWidth(width);
	}
}

void Layer::drawAxesBackbones(bool yes)
{
	if (drawAxesBackbone == yes)
		return;

	drawAxesBackbone = yes;

	for (int i=0; i<QwtPlot::axisCnt; i++)
	{
		QwtScaleWidget *scale=(QwtScaleWidget*) m_plot->axisWidget(i);
		if (scale)
		{
			ScaleDraw *sclDraw = (ScaleDraw *)m_plot->axisScaleDraw (i);
			sclDraw->enableComponent (QwtAbstractScaleDraw::Backbone, yes);
			scale->repaint();
		}
	}

	m_plot->replot();
	emit modified();
}

void Layer::loadAxesOptions(const QString& s)
{
	if (s == "1")
		return;

	drawAxesBackbone = false;

	for (int i=0; i<QwtPlot::axisCnt; i++)
	{
		QwtScaleWidget *scale=(QwtScaleWidget*) m_plot->axisWidget(i);
		if (scale)
		{
			ScaleDraw *sclDraw = (ScaleDraw *)m_plot->axisScaleDraw (i);
			sclDraw->enableComponent (QwtAbstractScaleDraw::Backbone, false);
			scale->repaint();
		}
	}
}

void Layer::setAxesLinewidth(int width)
{
	if (m_plot->axesLinewidth() == width)
		return;

	m_plot->setAxesLinewidth(width);

	for (int i=0; i<QwtPlot::axisCnt; i++)
	{
		QwtScaleWidget *scale=(QwtScaleWidget*) m_plot->axisWidget(i);
		if (scale)
		{
			scale->setPenWidth(width);
			scale->repaint();
		}
	}

	m_plot->replot();
	emit modified();
}

void Layer::loadAxesLinewidth(int width)
{
	m_plot->setAxesLinewidth(width);
}

QString Layer::saveCanvas()
{
	QString s="";
	int w = m_plot->canvas()->lineWidth();
	if (w>0)
	{
		s += "CanvasFrame\t" + QString::number(w)+"\t";
		s += canvasFrameColor().name()+"\n";
	}
	s += "CanvasBackground\t" + m_plot->canvasBackground().name()+"\t";
	s += QString::number(m_plot->canvasBackground().alpha())+"\n";
	return s;
}

QString Layer::saveFonts()
{
	int i;
	QString s;
	QStringList list,axesList;
	QFont f;
	list<<"TitleFont";
	f=m_plot->title().font();
	list<<f.family();
	list<<QString::number(f.pointSize());
	list<<QString::number(f.weight());
	list<<QString::number(f.italic());
	list<<QString::number(f.underline());
	list<<QString::number(f.strikeOut());
	s=list.join ("\t")+"\n";

	for (i=0;i<m_plot->axisCnt;i++)
	{
		f=m_plot->axisTitle(i).font();
		list[0]="ScaleFont"+QString::number(i);
		list[1]=f.family();
		list[2]=QString::number(f.pointSize());
		list[3]=QString::number(f.weight());
		list[4]=QString::number(f.italic());
		list[5]=QString::number(f.underline());
		list[6]=QString::number(f.strikeOut());
		s+=list.join ("\t")+"\n";
	}

	for (i=0;i<m_plot->axisCnt;i++)
	{
		f=m_plot->axisFont(i);
		list[0]="AxisFont"+QString::number(i);
		list[1]=f.family();
		list[2]=QString::number(f.pointSize());
		list[3]=QString::number(f.weight());
		list[4]=QString::number(f.italic());
		list[5]=QString::number(f.underline());
		list[6]=QString::number(f.strikeOut());
		s+=list.join ("\t")+"\n";
	}
	return s;
}

QString Layer::saveAxesFormulas()
{
	QString s;
	for (int i=0; i<4; i++)
		if (!axesFormulas[i].isEmpty())
		{
			s += "<AxisFormula pos=\""+QString::number(i)+"\">\n";
			s += axesFormulas[i];
			s += "\n</AxisFormula>\n";
		}
	return s;
}

QString Layer::saveScale()
{
	QString s;
	for (int i=0; i < QwtPlot::axisCnt; i++)
	{
		s += "scale\t" + QString::number(i)+"\t";

		const QwtScaleDiv *scDiv=m_plot->axisScaleDiv(i);
		QwtValueList lst = scDiv->ticks (QwtScaleDiv::MajorTick);

		s += QString::number(qMin(scDiv->lBound(), scDiv->hBound()), 'g', 15)+"\t";
		s += QString::number(qMax(scDiv->lBound(), scDiv->hBound()), 'g', 15)+"\t";
		s += QString::number(m_user_step[i], 'g', 15)+"\t";
		s += QString::number(m_plot->axisMaxMajor(i))+"\t";
		s += QString::number(m_plot->axisMaxMinor(i))+"\t";

		const QwtScaleEngine *sc_eng = m_plot->axisScaleEngine(i);
		QwtScaleTransformation *tr = sc_eng->transformation();
		s += QString::number((int)tr->type())+"\t";
		s += QString::number(sc_eng->testAttribute(QwtScaleEngine::Inverted))+"\n";
	}
	return s;
}

void Layer::setXAxisTitleColor(const QColor& c)
{
	QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(QwtPlot::xBottom);
	if (scale)
	{
		QwtText t = scale->title();
		t.setColor (c);
		scale->setTitle (t);
		emit modified();
	}
}

void Layer::setYAxisTitleColor(const QColor& c)
{
	QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(QwtPlot::yLeft);
	if (scale)
	{
		QwtText t = scale->title();
		t.setColor (c);
		scale->setTitle (t);
		emit modified();
	}
}

void Layer::setRightAxisTitleColor(const QColor& c)
{
	QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(QwtPlot::yRight);
	if (scale)
	{
		QwtText t = scale->title();
		t.setColor (c);
		scale->setTitle (t);
		emit modified();
	}
}

void Layer::setTopAxisTitleColor(const QColor& c)
{
	QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(QwtPlot::xTop);
	if (scale)
	{
		QwtText t = scale->title();
		t.setColor (c);
		scale->setTitle (t);
		emit modified();
	}
}

void Layer::setAxesTitleColor(QStringList l)
{
	for (int i=0;i<int(l.count()-1);i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
		if (scale)
		{
  	        QwtText title = scale->title();
  	        title.setColor(QColor(l[i+1]));
  	        scale->setTitle(title);
  	   }
	}
}

QString Layer::saveAxesTitleColors()
{
	QString s="AxesTitleColors\t";
	for (int i=0;i<4;i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)m_plot->axisWidget(i);
		QColor c;
		if (scale)
			c=scale->title().color();
		else
			c=QColor(Qt::black);

		s+=c.name()+"\t";
	}
	return s+"\n";
}

QString Layer::saveTitle()
{
	QString s="PlotTitle\t";
	s += m_plot->title().text().replace("\n", "<br>")+"\t";
	s += m_plot->title().color().name()+"\t";
	s += QString::number(m_plot->title().renderFlags())+"\n";
	return s;
}

QString Layer::saveScaleTitles()
{
	int a;
	QString s="";
	for (int i=0; i<4; i++)
	{
		switch (i)
		{
			case 0:
				a=2;
            break;
			case 1:
				a=0;
            break;
			case 2:
				a=3;
            break;
			case 3:
				a=1;
            break;
		}
		QString title = m_plot->axisTitle(a).text();
		if (!title.isEmpty())
            s += title.replace("\n", "<br>")+"\t";
        else
            s += "\t";
	}
	return s+"\n";
}

QString Layer::saveAxesTitleAlignement()
{
	QString s="AxesTitleAlignment\t";
	QStringList axes;
	int i;
	for (i=0;i<4;i++)
		axes<<QString::number(Qt::AlignHCenter);

	for (i=0;i<4;i++)
	{

		if (m_plot->axisEnabled(i))
			axes[i]=QString::number(m_plot->axisTitle(i).renderFlags());
	}

	s+=axes.join("\t")+"\n";
	return s;
}

QString Layer::savePieCurveLayout()
{
	QString s="PieCurve\t";

	PieCurve *pieCurve=(PieCurve*)curve(0);
	s+= pieCurve->title().text()+"\t";
	QPen pen=pieCurve->pen();

	s+=QString::number(pen.width())+"\t";
	s+=pen.color().name()+"\t";
	s+=penStyleName(pen.style()) + "\t";

	Qt::BrushStyle pattern=pieCurve->pattern();
	int index;
	if (pattern == Qt::SolidPattern)
		index=0;
	else if (pattern == Qt::HorPattern)
		index=1;
	else if (pattern == Qt::VerPattern)
		index=2;
	else if (pattern == Qt::CrossPattern)
		index=3;
	else if (pattern == Qt::BDiagPattern)
		index=4;
	else if (pattern == Qt::FDiagPattern)
		index=5;
	else if (pattern == Qt::DiagCrossPattern)
		index=6;
	else if (pattern == Qt::Dense1Pattern)
		index=7;
	else if (pattern == Qt::Dense2Pattern)
		index=8;
	else if (pattern == Qt::Dense3Pattern)
		index=9;
	else if (pattern == Qt::Dense4Pattern)
		index=10;
	else if (pattern == Qt::Dense5Pattern)
		index=11;
	else if (pattern == Qt::Dense6Pattern)
		index=12;
	else if (pattern == Qt::Dense7Pattern)
		index=13;

	s+=QString::number(index)+"\t";
	s+=QString::number(pieCurve->ray())+"\t";
	s+=QString::number(pieCurve->firstColor())+"\t";
	s+=QString::number(pieCurve->startRow())+"\t"+QString::number(pieCurve->endRow())+"\t";
	s+=QString::number(pieCurve->isVisible())+"\n";
	return s;
}

QString Layer::saveCurveLayout(int index)
{
	QString s = QString::null;
	int style = c_type[index];
	QwtPlotCurve *c = (QwtPlotCurve*)curve(index);
	if (c)
	{
		s+=QString::number(style)+"\t";
		if (style == Spline)
			s+="5\t";
		else if (style == VerticalSteps)
			s+="6\t";
		else
			s+=QString::number(c->style())+"\t";
		s+=QString::number(ColorBox::colorIndex(c->pen().color()))+"\t";
		s+=QString::number(c->pen().style()-1)+"\t";
		s+=QString::number(c->pen().width())+"\t";

		const QwtSymbol symbol = c->symbol();
		s+=QString::number(symbol.size().width())+"\t";
		s+=QString::number(SymbolBox::symbolIndex(symbol.style()))+"\t";
		s+=QString::number(ColorBox::colorIndex(symbol.pen().color()))+"\t";
		if (symbol.brush().style() != Qt::NoBrush)
			s+=QString::number(ColorBox::colorIndex(symbol.brush().color()))+"\t";
		else
			s+=QString::number(-1)+"\t";

		bool filled = c->brush().style() == Qt::NoBrush ? false : true;
		s+=QString::number(filled)+"\t";

		s+=QString::number(ColorBox::colorIndex(c->brush().color()))+"\t";
		s+=QString::number(PatternBox::patternIndex(c->brush().style()))+"\t";
		if (style <= LineSymbols || style == Box)
			s+=QString::number(symbol.pen().width())+"\t";
	}

	if(style == VerticalBars||style == HorizontalBars||style == Histogram)
	{
		BarCurve *b = (BarCurve*)c;
		s+=QString::number(b->gap())+"\t";
		s+=QString::number(b->offset())+"\t";
	}

	if(style == Histogram)
	{
		HistogramCurve *h = (HistogramCurve*)c;
		s+=QString::number(h->autoBinning())+"\t";
		s+=QString::number(h->binSize())+"\t";
		s+=QString::number(h->begin())+"\t";
		s+=QString::number(h->end())+"\t";
	}
	else if(style == VectXYXY || style == VectXYAM)
	{
		VectorCurve *v = (VectorCurve*)c;
		s+=v->color().name()+"\t";
		s+=QString::number(v->width())+"\t";
		s+=QString::number(v->headLength())+"\t";
		s+=QString::number(v->headAngle())+"\t";
		s+=QString::number(v->filledArrowHead())+"\t";

		QStringList colsList = v->plotAssociation().split(",", QString::SkipEmptyParts);
		s+=colsList[2].remove("(X)").remove("(A)")+"\t";
		s+=colsList[3].remove("(Y)").remove("(M)");
		if (style == VectXYAM)
			s+="\t"+QString::number(v->position());
		s+="\t";
	}
	else if(style == Box)
	{
		BoxCurve *b = (BoxCurve*)c;
		s+=QString::number(SymbolBox::symbolIndex(b->maxStyle()))+"\t";
		s+=QString::number(SymbolBox::symbolIndex(b->p99Style()))+"\t";
		s+=QString::number(SymbolBox::symbolIndex(b->meanStyle()))+"\t";
		s+=QString::number(SymbolBox::symbolIndex(b->p1Style()))+"\t";
		s+=QString::number(SymbolBox::symbolIndex(b->minStyle()))+"\t";
		s+=QString::number(b->boxStyle())+"\t";
		s+=QString::number(b->boxWidth())+"\t";
		s+=QString::number(b->boxRangeType())+"\t";
		s+=QString::number(b->boxRange())+"\t";
		s+=QString::number(b->whiskersRangeType())+"\t";
		s+=QString::number(b->whiskersRange())+"\t";
	}

	return s;
}

QString Layer::saveCurves()
{
	QString s;
	if (isPiePlot())
		s+=savePieCurveLayout();
	else
	{
		for (int i=0; i<n_curves; i++)
		{
			QwtPlotItem *it = plotItem(i);
			if (!it)
  	        	continue;

            if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
            {
                s += ((Spectrogram *)it)->saveToString();
                continue;
            }

            DataCurve *c = (DataCurve *)it;
			if (c->type() != ErrorBars)
			{
				if (c->type() == Function)
					s += ((FunctionCurve *)c)->saveToString();
				else if (c->type() == Box)
					s += "curve\t" + QString::number(c->x(0)) + "\t" + c->title().text() + "\t";
				else
					s += "curve\t" + c->xColumnName() + "\t" + c->title().text() + "\t";

				s += saveCurveLayout(i);
				s += QString::number(c->xAxis())+"\t"+QString::number(c->yAxis())+"\t";
				s += QString::number(c->startRow())+"\t"+QString::number(c->endRow())+"\t";
				s += QString::number(c->isVisible())+"\n";
			}
		    else if (c->type() == ErrorBars)
  	        {
  	        	ErrorCurve *er = (ErrorCurve *)it;
  	            s += "ErrorBars\t";
  	            s += QString::number(er->direction())+"\t";
  	            s += er->masterCurve()->xColumnName() + "\t";
  	            s += er->masterCurve()->title().text() + "\t";
  	            s += er->title().text() + "\t";
  	            s += QString::number(er->width())+"\t";
  	            s += QString::number(er->capLength())+"\t";
  	            s += er->color().name()+"\t";
  	            s += QString::number(er->throughSymbol())+"\t";
  	            s += QString::number(er->plusSide())+"\t";
  	            s += QString::number(er->minusSide())+"\n";
  	       }
		}
	}
	return s;
}

QString Layer::saveGridOptions()
{
	QString s="grid\t";
	s+=QString::number(grid.majorOnX)+"\t";
	s+=QString::number(grid.minorOnX)+"\t";
	s+=QString::number(grid.majorOnY)+"\t";
	s+=QString::number(grid.minorOnY)+"\t";
	s+=QString::number(grid.majorCol)+"\t";
	s+=QString::number(grid.majorStyle)+"\t";
	s+=QString::number(grid.majorWidth)+"\t";
	s+=QString::number(grid.minorCol)+"\t";
	s+=QString::number(grid.minorStyle)+"\t";
	s+=QString::number(grid.minorWidth)+"\t";
	s+=QString::number(grid.xZeroOn)+"\t";
	s+=QString::number(grid.yZeroOn)+"\t";
	s+=QString::number(grid.xAxis)+"\t";
	s+=QString::number(grid.yAxis)+"\n";
	return s;
}

TextEnrichment* Layer::newLegend()
{
	TextEnrichment* mrk = new TextEnrichment(m_plot);
	mrk->setOrigin(QPoint(10, 10));

	if (isPiePlot())
		mrk->setText(pieLegendText());
	else
		mrk->setText(legendText());

	mrk->setFrameStyle(defaultMarkerFrame);
	mrk->setFont(defaultMarkerFont);
	mrk->setTextColor(defaultTextMarkerColor);
	mrk->setBackgroundColor(defaultTextMarkerBackground);

	legendMarkerID = m_plot->insertMarker(mrk);
	int texts = m_texts.size();
	m_texts.resize(++texts);
	m_texts[texts-1] = legendMarkerID;

	emit modified();
	m_plot->replot();
	return mrk;
}

void Layer::addTimeStamp()
{
	TextEnrichment* mrk= newLegend(QDateTime::currentDateTime().toString(Qt::LocalDate));
	mrk->setOrigin(QPoint(m_plot->canvas()->width()/2, 10));
	emit modified();
	m_plot->replot();
}

TextEnrichment* Layer::newLegend(const QString& text)
{
	TextEnrichment* mrk = new TextEnrichment(m_plot);
	selectedMarker = m_plot->insertMarker(mrk);
	if(m_markers_selector)
		delete m_markers_selector;

	int texts = m_texts.size();
	m_texts.resize(++texts);
	m_texts[texts-1] = selectedMarker;

	mrk->setOrigin(QPoint(5,5));
	mrk->setText(text);
	mrk->setFrameStyle(defaultMarkerFrame);
	mrk->setFont(defaultMarkerFont);
	mrk->setTextColor(defaultTextMarkerColor);
	mrk->setBackgroundColor(defaultTextMarkerBackground);
	return mrk;
}

void Layer::insertLegend(const QStringList& lst, int fileVersion)
{
	legendMarkerID = insertTextMarker(lst, fileVersion);
}

long Layer::insertTextMarker(const QStringList& list, int fileVersion)
{
	QStringList fList=list;
	TextEnrichment* mrk = new TextEnrichment(m_plot);
	long key = m_plot->insertMarker(mrk);

	int texts = m_texts.size();
	m_texts.resize(++texts);
	m_texts[texts-1] = key;

	if (fileVersion < 86)
		mrk->setOrigin(QPoint(fList[1].toInt(),fList[2].toInt()));
	else
		mrk->setOriginCoord(fList[1].toDouble(), fList[2].toDouble());

	QFont fnt=QFont (fList[3],fList[4].toInt(),fList[5].toInt(),fList[6].toInt());
	fnt.setUnderline(fList[7].toInt());
	fnt.setStrikeOut(fList[8].toInt());
	mrk->setFont(fnt);

	mrk->setAngle(fList[11].toInt());

    QString text = QString();
	if (fileVersion < 71)
	{
		int bkg=fList[10].toInt();
		if (bkg <= 2)
			mrk->setFrameStyle(bkg);
		else if (bkg == 3)
		{
			mrk->setFrameStyle(0);
			mrk->setBackgroundColor(QColor(255, 255, 255));
		}
		else if (bkg == 4)
		{
			mrk->setFrameStyle(0);
			mrk->setBackgroundColor(QColor(Qt::black));
		}

		int n =(int)fList.count();
		for (int i=0;i<n-12;i++)
			text += fList[12+i]+"\n";
	}
	else if (fileVersion < 90)
	{
		mrk->setTextColor(QColor(fList[9]));
		mrk->setFrameStyle(fList[10].toInt());
		mrk->setBackgroundColor(QColor(fList[12]));

		int n=(int)fList.count();
		for (int i=0;i<n-13;i++)
			text += fList[13+i]+"\n";
	}
	else
	{
		mrk->setTextColor(QColor(fList[9]));
		mrk->setFrameStyle(fList[10].toInt());
		QColor c = QColor(fList[12]);
		c.setAlpha(fList[13].toInt());
		mrk->setBackgroundColor(c);

		int n = (int)fList.count();
		for (int i=0; i<n-14; i++)
			text += fList[14+i]+"\n";
	}
    mrk->setText(text.trimmed());
	return key;
}

void Layer::addArrow(QStringList list, int fileVersion)
{
	LineEnrichment* mrk= new LineEnrichment();
	long mrkID=m_plot->insertMarker(mrk);
    int linesOnPlot = (int)m_lines.size();
	m_lines.resize(++linesOnPlot);
	m_lines[linesOnPlot-1]=mrkID;

	if (fileVersion < 86)
	{
		mrk->setStartPoint(QPoint(list[1].toInt(), list[2].toInt()));
		mrk->setEndPoint(QPoint(list[3].toInt(), list[4].toInt()));
	}
	else
		mrk->setBoundingRect(list[1].toDouble(), list[2].toDouble(),
							list[3].toDouble(), list[4].toDouble());

	mrk->setWidth(list[5].toInt());
	mrk->setColor(QColor(list[6]));
	mrk->setStyle(getPenStyle(list[7]));
	mrk->drawEndArrow(list[8]=="1");
	mrk->drawStartArrow(list[9]=="1");
	if (list.count()>10)
	{
		mrk->setHeadLength(list[10].toInt());
		mrk->setHeadAngle(list[11].toInt());
		mrk->fillArrowHead(list[12]=="1");
	}
}

void Layer::addArrow(LineEnrichment* mrk)
{
	LineEnrichment* aux= new LineEnrichment();
    int linesOnPlot = (int)m_lines.size();
	m_lines.resize(++linesOnPlot);
	m_lines[linesOnPlot-1] = m_plot->insertMarker(aux);

	aux->setBoundingRect(mrk->startPointCoord().x(), mrk->startPointCoord().y(),
						 mrk->endPointCoord().x(), mrk->endPointCoord().y());
	aux->setWidth(mrk->width());
	aux->setColor(mrk->color());
	aux->setStyle(mrk->style());
	aux->drawEndArrow(mrk->hasEndArrow());
	aux->drawStartArrow(mrk->hasStartArrow());
	aux->setHeadLength(mrk->headLength());
	aux->setHeadAngle(mrk->headAngle());
	aux->fillArrowHead(mrk->filledArrowHead());
}

LineEnrichment* Layer::arrow(long id)
{
	return (LineEnrichment*)m_plot->marker(id);
}

ImageEnrichment* Layer::imageMarker(long id)
{
	return (ImageEnrichment*)m_plot->marker(id);
}

TextEnrichment* Layer::textMarker(long id)
{
	return (TextEnrichment*)m_plot->marker(id);
}

long Layer::insertTextMarker(TextEnrichment* mrk)
{
	TextEnrichment* aux = new TextEnrichment(m_plot);
	selectedMarker = m_plot->insertMarker(aux);
	if(m_markers_selector)
		delete m_markers_selector;

	int texts = m_texts.size();
	m_texts.resize(++texts);
	m_texts[texts-1] = selectedMarker;

	aux->setTextColor(mrk->textColor());
	aux->setBackgroundColor(mrk->backgroundColor());
	aux->setOriginCoord(mrk->xValue(), mrk->yValue());
	aux->setFont(mrk->font());
	aux->setFrameStyle(mrk->frameStyle());
	aux->setAngle(mrk->angle());
	aux->setText(mrk->text());
	return selectedMarker;
}

QString Layer::saveMarkers()
{
	QString s;
	int t = m_texts.size(), l = m_lines.size(), im = m_images.size();
	for (int i=0; i<im; i++)
	{
		ImageEnrichment* mrkI=(ImageEnrichment*) m_plot->marker(m_images[i]);
		s += "<image>\t";
		s += mrkI->fileName()+"\t";
		s += QString::number(mrkI->xValue(), 'g', 15)+"\t";
		s += QString::number(mrkI->yValue(), 'g', 15)+"\t";
		s += QString::number(mrkI->right(), 'g', 15)+"\t";
		s += QString::number(mrkI->bottom(), 'g', 15)+"</image>\n";
	}

	for (int i=0; i<l; i++)
	{
		LineEnrichment* mrkL=(LineEnrichment*) m_plot->marker(m_lines[i]);
		s+="<line>\t";

		QwtDoublePoint sp = mrkL->startPointCoord();
		s+=(QString::number(sp.x(), 'g', 15))+"\t";
		s+=(QString::number(sp.y(), 'g', 15))+"\t";

		QwtDoublePoint ep = mrkL->endPointCoord();
		s+=(QString::number(ep.x(), 'g', 15))+"\t";
		s+=(QString::number(ep.y(), 'g', 15))+"\t";

		s+=QString::number(mrkL->width())+"\t";
		s+=mrkL->color().name()+"\t";
		s+=penStyleName(mrkL->style())+"\t";
		s+=QString::number(mrkL->hasEndArrow())+"\t";
		s+=QString::number(mrkL->hasStartArrow())+"\t";
		s+=QString::number(mrkL->headLength())+"\t";
		s+=QString::number(mrkL->headAngle())+"\t";
		s+=QString::number(mrkL->filledArrowHead())+"</line>\n";
	}

	for (int i=0; i<t; i++)
	{
		TextEnrichment* mrk=(TextEnrichment*) m_plot->marker(m_texts[i]);
		if (m_texts[i] != legendMarkerID)
			s+="<text>\t";
		else
			s+="<legend>\t";

		s+=QString::number(mrk->xValue(), 'g', 15)+"\t";
		s+=QString::number(mrk->yValue(), 'g', 15)+"\t";

		QFont f=mrk->font();
		s+=f.family()+"\t";
		s+=QString::number(f.pointSize())+"\t";
		s+=QString::number(f.weight())+"\t";
		s+=QString::number(f.italic())+"\t";
		s+=QString::number(f.underline())+"\t";
		s+=QString::number(f.strikeOut())+"\t";
		s+=mrk->textColor().name()+"\t";
		s+=QString::number(mrk->frameStyle())+"\t";
		s+=QString::number(mrk->angle())+"\t";
		s+=mrk->backgroundColor().name()+"\t";
		s+=QString::number(mrk->backgroundColor().alpha())+"\t";

		QStringList textList=mrk->text().split("\n", QString::KeepEmptyParts);
		s+=textList.join ("\t");
		if (m_texts[i]!=legendMarkerID)
  	        s+="</text>\n";
  	    else
  	        s+="</legend>\n";
	}
	return s;
}

double Layer::selectedXStartValue()
{
	if (m_range_selector)
		return m_range_selector->minXValue();
	else
		return 0;
}

double Layer::selectedXEndValue()
{
	if (m_range_selector)
		return m_range_selector->maxXValue();
	else
		return 0;
}

QwtPlotItem* Layer::plotItem(int index)
{
    if (!n_curves || index >= n_curves || index < 0)
		return 0;

	return m_plot->plotItem(c_keys[index]);
}

int Layer::plotItemIndex(QwtPlotItem *it) const
{
	for (int i = 0; i < n_curves; i++)
	{
		if (m_plot->plotItem(c_keys[i]) == it)
			return i;
	}
	return -1;
}

QwtPlotCurve *Layer::curve(int index)
{
	if (!n_curves || index >= n_curves || index < 0)
		return 0;

	return m_plot->curve(c_keys[index]);
}

int Layer::curveIndex(QwtPlotCurve *c) const
{
	return plotItemIndex(c);
}

int Layer::range(int index, double *start, double *end)
{
	if (m_range_selector && m_range_selector->selectedCurve() == curve(index)) {
		*start = m_range_selector->minXValue();
		*end = m_range_selector->maxXValue();
		return m_range_selector->dataSize();
	} else {
		QwtPlotCurve *c = curve(index);
		if (!c)
			return 0;

		*start = c->x(0);
		*end = c->x(c->dataSize() - 1);
		return c->dataSize();
	}
}

CurveLayout Layer::initCurveLayout()
{
	CurveLayout cl;
	cl.connectType=1;
	cl.lStyle=0;
	cl.lWidth=1;
	cl.sSize=7;
	cl.sType=0;
	cl.filledArea=0;
	cl.aCol=0;
	cl.aStyle=0;
	cl.lCol=0;
	cl.penWidth = 1;
	cl.symCol=0;
	cl.fillCol=0;
	return cl;
}

CurveLayout Layer::initCurveLayout(int style, int curves)
{
    int i = n_curves - 1;

	CurveLayout cl = initCurveLayout();
	int color;
	guessUniqueCurveLayout(color, cl.sType);

  	cl.lCol = color;
  	cl.symCol = color;
  	cl.fillCol = color;

	if (style == Layer::Line)
		cl.sType = 0;
	else if (style == Layer::VerticalDropLines)
		cl.connectType=2;
	else if (style == Layer::HorizontalSteps || style == Layer::VerticalSteps)
	{
		cl.connectType=3;
		cl.sType = 0;
	}
	else if (style == Layer::Spline)
		cl.connectType=5;
	else if (curves && (style == Layer::VerticalBars || style == Layer::HorizontalBars))
	{
		cl.filledArea=1;
		cl.lCol=0;//black color pen
		cl.aCol=i+1;
		cl.sType = 0;
		if (c_type[i] == Layer::VerticalBars || style == Layer::HorizontalBars)
		{
			BarCurve *b = (BarCurve*)curve(i);
			if (b)
			{
				b->setGap(qRound(100*(1-1.0/(double)curves)));
				b->setOffset(-50*(curves-1) + i*100);
			}
		}
	}
	else if (style == Layer::Histogram)
	{
		cl.filledArea=1;
		cl.lCol=i+1;//start with red color pen
		cl.aCol=i+1; //start with red fill color
		cl.aStyle=4;
		cl.sType = 0;
	}
	else if (style== Layer::Area)
	{
		cl.filledArea=1;
		cl.aCol= color;
		cl.sType = 0;
	}
	return cl;
}

bool Layer::canConvertTo(QwtPlotCurve *c, CurveType type)
{
	if (!c) return false;
	// conversion between VectXYXY and VectXYAM is possible, but not implemented
	if (dynamic_cast<VectorCurve*>(c))
		return false;
	// conversion between Pie, Histogram and Box should be possible (all of them take one input column),
	// but lots of special-casing in ApplicationWindow and Layer makes this very difficult
	if (dynamic_cast<PieCurve*>(c) || dynamic_cast<HistogramCurve*>(c) || dynamic_cast<BoxCurve*>(c))
		return false;
	// converting error bars doesn't make sense
	if (dynamic_cast<ErrorCurve*>(c))
		return false;
	// line/symbol, area and bar curves can be converted to each other
	if (dynamic_cast<DataCurve*>(c))
		return
			type == Line || type == Scatter || type == LineSymbols || 
			type == VerticalBars || type == HorizontalBars ||
			type == HorizontalSteps || type == VerticalSteps ||
			type == Area || type == VerticalDropLines ||
			type == Spline;
	return false;
}

void Layer::setCurveType(int curve_index, CurveType type, bool update)
{
	if (type == c_type[curve_index]) return;
	c_type[curve_index] = type;
	if (!update) return;

	DataCurve *c = static_cast<DataCurve*>(curve(curve_index));
	insertCurve(c->table(), c->xColumnName(), c->yColumnName() , type, c->startRow(), c->endRow());
	removeCurve(curve_index);
	CurveLayout cl = initCurveLayout(type, 1);
	updateCurveLayout(curve_index, &cl);
	updatePlot();
}

void Layer::updateCurveLayout(int index, const CurveLayout *cL)
{
	DataCurve *c = (DataCurve *)this->curve(index);
	if (!c)
		return;
    if (c_type.isEmpty() || (int)c_type.size() < index)
        return;

	QPen pen = QPen(ColorBox::color(cL->symCol),cL->penWidth, Qt::SolidLine);
	if (cL->fillCol != -1)
		c->setSymbol(QwtSymbol(SymbolBox::style(cL->sType), QBrush(ColorBox::color(cL->fillCol)), pen, QSize(cL->sSize,cL->sSize)));
	else
		c->setSymbol(QwtSymbol(SymbolBox::style(cL->sType), QBrush(), pen, QSize(cL->sSize,cL->sSize)));

	c->setPen(QPen(ColorBox::color(cL->lCol), cL->lWidth, getPenStyle(cL->lStyle)));

	int style = c_type[index];
	if (style == Scatter)
		c->setStyle(QwtPlotCurve::NoCurve);
	else if (style == Spline)
	{
		c->setStyle(QwtPlotCurve::Lines);
		c->setCurveAttribute(QwtPlotCurve::Fitted, true);
	}
	else if (style == VerticalSteps)
	{
		c->setStyle(QwtPlotCurve::Steps);
		c->setCurveAttribute(QwtPlotCurve::Inverted, true);
	}
	else
		c->setStyle((QwtPlotCurve::CurveStyle)cL->connectType);

	QBrush brush = QBrush(ColorBox::color(cL->aCol));
	if (cL->filledArea)
		brush.setStyle(getBrushStyle(cL->aStyle));
	else
		brush.setStyle(Qt::NoBrush);
	c->setBrush(brush);
}

void Layer::updateErrorBars(ErrorCurve *er, bool xErr, int width, int cap, const QColor& c,
		bool plus, bool minus, bool through)
{
	if (!er)
		return;

	if (er->width() == width &&
			er->capLength() == cap &&
			er->color() == c &&
			er->plusSide() == plus &&
			er->minusSide() == minus &&
			er->throughSymbol() == through &&
			er->xErrors() == xErr)
		return;

	er->setWidth(width);
	er->setCapLength(cap);
	er->setColor(c);
	er->setXErrors(xErr);
	er->drawThroughSymbol(through);
	er->drawPlusSide(plus);
	er->drawMinusSide(minus);

	m_plot->replot();
	emit modified();
}

bool Layer::addErrorBars(const QString& yColName, Table *errTable, const QString& errColName,
		int type, int width, int cap, const QColor& color, bool through, bool minus, bool plus)
{
	QList<int> keys = m_plot->curveKeys();
	for (int i = 0; i<n_curves; i++ )
	{
		DataCurve *c = (DataCurve *)m_plot->curve(keys[i]);
		if (c && c->title().text() == yColName && c_type[i] != ErrorBars)
		{
			return addErrorBars(c->xColumnName(), yColName, errTable, errColName,
					type, width, cap, color, through, minus, plus);
		}
	}
	return false;
}

bool Layer::addErrorBars(const QString& xColName, const QString& yColName,
		Table *errTable, const QString& errColName, int type, int width, int cap,
		const QColor& color, bool through, bool minus, bool plus)
{
	DataCurve *master_curve = masterCurve(xColName, yColName);
	if (!master_curve)
		return false;

	ErrorCurve *er = new ErrorCurve(type, errTable, errColName);
	er->setMasterCurve(master_curve);
	er->setCapLength(cap);
	er->setColor(color);
	er->setWidth(width);
	er->drawPlusSide(plus);
	er->drawMinusSide(minus);
	er->drawThroughSymbol(through);

	c_type.resize(++n_curves);
	c_type[n_curves-1] = ErrorBars;

	c_keys.resize(n_curves);
	c_keys[n_curves-1] = m_plot->insertCurve(er);

	updatePlot();
	return true;
}

void Layer::plotPie(Table* w, const QString& name, const QPen& pen, int brush,
					int size, int firstColor, int startRow, int endRow, bool visible)
{
	if (endRow < 0)
		endRow = w->rowCount() - 1;

	PieCurve *pieCurve = new PieCurve(w, name, startRow, endRow);
	pieCurve->loadData();
	pieCurve->setPen(pen);
	pieCurve->setRay(size);
	pieCurve->setFirstColor(firstColor);
	pieCurve->setBrushStyle(getBrushStyle(brush));
	pieCurve->setVisible(visible);

	c_keys.resize(++n_curves);
	c_keys[n_curves-1] = m_plot->insertCurve(pieCurve);

	c_type.resize(n_curves);
	c_type[n_curves-1] = Pie;
}

void Layer::plotPie(Table* w, const QString& name, int startRow, int endRow)
{
	int ycol = w->colIndex(name);
	int size = 0;
	double sum = 0.0;

	if (endRow < 0)
		endRow = w->rowCount() - 1;

	for (int i=0; i<QwtPlot::axisCnt; i++)
		m_plot->enableAxis(i, false);
	scalePicker->refresh();

	m_plot->setTitle(QString::null);

	static_cast<QwtPlotCanvas*>(m_plot->canvas())->setLineWidth(1);

	QVarLengthArray<double> Y(abs(endRow - startRow) + 1);
	for (int i = startRow; i<= endRow; i++){
		QString yval = w->text(i,ycol);
		if (!yval.isEmpty()){
		    bool valid_data = true;
			Y[size] = QLocale().toDouble(yval, &valid_data);
			if (valid_data){
                sum += Y[size];
                size++;
			}
		}
	}
	if (!size)
		return;
    Y.resize(size);

	PieCurve *pieCurve = new PieCurve(w, name, startRow, endRow);
	pieCurve->setData(Y.data(), Y.data(), size);

	c_keys.resize(++n_curves);
	c_keys[n_curves-1] = m_plot->insertCurve(pieCurve);

	c_type.resize(n_curves);
	c_type[n_curves-1] = Pie;

	// This has to be synced with PieCurve::drawPie() for now... until we have a clean solution.
	QRect canvas_rect = m_plot->plotLayout()->canvasRect();
	float radius = 0.45*qMin(canvas_rect.width(), canvas_rect.height());

	double PI = 4*atan(1.0);
	double angle = 90;

	for (int i = 0; i<size; i++ ){
		const double value = Y[i]/sum*360;
		double alabel = (angle - value*0.5)*PI/180.0;

		TextEnrichment* aux = new TextEnrichment(m_plot);
		aux->setFrameStyle(0);
		aux->setText(QString::number(Y[i]/sum*100,'g',2)+"%");

		int texts = m_texts.size();
		m_texts.resize(++texts);
		m_texts[texts-1] = m_plot->insertMarker(aux);

		aux->setOrigin(canvas_rect.center() +
				QPoint(radius*cos(alabel) - 0.5*aux->rect().width(),
					-radius*sin(alabel) - 0.5*aux->rect().height()));

		angle -= value;
	}

	if (legendMarkerID>=0){
		TextEnrichment* mrk=(TextEnrichment*) m_plot->marker(legendMarkerID);
		if (mrk){
			QString text="";
			for (int i=0; i<size; i++){
				text+="\\p{";
				text+=QString::number(i+1);
				text+="} ";
				text+=QString::number(i+1);
				text+="\n";
			}
			mrk->setText(text);
		}
	}

	m_plot->replot();
	updateScale();
}

void Layer::insertPlotItem(QwtPlotItem *i, int type)
{
	c_type.resize(++n_curves);
	c_type[n_curves-1] = type;

	c_keys.resize(n_curves);
	c_keys[n_curves-1] = m_plot->insertCurve(i);

	if (i->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
  		addLegendItem(i->title().text());
}

bool Layer::insertCurvesList(Table* w, const QStringList& names, int style, int lWidth,
							int sSize, int startRow, int endRow)
{
	if (style==Layer::Pie)
		plotPie(w, names[0], startRow, endRow);
	else if (style == Box)
		plotBoxDiagram(w, names, startRow, endRow);
	else if (style==Layer::VectXYXY || style==Layer::VectXYAM)
		plotVectorCurve(w, names, style, startRow, endRow);
	else
	{
		int curves = (int)names.count();
        int errCurves = 0;
		QStringList lst = QStringList();
        for (int i=0; i<curves; i++)
        {//We rearrange the list so that the error bars are placed at the end
        	int j = w->colIndex(names[i]);
  	        if (w->plotDesignation(j) == SciDAVis::xErr || w->plotDesignation(j) == SciDAVis::yErr)
			{
				errCurves++;
				lst << names[i];
			}
			else
				lst.prepend(names[i]);
        }

		for (int i=0; i<curves; i++)
		{
            int j = w->colIndex(names[i]);
            bool ok = false;
            if (w->plotDesignation(j) == SciDAVis::xErr || w->plotDesignation(j) == SciDAVis::yErr)
			{
				int ycol = w->colY(w->colIndex(names[i]));
                if (ycol < 0)
                    return false;

                if (w->plotDesignation(j) == SciDAVis::xErr)
                    ok = addErrorBars(w->colName(ycol), w, names[i], (int)ErrorCurve::Horizontal);
                else
                    ok = addErrorBars(w->colName(ycol), w, names[i]);
			}
			else
                ok = insertCurve(w, names[i], style, startRow, endRow);

            if (ok)
			{
				CurveLayout cl = initCurveLayout(style, curves - errCurves);
				cl.sSize = sSize;
				cl.lWidth = lWidth;
				updateCurveLayout(i, &cl);
			}
			else
                return false;
		}
	}
	updatePlot();
	return true;
}

bool Layer::insertCurve(Table* w, const QString& name, int style, int startRow, int endRow)
{//provided for convenience
	int ycol = w->colIndex(name);
	int xcol = w->colX(ycol);

	bool succes = insertCurve(w, w->colName(xcol), w->colName(ycol), style, startRow, endRow);
	if (succes)
		emit modified();
	return succes;
}

bool Layer::insertCurve(Table* w, int xcol, const QString& name, int style)
{
	return insertCurve(w, w->colName(xcol), w->colName(w->colIndex(name)), style);
}

bool Layer::insertCurve(Table* w, const QString& xColName, const QString& yColName, int style, int startRow, int endRow)
{
	int xcol=w->colIndex(xColName);
	int ycol=w->colIndex(yColName);
	if (xcol < 0 || ycol < 0)
		return false;

	int xColType = w->columnType(xcol);
	int yColType = w->columnType(ycol);
	int i, size=0;
	QString date_time_fmt = w->columnFormat(xcol);
	QStringList xLabels, yLabels;// store text labels
	QTime time0;
	QDate date0;
    QLocale locale;

	if (endRow < 0)
		endRow = w->rowCount() - 1;

	int r = abs(endRow - startRow) + 1;
    QVector<double> X(r), Y(r);
	if (xColType == SciDAVis::Time){
		for (i = startRow; i<=endRow; i++ ){
			QString xval=w->text(i,xcol);
			if (!xval.isEmpty()){
				time0 = QTime::fromString (xval, date_time_fmt);
				if (time0.isValid())
					break;
			}
		}
	}
	else if (xColType == SciDAVis::Date){
		for (i = startRow; i<=endRow; i++ ){
			QString xval=w->text(i,xcol);
			if (!xval.isEmpty()){
				date0 = QDate::fromString(xval, date_time_fmt);
				if (date0.isValid())
					break;
			}
		}
	}

	for (i = startRow; i<=endRow; i++ ){
		QString xval=w->text(i,xcol);
		QString yval=w->text(i,ycol);
		if (!xval.isEmpty() && !yval.isEmpty()){
		    bool valid_data = true;
			if (xColType == SciDAVis::Text){
				if (xLabels.contains(xval) == 0)
					xLabels << xval;
				X[size] = (double)(xLabels.indexOf(xval)+1);
			}
			else if (xColType == SciDAVis::Time){
				QTime time = QTime::fromString (xval, date_time_fmt);
				if (time.isValid())
					X[size] = time0.msecsTo (time);
				else
					X[size] = 0;
			}
			else if (xColType == SciDAVis::Date){
				QDate d = QDate::fromString (xval, date_time_fmt);
				if (d.isValid())
					X[size] = (double) date0.daysTo(d);
			}
			else
                X[size] = QLocale().toDouble(xval, &valid_data);

			if (yColType == SciDAVis::Text){
				yLabels << yval;
				Y[size] = (double) (size + 1);
			}
			else
                Y[size] = QLocale().toDouble(yval, &valid_data);

            if (valid_data)
                size++;
		}
	}

	if (!size)
		return false;

	X.resize(size);
	Y.resize(size);

	DataCurve *c = 0;
	if (style == VerticalBars){
		c = new BarCurve(BarCurve::Vertical, w, xColName, yColName, startRow, endRow);
		c->setStyle(QwtPlotCurve::UserCurve);
	}
	else if (style == HorizontalBars){
		c = new BarCurve(BarCurve::Horizontal, w, xColName, yColName, startRow, endRow);
		c->setStyle(QwtPlotCurve::UserCurve);
	}
	else if (style == Histogram){
		c = new HistogramCurve(w, xColName, yColName, startRow, endRow);
		((HistogramCurve *)c)->initData(Y, size);
		c->setStyle(QwtPlotCurve::UserCurve);
	}
	else
		c = new DataCurve(w, xColName, yColName, startRow, endRow);

	c_type.resize(++n_curves);
	c_type[n_curves-1] = style;
	c_keys.resize(n_curves);
	c_keys[n_curves-1] = m_plot->insertCurve(c);

	c->setPen(QPen(Qt::black,widthLine));

	if (style == HorizontalBars)
		c->setData(Y.data(), X.data(), size);
	else if (style != Histogram)
		c->setData(X.data(), Y.data(), size);

	if (xColType == SciDAVis::Text ){
		if (style == HorizontalBars){
			axesFormatInfo[QwtPlot::yLeft] = xColName;
			axesFormatInfo[QwtPlot::yRight] = xColName;
			axisType[QwtPlot::yLeft] = Txt;
			m_plot->setAxisScaleDraw (QwtPlot::yLeft, new QwtTextScaleDraw(xLabels));
		}
		else{
			axesFormatInfo[QwtPlot::xBottom] = xColName;
			axesFormatInfo[QwtPlot::xTop] = xColName;
			axisType[QwtPlot::xBottom] = Txt;
			m_plot->setAxisScaleDraw (QwtPlot::xBottom, new QwtTextScaleDraw(xLabels));
		}
	}
	else if (xColType == SciDAVis::Time){
		QString fmtInfo = time0.toString() + ";" + date_time_fmt;
		if (style == HorizontalBars)
			setLabelsDateTimeFormat(QwtPlot::yLeft, Time, fmtInfo);
		else
			setLabelsDateTimeFormat(QwtPlot::xBottom, Time, fmtInfo);
	}
	else if (xColType == SciDAVis::Date ){
		QString fmtInfo = date0.toString(Qt::ISODate) + ";" + date_time_fmt;
		if (style == HorizontalBars)
			setLabelsDateTimeFormat(QwtPlot::yLeft, Date, fmtInfo);
		else
			setLabelsDateTimeFormat(QwtPlot::xBottom, Date, fmtInfo);
	}

	if (yColType == SciDAVis::Text){
		axesFormatInfo[QwtPlot::yLeft] = yColName;
		axesFormatInfo[QwtPlot::yRight] = yColName;
		axisType[QwtPlot::yLeft] = Txt;
		m_plot->setAxisScaleDraw (QwtPlot::yLeft, new QwtTextScaleDraw(yLabels));
	}

	addLegendItem(yColName);
	updatePlot();

	return true;
}

void Layer::plotVectorCurve(Table* w, const QStringList& colList, int style, int startRow, int endRow)
{
	if (colList.count() != 4)
		return;

	if (endRow < 0)
		endRow = w->rowCount() - 1;

	VectorCurve *v = 0;
	if (style == VectXYAM)
		v = new VectorCurve(VectorCurve::XYAM, w, colList[0], colList[1], colList[2], colList[3], startRow, endRow);
	else
		v = new VectorCurve(VectorCurve::XYXY, w, colList[0], colList[1], colList[2], colList[3], startRow, endRow);

	if (!v)
		return;

	v->loadData();
	v->setStyle(QwtPlotCurve::NoCurve);

	c_type.resize(++n_curves);
	c_type[n_curves-1] = style;

	c_keys.resize(n_curves);
	c_keys[n_curves-1] = m_plot->insertCurve(v);

	addLegendItem(colList[1]);
	updatePlot();
}

void Layer::updateVectorsLayout(int curve, const QColor& color, int width,
		int arrowLength, int arrowAngle, bool filled, int position,
		const QString& xEndColName, const QString& yEndColName)
{
	VectorCurve *vect = (VectorCurve *)this->curve(curve);
	if (!vect)
		return;

	vect->setColor(color);
	vect->setWidth(width);
	vect->setHeadLength(arrowLength);
	vect->setHeadAngle(arrowAngle);
	vect->fillArrowHead(filled);
	vect->setPosition(position);

	if (!xEndColName.isEmpty() && !yEndColName.isEmpty())
		vect->setVectorEnd(xEndColName, yEndColName);

	m_plot->replot();
	emit modified();
}

void Layer::updatePlot()
{
	if (autoscale && !zoomOn() && m_active_tool==NULL)	{
		for (int i = 0; i < QwtPlot::axisCnt; i++)
			m_plot->setAxisAutoScale(i);
	}

	m_plot->replot();
    updateMarkersBoundingRect();
	updateSecondaryAxis(QwtPlot::xTop);
	updateSecondaryAxis(QwtPlot::yRight);

    if (isPiePlot()){
        PieCurve *c = (PieCurve *)curve(0);
        c->updateBoundingRect();
    }

    m_plot->replot();
	m_zoomer[0]->setZoomBase();
	m_zoomer[1]->setZoomBase();
}

void Layer::updateScale()
{
	const QwtScaleDiv *scDiv=m_plot->axisScaleDiv(QwtPlot::xBottom);
	QwtValueList lst = scDiv->ticks (QwtScaleDiv::MajorTick);

	double step = fabs(lst[1]-lst[0]);

	if (!autoscale)
		m_plot->setAxisScale (QwtPlot::xBottom, scDiv->lBound(), scDiv->hBound(), step);

	scDiv=m_plot->axisScaleDiv(QwtPlot::yLeft);
	lst = scDiv->ticks (QwtScaleDiv::MajorTick);

	step = fabs(lst[1]-lst[0]);

	if (!autoscale)
		m_plot->setAxisScale (QwtPlot::yLeft, scDiv->lBound(), scDiv->hBound(), step);

	m_plot->replot();
	updateMarkersBoundingRect();
	updateSecondaryAxis(QwtPlot::xTop);
	updateSecondaryAxis(QwtPlot::yRight);

	m_plot->replot();//TODO: avoid 2nd replot!
}

void Layer::setBarsGap(int curve, int gapPercent, int offset)
{
	BarCurve *bars = (BarCurve *)this->curve(curve);
	if (!bars)
		return;

    if (bars->gap() == gapPercent && bars->offset() == offset)
        return;

	bars->setGap(gapPercent);
	bars->setOffset(offset);
}

void Layer::removePie()
{
	if (legendMarkerID>=0)
	{
		TextEnrichment* mrk=(TextEnrichment*) m_plot->marker(legendMarkerID);
		if (mrk)
			mrk->setText(QString::null);
	}

	m_plot->removeCurve(c_keys[0]);
	m_plot->replot();

	c_keys.resize(0);
	c_type.resize(0);
	n_curves=0;
	emit modified();
}

void Layer::removeCurves(const QString& s)
{
    QList<int> keys = m_plot->curveKeys();
	for (int i=0; i<(int)keys.count(); i++)
	{
		QwtPlotItem *it = m_plot->plotItem(keys[i]);
		if (!it)
            continue;

        if (it->title().text() == s)
        {
            removeCurve(i);
            continue;
        }

        if (it->rtti() != QwtPlotItem::Rtti_PlotCurve)
            continue;
        if (((PlotCurve *)it)->type() == Function)
            continue;

        if(((DataCurve *)it)->plotAssociation().contains(s))
            removeCurve(i);
	}
	m_plot->replot();
}

void Layer::removeCurve(const QString& s)
{
	removeCurve(plotItemsList().indexOf(s));
}

void Layer::removeCurve(int index)
{
	if (index < 0 || index >= n_curves)
		return;

	QwtPlotItem *it = plotItem(index);
	if (!it)
		return;

	removeLegendItem(index);

	if (it->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
	{
        if (((PlotCurve *)it)->type() == ErrorBars)
            ((ErrorCurve *)it)->detachFromMasterCurve();
		else if (((PlotCurve *)it)->type() != Function)
			((DataCurve *)it)->clearErrorBars();

		if (m_fit_curves.contains((QwtPlotCurve *)it))
		{
			int i = m_fit_curves.indexOf((QwtPlotCurve *)it);
			if (i >= 0 && i < m_fit_curves.size())
				m_fit_curves.removeAt(i);
		}
	}

    if (m_range_selector && curve(index) == m_range_selector->selectedCurve())
    {
        if (n_curves > 1 && (index - 1) >= 0)
            m_range_selector->setSelectedCurve(curve(index - 1));
        else if (n_curves > 1 && index + 1 < n_curves)
            m_range_selector->setSelectedCurve(curve(index + 1));
        else
            delete m_range_selector;
    }

	m_plot->removeCurve(c_keys[index]);
	n_curves--;

	for (int i=index; i<n_curves; i++)
    {
        c_type[i] = c_type[i+1];
        c_keys[i] = c_keys[i+1];
    }
    c_type.resize(n_curves);
    c_keys.resize(n_curves);
	emit modified();
}

void Layer::removeLegendItem(int index)
{
	if (legendMarkerID<0 || c_type[index] == ErrorBars)
		return;

	TextEnrichment* mrk=(TextEnrichment*) m_plot->marker(legendMarkerID);
	if (!mrk)
		return;

	if (isPiePlot())
	{
		mrk->setText(QString::null);
		return;
	}

	QString text=mrk->text();
	QStringList items=text.split( "\n", QString::SkipEmptyParts);

	if (index >= (int) items.count())
		return;

	QStringList l = items.filter( "\\c{" + QString::number(index+1) + "}" );
	items.remove(l[0]);//remove the corresponding legend string

	int cv=0;
	for (int i=0; i< (int)items.count(); i++)
	{//set new curves indexes in legend text
		QString item = (items[i]).trimmed();
		if (item.startsWith("\\c{", true))
		{
			item.remove(0, item.find("}", 0));
			item.prepend("\\c{"+QString::number(++cv));
		}
		items[i]=item;
	}
	text=items.join ( "\n" ) + "\n";
	mrk->setText(text);
}

void Layer::addLegendItem(const QString& colName)
{
	if (legendMarkerID >= 0 ){
		TextEnrichment* mrk=(TextEnrichment*) m_plot->marker(legendMarkerID);
		if (mrk){
			QString text = mrk->text();
			if (text.endsWith ( "\n", true ) )
				text.append("\\c{"+QString::number(n_curves)+"}"+colName+"\n");
			else
				text.append("\n\\c{"+QString::number(n_curves)+"}"+colName+"\n");

			mrk->setText(text);
		}
	}
}

void Layer::contextMenuEvent(QContextMenuEvent *e)
{
	if (selectedMarker>=0) {
		emit showMarkerPopupMenu();
		return;
	}

	QPoint pos = m_plot->canvas()->mapFrom(m_plot, e->pos());
	int dist, point;
	const long curve = m_plot->closestCurve(pos.x(), pos.y(), dist, point);
	const QwtPlotCurve *c = (QwtPlotCurve *)m_plot->curve(curve);

	if (c && dist < 10)//10 pixels tolerance
		emit showCurveContextMenu(curve);
	else
		emit showContextMenu();

	e->accept();
}

void Layer::closeEvent(QCloseEvent *e)
{
	emit closed();
	e->accept();

}

bool Layer::zoomOn()
{
	return (m_zoomer[0]->isEnabled() || m_zoomer[1]->isEnabled());
}

void Layer::zoomed (const QwtDoubleRect &)
{
	emit modified();
}

void Layer::zoom(bool on)
{
	m_zoomer[0]->setEnabled(on);
	m_zoomer[1]->setEnabled(on);
	for (int i=0; i<n_curves; i++)
  	{
  		Spectrogram *sp = (Spectrogram *)this->curve(i);
  	    if (sp && sp->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
  	    {
  	     	if (sp->colorScaleAxis() == QwtPlot::xBottom || sp->colorScaleAxis() == QwtPlot::yLeft)
  	        	m_zoomer[0]->setEnabled(false);
  	        else
  	        	m_zoomer[1]->setEnabled(false);
  	    }
  	}

	QCursor cursor=QCursor (QPixmap(":/lens.xpm"),-1,-1);
	if (on)
		m_plot->canvas()->setCursor(cursor);
	else
		m_plot->canvas()->setCursor(Qt::ArrowCursor);
}

void Layer::zoomOut()
{
	m_zoomer[0]->zoom(-1);
	m_zoomer[1]->zoom(-1);

	updateSecondaryAxis(QwtPlot::xTop);
  	updateSecondaryAxis(QwtPlot::yRight);
}

void Layer::drawText(bool on)
{
	QCursor c=QCursor(Qt::IBeamCursor);
	if (on)
		m_plot->canvas()->setCursor(c);
	else
		m_plot->canvas()->setCursor(Qt::ArrowCursor);

	drawTextOn=on;
}

ImageEnrichment* Layer::addImage(ImageEnrichment* mrk)
{
	if (!mrk)
		return 0;
	
	ImageEnrichment* mrk2 = new ImageEnrichment(mrk->fileName());

	int imagesOnPlot = m_images.size();
	m_images.resize(++imagesOnPlot);
	m_images[imagesOnPlot-1]=m_plot->insertMarker(mrk2);

	mrk2->setBoundingRect(mrk->xValue(), mrk->yValue(), mrk->right(), mrk->bottom());
	return mrk;
}

ImageEnrichment* Layer::addImage(const QString& fileName)
{
	if (fileName.isEmpty() || !QFile::exists(fileName)){
		QMessageBox::warning(0, tr("File open error"),
				tr("Image file: <p><b> %1 </b><p>does not exist anymore!").arg(fileName));
		return 0;
	}
	
	ImageEnrichment* mrk = new ImageEnrichment(fileName);
	int imagesOnPlot = m_images.size();
	m_images.resize(++imagesOnPlot);
	m_images[imagesOnPlot-1] = m_plot->insertMarker(mrk);

	QSize picSize = mrk->pixmap().size();
	int w = m_plot->canvas()->width();
	if (picSize.width()>w)
		picSize.setWidth(w);
	
	int h=m_plot->canvas()->height();
	if (picSize.height()>h)
		picSize.setHeight(h);

	mrk->setSize(picSize);
	m_plot->replot();

	emit modified();
	return mrk;
}

void Layer::insertImageEnrichment(const QStringList& lst, int fileVersion)
{
	QString fn = lst[1];
	if (!QFile::exists(fn)){
		QMessageBox::warning(0, tr("File open error"),
				tr("Image file: <p><b> %1 </b><p>does not exist anymore!").arg(fn));
	} else {
		ImageEnrichment* mrk = new ImageEnrichment(fn);
		if (!mrk)
			return;
		
        int imagesOnPlot = m_images.size();
		m_images.resize(++imagesOnPlot);
		m_images[imagesOnPlot-1] = m_plot->insertMarker(mrk);

		if (fileVersion < 86){
			mrk->setOrigin(QPoint(lst[2].toInt(), lst[3].toInt()));
			mrk->setSize(QSize(lst[4].toInt(), lst[5].toInt()));
		} else if (fileVersion < 90) {
		    double left = lst[2].toDouble();
		    double right = left + lst[4].toDouble();
		    double top = lst[3].toDouble();
		    double bottom = top - lst[5].toDouble();
			mrk->setBoundingRect(left, top, right, bottom);
		} else
			mrk->setBoundingRect(lst[2].toDouble(), lst[3].toDouble(), lst[4].toDouble(), lst[5].toDouble());
	}
}

void Layer::drawLine(bool on, bool arrow)
{
	drawLineOn=on;
	drawArrowOn=arrow;
	if (!on)
		emit drawLineEnded(true);
}

void Layer::modifyFunctionCurve(int curve, int type, const QStringList &formulas,
		const QString& var,QList<double> &ranges, int points)
{
	FunctionCurve *c = (FunctionCurve *)this->curve(curve);
	if (!c)
		return;

	if (c->functionType() == type &&
		c->variable() == var &&
		c->formulas() == formulas &&
		c->startRange() == ranges[0] &&
		c->endRange() == ranges[1] &&
		c->dataSize() == points)
		return;

	QString oldLegend = c->legend();

	c->setFunctionType((FunctionCurve::FunctionType)type);
	c->setRange(ranges[0], ranges[1]);
	c->setFormulas(formulas);
	c->setVariable(var);
	c->loadData(points);

	if (legendMarkerID >= 0)
	{//update the legend marker
		TextEnrichment* mrk=(TextEnrichment*) m_plot->marker(legendMarkerID);
		if (mrk)
		{
			QString text = (mrk->text()).replace(oldLegend, c->legend());
			mrk->setText(text);
		}
	}
	updatePlot();
	emit modified();
}

QString Layer::generateFunctionName(const QString& name)
{
    int index = 1;
  	QString newName = name + QString::number(index);

  	QStringList lst;
  	for (int i=0; i<n_curves; i++)
  	{
  		PlotCurve *c = (PlotCurve*)this->curve(i);
  		if (!c)
            continue;

  	    if (c->type() == Function)
  	    	lst << c->title().text();
	}

  	while(lst.contains(newName)){
  	        newName = name + QString::number(++index);}
  	return newName;
}

void Layer::addFunctionCurve(int type, const QStringList &formulas, const QString &var,
		QList<double> &ranges, int points, const QString& title)
{
	QString name;
	if (!title.isEmpty())
		name = title;
	else
		name = generateFunctionName();

	FunctionCurve *c = new FunctionCurve((const FunctionCurve::FunctionType)type, name);
	c->setPen(QPen(QColor(Qt::black), widthLine));
	c->setRange(ranges[0], ranges[1]);
	c->setFormulas(formulas);
	c->setVariable(var);
	c->loadData(points);

	c_type.resize(++n_curves);
	c_type[n_curves-1] = Line;

	c_keys.resize(n_curves);
	c_keys[n_curves-1] = m_plot->insertCurve(c);

	addLegendItem(c->legend());
	updatePlot();

	emit modified();
}

void Layer::insertFunctionCurve(const QString& formula, int points, int fileVersion)
{
	int type;
	QStringList formulas;
	QString var, name = QString::null;
	QList<double> ranges;

	QStringList curve = formula.split(",");
	if (fileVersion < 87)
	{
		if (curve[0][0]=='f')
		{
			type = FunctionCurve::Normal;
			formulas += curve[0].section('=',1,1);
			var = curve[1];
			ranges += curve[2].toDouble();
			ranges += curve[3].toDouble();
		}
		else if (curve[0][0]=='X')
		{
			type = FunctionCurve::Parametric;
			formulas += curve[0].section('=',1,1);
			formulas += curve[1].section('=',1,1);
			var = curve[2];
			ranges += curve[3].toDouble();
			ranges += curve[4].toDouble();
		}
		else if (curve[0][0]=='R')
		{
			type = FunctionCurve::Polar;
			formulas += curve[0].section('=',1,1);
			formulas += curve[1].section('=',1,1);
			var = curve[2];
			ranges += curve[3].toDouble();
			ranges += curve[4].toDouble();
		}
	}
	else
	{
		type = curve[0].toInt();
		name = curve[1];

		if (type == FunctionCurve::Normal)
		{
			formulas << curve[2];
			var = curve[3];
			ranges += curve[4].toDouble();
			ranges += curve[5].toDouble();
		}
		else if (type == FunctionCurve::Polar || type == FunctionCurve::Parametric)
		{
			formulas << curve[2];
			formulas << curve[3];
			var = curve[4];
			ranges += curve[5].toDouble();
			ranges += curve[6].toDouble();
		}
	}
	addFunctionCurve(type, formulas, var, ranges, points, name);
}

void Layer::createTable(const QString& curveName)
{
    if (curveName.isEmpty())
        return;

    const QwtPlotCurve* cv = curve(curveName);
    if (!cv)
        return;

    createTable(cv);
}

void Layer::createTable(const QwtPlotCurve* curve)
{
    if (!curve)
        return;

    int size = curve->dataSize();
    QString text = "1\t2\n";
    for (int i=0; i<size; i++)
	{
        text += QString::number(curve->x(i))+"\t";
        text += QString::number(curve->y(i))+"\n";
    }
    QString legend = tr("Data set generated from curve") + ": " + curve->title().text();
    emit createTable(tr("Table") + "1" + "\t" + legend, size, 2, text);
}

QString Layer::saveToString(bool saveAsTemplate)
{
	QString s="<graph>\n";
	s+="ggeometry\t";
	s+=QString::number(this->pos().x())+"\t";
	s+=QString::number(this->pos().y())+"\t";
	s+=QString::number(this->frameGeometry().width())+"\t";
	s+=QString::number(this->frameGeometry().height())+"\n";
	s+=saveTitle();
	s+="<Antialiasing>" + QString::number(m_antialiasing) + "</Antialiasing>\n";
	s+="Background\t" + m_plot->paletteBackgroundColor().name() + "\t";
	s+=QString::number(m_plot->paletteBackgroundColor().alpha()) + "\n";
	s+="Margin\t"+QString::number(m_plot->margin())+"\n";
	s+="Border\t"+QString::number(m_plot->lineWidth())+"\t"+m_plot->frameColor().name()+"\n";
	s+=saveGridOptions();
	s+=saveEnabledAxes();
	s+="AxesTitles\t"+saveScaleTitles();
	s+=saveAxesTitleColors();
	s+=saveAxesTitleAlignement();
	s+=saveFonts();
	s+=saveEnabledTickLabels();
	s+=saveAxesColors();
	s+=saveAxesBaseline();
	s+=saveCanvas();

    if (!saveAsTemplate)
	   s+=saveCurves();

	s+=saveScale();
	s+=saveAxesFormulas();
	s+=saveLabelsFormat();
	s+=saveAxesLabelsType();
	s+=saveTicksType();
	s+="TicksLength\t"+QString::number(minorTickLength())+"\t"+QString::number(majorTickLength())+"\n";
	s+="DrawAxesBackbone\t"+QString::number(drawAxesBackbone)+"\n";
	s+="AxesLineWidth\t"+QString::number(m_plot->axesLinewidth())+"\n";
	s+=saveLabelsRotation();
	s+=saveMarkers();
	s+="</graph>\n";
	return s;
}

void Layer::showIntensityTable()
{
	ImageEnrichment* mrk=(ImageEnrichment*) m_plot->marker(selectedMarker);
	if (!mrk)
		return;

	emit createIntensityTable(mrk->fileName());
}

void Layer::updateMarkersBoundingRect()
{
	for (int i=0;i<(int)m_lines.size();i++)
	{
		LineEnrichment* mrkL = (LineEnrichment*)m_plot->marker(m_lines[i]);
		if (mrkL)
			mrkL->updateBoundingRect();
	}
	for (int i=0; i<(int)m_texts.size(); i++)
	{
		TextEnrichment* mrkT = (TextEnrichment*) m_plot->marker(m_texts[i]);
		if (mrkT)
			mrkT->updateOrigin();
	}

	for (int i=0;i<(int)m_images.size();i++)
	{
		ImageEnrichment* mrk = (ImageEnrichment*) m_plot->marker(m_images[i]);
		if (mrk)
			mrk->updateBoundingRect();
	}
}

void Layer::resizeEvent ( QResizeEvent *e )
{
	if (ignoreResize || !this->isVisible())
		return;

	if (autoScaleFonts)
	{
		QSize oldSize=e->oldSize();
		QSize size=e->size();

		double ratio=(double)size.height()/(double)oldSize.height();
		scaleFonts(ratio);
	}

	m_plot->resize(e->size());
}

void Layer::scaleFonts(double factor)
{
	for (int i=0;i<(int)m_texts.size();i++)
	{
		TextEnrichment* mrk = (TextEnrichment*) m_plot->marker(m_texts[i]);
		QFont font = mrk->font();
		font.setPointSizeF(factor*font.pointSizeFloat());
		mrk->setFont(font);
	}
	for (int i = 0; i<QwtPlot::axisCnt; i++)
	{
		QFont font = axisFont(i);
		font.setPointSizeF(factor*font.pointSizeFloat());
		m_plot->setAxisFont(i, font);

		QwtText title = m_plot->axisTitle(i);
		font = title.font();
		font.setPointSizeF(factor*font.pointSizeFloat());
		title.setFont(font);
		m_plot->setAxisTitle(i, title);
	}

	QwtText title = m_plot->title();
	QFont font = title.font();
	font.setPointSizeF(factor*font.pointSizeFloat());
	title.setFont(font);
	m_plot->setTitle(title);

	m_plot->replot();
}

void Layer::setMargin (int d)
{
	if (m_plot->margin() == d)
		return;

	m_plot->setMargin(d);
	emit modified();
}

void Layer::setFrame (int width, const QColor& color)
{
	if (m_plot->frameColor() == color && width == m_plot->lineWidth())
		return;

	QPalette pal = m_plot->palette();
	pal.setColor(QPalette::WindowText, color);
	m_plot->setPalette(pal);

	m_plot->setLineWidth(width);
}

void Layer::setBackgroundColor(const QColor& color)
{
    QColorGroup cg;
	QPalette p = m_plot->palette();
	p.setColor(QPalette::Window, color);
    m_plot->setPalette(p);

    m_plot->setAutoFillBackground(true);
	emit modified();
}

void Layer::setCanvasBackground(const QColor& color)
{
	m_plot->setCanvasBackground(color);
	emit modified();
}

Qt::BrushStyle Layer::getBrushStyle(int style)
{
	Qt::BrushStyle brushStyle;
	switch (style)
	{
		case 0:
			brushStyle=Qt::SolidPattern;
			break;
		case 1:
			brushStyle=Qt::HorPattern;
			break;
		case 2:
			brushStyle=Qt::VerPattern;
			break;
		case 3:
			brushStyle=Qt::CrossPattern;
			break;
		case 4:
			brushStyle=Qt::BDiagPattern;
			break;
		case 5:
			brushStyle=Qt::FDiagPattern;
			break;
		case 6:
			brushStyle=Qt::DiagCrossPattern;
			break;
		case 7:
			brushStyle=Qt::Dense1Pattern;
			break;
		case 8:
			brushStyle=Qt::Dense2Pattern;
			break;
		case 9:
			brushStyle=Qt::Dense3Pattern;
			break;
		case 10:
			brushStyle=Qt::Dense4Pattern;
			break;
		case 11:
			brushStyle=Qt::Dense5Pattern;
			break;
		case 12:
			brushStyle=Qt::Dense6Pattern;
			break;
		case 13:
			brushStyle=Qt::Dense7Pattern;
			break;
	}
	return brushStyle;
}

QString Layer::penStyleName(Qt::PenStyle style)
{
	if (style==Qt::SolidLine)
		return "SolidLine";
	else if (style==Qt::DashLine)
		return "DashLine";
	else if (style==Qt::DotLine)
		return "DotLine";
	else if (style==Qt::DashDotLine)
		return "DashDotLine";
	else if (style==Qt::DashDotDotLine)
		return "DashDotDotLine";
	else
		return "SolidLine";
}

Qt::PenStyle Layer::getPenStyle(int style)
{
	Qt::PenStyle linePen;
	switch (style)
	{
		case 0:
			linePen=Qt::SolidLine;
			break;
		case 1:
			linePen=Qt::DashLine;
			break;
		case 2:
			linePen=Qt::DotLine;
			break;
		case 3:
			linePen=Qt::DashDotLine;
			break;
		case 4:
			linePen=Qt::DashDotDotLine;
			break;
	}
	return linePen;
}

Qt::PenStyle Layer::getPenStyle(const QString& s)
{
	Qt::PenStyle style;
	if (s=="SolidLine")
		style=Qt::SolidLine;
	else if (s=="DashLine")
		style=Qt::DashLine;
	else if (s=="DotLine")
		style=Qt::DotLine;
	else if (s=="DashDotLine")
		style=Qt::DashDotLine;
	else if (s=="DashDotDotLine")
		style=Qt::DashDotDotLine;
	return style;
}

int Layer::obsoleteSymbolStyle(int type)
{
	if (type <= 4)
		return type+1;
	else
		return type+2;
}

int Layer::curveType(int curveIndex)
{
	if (curveIndex < (int)c_type.size() && curveIndex >= 0)
		return c_type[curveIndex];
	else
		return -1;
}

void Layer::showPlotErrorMessage(QWidget *parent, const QStringList& emptyColumns)
{
	QApplication::restoreOverrideCursor();

	int n = (int)emptyColumns.count();
	if (n > 1)
	{
		QString columns;
		for (int i = 0; i < n; i++)
			columns += "<p><b>" + emptyColumns[i] + "</b></p>";

		QMessageBox::warning(parent, tr("Warning"),
				tr("The columns") + ": " + columns + tr("are empty and will not be added to the plot!"));
	}
	else if (n == 1)
		QMessageBox::warning(parent, tr("Warning"),
				tr("The column") + " <b>" + emptyColumns[0] + "</b> " + tr("is empty and will not be added to the plot!"));
}

void Layer::showTitleContextMenu()
{
	QMenu titleMenu(this);
	titleMenu.addAction(QPixmap(":/cut.xpm"), tr("&Cut"),this, SLOT(cutTitle()));
	titleMenu.addAction(QPixmap(":/copy.xpm"), tr("&Copy"),this, SLOT(copyTitle()));
	titleMenu.addAction(tr("&Delete"),this, SLOT(removeTitle()));
	titleMenu.addSeparator();
	titleMenu.addAction(tr("&Properties..."), this, SIGNAL(viewTitleDialog()));
	titleMenu.exec(QCursor::pos());
}

void Layer::cutTitle()
{
	QApplication::clipboard()->setText(m_plot->title().text(), QClipboard::Clipboard);
	removeTitle();
}

void Layer::copyTitle()
{
	QApplication::clipboard()->setText(m_plot->title().text(), QClipboard::Clipboard);
}

void Layer::removeAxisTitle()
{
	int axis = (selectedAxis + 2)%4;//unconsistent notation in Qwt enumerations between
  	//QwtScaleDraw::alignement and QwtPlot::Axis
  	m_plot->setAxisTitle(axis, QString::null);
	m_plot->replot();
	emit modified();
}

void Layer::cutAxisTitle()
{
	copyAxisTitle();
	removeAxisTitle();
}

void Layer::copyAxisTitle()
{
	int axis = (selectedAxis + 2)%4;//unconsistent notation in Qwt enumerations between
  	//QwtScaleDraw::alignement and QwtPlot::Axis
  	QApplication::clipboard()->setText(m_plot->axisTitle(axis).text(), QClipboard::Clipboard);
	}

void Layer::showAxisTitleMenu(int axis)
{
	selectedAxis = axis;

	QMenu titleMenu(this);
	titleMenu.addAction(QPixmap(":/cut.xpm"), tr("&Cut"), this, SLOT(cutAxisTitle()));
	titleMenu.addAction(QPixmap(":/copy.xpm"), tr("&Copy"), this, SLOT(copyAxisTitle()));
	titleMenu.addAction(tr("&Delete"),this, SLOT(removeAxisTitle()));
	titleMenu.addSeparator();
	switch (axis)
	{
		case QwtScaleDraw::BottomScale:
			titleMenu.addAction(tr("&Properties..."), this, SIGNAL(xAxisTitleDblClicked()));
			break;

		case QwtScaleDraw::LeftScale:
			titleMenu.addAction(tr("&Properties..."), this, SIGNAL(yAxisTitleDblClicked()));
			break;

		case QwtScaleDraw::TopScale:
			titleMenu.addAction(tr("&Properties..."), this, SIGNAL(topAxisTitleDblClicked()));
			break;

		case QwtScaleDraw::RightScale:
			titleMenu.addAction(tr("&Properties..."), this, SIGNAL(rightAxisTitleDblClicked()));
			break;
	}

	titleMenu.exec(QCursor::pos());
}

void Layer::showAxisContextMenu(int axis)
{
	selectedAxis = axis;

	QMenu menu(this);
	menu.setCheckable(true);
	menu.addAction(QPixmap(":/unzoom.xpm"), tr("&Rescale to show all"), this, SLOT(setAutoScale()), tr("Ctrl+Shift+R"));
	menu.addSeparator();
	menu.addAction(tr("&Hide axis"), this, SLOT(hideSelectedAxis()));

	QAction * gridsID = menu.addAction(tr("&Show grids"), this, SLOT(showGrids()));
	if (axis == QwtScaleDraw::LeftScale || axis == QwtScaleDraw::RightScale){
		if (grid.majorOnY)
			gridsID->setChecked(true);
	} else {
		if (grid.majorOnX)
			gridsID->setChecked(true);
	}

	menu.addSeparator();
	menu.addAction(tr("&Scale..."), this, SLOT(showScaleDialog()));
	menu.addAction(tr("&Properties..."), this, SLOT(showAxisDialog()));
	menu.exec(QCursor::pos());
}

void Layer::showAxisDialog()
{
	emit showAxisDialog(selectedAxis);
}

void Layer::showScaleDialog()
{
	emit axisDblClicked(selectedAxis);
}

void Layer::hideSelectedAxis()
{
	int axis = -1;
	if (selectedAxis == QwtScaleDraw::LeftScale || selectedAxis == QwtScaleDraw::RightScale)
		axis = selectedAxis - 2;
	else
		axis = selectedAxis + 2;

	m_plot->enableAxis(axis, false);
	scalePicker->refresh();
	emit modified();
}

void Layer::showGrids()
{
	showGrid (selectedAxis);
}

void Layer::showGrid()
{
	showGrid(QwtScaleDraw::LeftScale);
	showGrid(QwtScaleDraw::BottomScale);
}

void Layer::showGrid(int axis)
{
	if (axis == QwtScaleDraw::LeftScale || axis == QwtScaleDraw::RightScale){
		grid.majorOnY = 1 - grid.majorOnY;
		m_plot->grid()->enableY(grid.majorOnY);
		grid.minorOnY = 1 - grid.minorOnY;
		m_plot->grid()->enableYMin(grid.minorOnY);
	} else if (axis == QwtScaleDraw::BottomScale || axis == QwtScaleDraw::TopScale){
		grid.majorOnX = 1 - grid.majorOnX;
		m_plot->grid()->enableX(grid.majorOnX);
		grid.minorOnX = 1 - grid.minorOnX;
		m_plot->grid()->enableXMin(grid.minorOnX);
	} else
		return;

	m_plot->replot();
	emit modified();
}

void Layer::copy(Layer* g)
{
	int i;
	Plot *plot = g->plotWidget();
	m_plot->setMargin(plot->margin());

	setAntialiasing(g->antialiasing());

	setBackgroundColor(plot->paletteBackgroundColor());
	setFrame(plot->lineWidth(), plot->frameColor());
	setCanvasBackground(plot->canvasBackground());

	enableAxes(g->enabledAxes());
	setAxesColors(g->axesColors());
    setAxesNumColors(g->axesNumColors());
	setAxesBaseline(g->axesBaseline());

	setGridOptions(g->gridOptions());

	m_plot->setTitle (g->plotWidget()->title());

	drawCanvasFrame(g->framed(),g->canvasFrameWidth(), g->canvasFrameColor());

	QStringList lst = g->scalesTitles();
	for (i=0;i<(int)lst.count();i++)
		setAxisTitle(i, lst[i]);

	for (i=0;i<4;i++)
		setAxisFont(i,g->axisFont(i));

	setXAxisTitleColor(g->axisTitleColor(2));
	setXAxisTitleFont(g->axisTitleFont(2));
	setXAxisTitleAlignment(g->axisTitleAlignment(2));

	setYAxisTitleColor(g->axisTitleColor(0));
	setYAxisTitleFont(g->axisTitleFont(0));
	setYAxisTitleAlignment(g->axisTitleAlignment(0));

	setTopAxisTitleColor(g->axisTitleColor(3));
	setTopAxisTitleFont(g->axisTitleFont(3));
	setTopAxisTitleAlignment(g->axisTitleAlignment(3));

	setRightAxisTitleColor(g->axisTitleColor(1));
	setRightAxisTitleFont(g->axisTitleFont(1));
	setRightAxisTitleAlignment(g->axisTitleAlignment(1));

	setAxesLinewidth(plot->axesLinewidth());
	removeLegend();

    for (i=0; i<g->curveCount(); i++)
		{
			QwtPlotItem *it = (QwtPlotItem *)g->plotItem(i);
			if (it->rtti() == QwtPlotItem::Rtti_PlotCurve)
  	        {
  	        DataCurve *cv = (DataCurve *)it;
			int n = cv->dataSize();
			int style = ((PlotCurve *)it)->type();
			QVector<double> x(n);
			QVector<double> y(n);
			for (int j=0; j<n; j++)
			{
				x[j]=cv->x(j);
				y[j]=cv->y(j);
			}

			PlotCurve *c = 0;
			if (style == Pie)
			{
				c = new PieCurve(cv->table(), cv->title().text(), cv->startRow(), cv->endRow());
				((PieCurve*)c)->setRay(((PieCurve*)cv)->ray());
                ((PieCurve*)c)->setFirstColor(((PieCurve*)cv)->firstColor());
			}
			else if (style == Function)
			{
				c = new FunctionCurve(cv->title().text());
				((FunctionCurve*)c)->copy((FunctionCurve*)cv);
			}
			else if (style == VerticalBars || style == HorizontalBars)
			{
				c = new BarCurve(((BarCurve*)cv)->orientation(), cv->table(), cv->xColumnName(),
									cv->title().text(), cv->startRow(), cv->endRow());
				((BarCurve*)c)->copy((const BarCurve*)cv);
			}
			else if (style == ErrorBars)
			{
				ErrorCurve *er = (ErrorCurve*)cv;
				DataCurve *master_curve = masterCurve(er);
				if (master_curve)
				{
					c = new ErrorCurve(cv->table(), cv->title().text());
					((ErrorCurve*)c)->copy(er);
					((ErrorCurve*)c)->setMasterCurve(master_curve);
				}
			}
			else if (style == Histogram)
			{
				c = new HistogramCurve(cv->table(), cv->xColumnName(), cv->title().text(), cv->startRow(), cv->endRow());
				((HistogramCurve *)c)->copy((const HistogramCurve*)cv);
			}
			else if (style == VectXYXY || style == VectXYAM)
			{
				VectorCurve::VectorStyle vs = VectorCurve::XYXY;
				if (style == VectXYAM)
					vs = VectorCurve::XYAM;
				c = new VectorCurve(vs, cv->table(), cv->xColumnName(), cv->title().text(),
									((VectorCurve *)cv)->vectorEndXAColName(),
									((VectorCurve *)cv)->vectorEndYMColName(),
									cv->startRow(), cv->endRow());
				((VectorCurve *)c)->copy((const VectorCurve *)cv);
			}
			else if (style == Box)
			{
				c = new BoxCurve(cv->table(), cv->title().text(), cv->startRow(), cv->endRow());
				((BoxCurve*)c)->copy((const BoxCurve *)cv);
				QwtSingleArrayData dat(x[0], y, n);
				c->setData(dat);
			}
			else
				c = new DataCurve(cv->table(), cv->xColumnName(), cv->title().text(), cv->startRow(), cv->endRow());

			c_keys.resize(++n_curves);
			c_keys[i] = m_plot->insertCurve(c);

			c_type.resize(n_curves);
			c_type[i] = g->curveType(i);

			if (c_type[i] != Box && c_type[i] != ErrorBars)
				c->setData(x.data(), y.data(), n);

			c->setPen(cv->pen());
			c->setBrush(cv->brush());
			c->setStyle(cv->style());
			c->setSymbol(cv->symbol());

			if (cv->testCurveAttribute (QwtPlotCurve::Fitted))
				c->setCurveAttribute(QwtPlotCurve::Fitted, true);
			else if (cv->testCurveAttribute (QwtPlotCurve::Inverted))
				c->setCurveAttribute(QwtPlotCurve::Inverted, true);

			c->setAxis(cv->xAxis(), cv->yAxis());
			c->setVisible(cv->isVisible());

			QList<QwtPlotCurve *>lst = g->fitCurvesList();
			if (lst.contains((QwtPlotCurve *)it))
				m_fit_curves << c;
		}
		else if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
  	    	{
  	     	Spectrogram *sp = ((Spectrogram *)it)->copy();
  	        c_keys.resize(++n_curves);
  	        c_keys[i] = m_plot->insertCurve(sp);

  	        sp->showColorScale(((Spectrogram *)it)->colorScaleAxis(), ((Spectrogram *)it)->hasColorScale());
  	        sp->setColorBarWidth(((Spectrogram *)it)->colorBarWidth());
			sp->setVisible(it->isVisible());

  	        c_type.resize(n_curves);
  	        c_type[i] = g->curveType(i);
  	        }
  	    }

	axesFormulas = g->axesFormulas;
	axisType = g->axisType;
	axesFormatInfo = g->axesFormatInfo;
	axisType = g->axisType;

	for (i=0; i<QwtPlot::axisCnt; i++)
	{
		QwtScaleWidget *sc = g->plotWidget()->axisWidget(i);
		if (!sc)
			continue;

		QwtScaleDraw *sd = g->plotWidget()->axisScaleDraw (i);
		if (sd->hasComponent(QwtAbstractScaleDraw::Labels))
		{
			if (axisType[i] == Layer::Numeric)
				setLabelsNumericFormat(i, plot->axisLabelFormat(i), plot->axisLabelPrecision(i), axesFormulas[i]);
			else if (axisType[i] == Layer::Day)
				setLabelsDayFormat(i, axesFormatInfo[i].toInt());
			else if (axisType[i] == Layer::Month)
				setLabelsMonthFormat(i, axesFormatInfo[i].toInt());
			else if (axisType[i] == Layer::Time || axisType[i] == Layer::Date)
				setLabelsDateTimeFormat(i, axisType[i], axesFormatInfo[i]);
			else
			{
				QwtTextScaleDraw *sd = (QwtTextScaleDraw *)plot->axisScaleDraw (i);
				m_plot->setAxisScaleDraw(i, new QwtTextScaleDraw(sd->labelsList()));
			}
		}
		else
		{
			sd = m_plot->axisScaleDraw (i);
			sd->enableComponent (QwtAbstractScaleDraw::Labels, false);
		}
	}
	for (i=0; i<QwtPlot::axisCnt; i++)
	{//set same scales
		const QwtScaleEngine *se = plot->axisScaleEngine(i);
		if (!se)
			continue;

		QwtScaleEngine *sc_engine = 0;
		if (se->transformation()->type() == QwtScaleTransformation::Log10)
			sc_engine = new QwtLog10ScaleEngine();
		else if (se->transformation()->type() == QwtScaleTransformation::Linear)
			sc_engine = new QwtLinearScaleEngine();

		int majorTicks = plot->axisMaxMajor(i);
  	    int minorTicks = plot->axisMaxMinor(i);
  	    m_plot->setAxisMaxMajor (i, majorTicks);
  	    m_plot->setAxisMaxMinor (i, minorTicks);

		const QwtScaleDiv *sd = plot->axisScaleDiv(i);
		QwtValueList lst = sd->ticks (QwtScaleDiv::MajorTick);

		m_user_step[i] = g->axisStep(i);

		QwtScaleDiv div = sc_engine->divideScale (qMin(sd->lBound(), sd->hBound()),
				qMax(sd->lBound(), sd->hBound()), majorTicks, minorTicks, m_user_step[i]);

		if (se->testAttribute(QwtScaleEngine::Inverted))
		{
			sc_engine->setAttribute(QwtScaleEngine::Inverted);
			div.invert();
		}

		m_plot->setAxisScaleEngine (i, sc_engine);
		m_plot->setAxisScaleDiv (i, div);
	}

	drawAxesBackbones(g->drawAxesBackbone);
	setMajorTicksType(g->plotWidget()->getMajorTicksType());
	setMinorTicksType(g->plotWidget()->getMinorTicksType());
	setTicksLength(g->minorTickLength(), g->majorTickLength());

	setAxisLabelRotation(QwtPlot::xBottom, g->labelsRotation(QwtPlot::xBottom));
  	setAxisLabelRotation(QwtPlot::xTop, g->labelsRotation(QwtPlot::xTop));

	QVector<int> imag = g->imageMarkerKeys();
	for (i=0; i<(int)imag.size(); i++)
		addImage((ImageEnrichment*)g->imageMarker(imag[i]));
	
	QVector<int> txtMrkKeys=g->textMarkerKeys();
	for (i=0; i<(int)txtMrkKeys.size(); i++){
		TextEnrichment* mrk = (TextEnrichment*)g->textMarker(txtMrkKeys[i]);
		if (!mrk)
			continue;

		if (txtMrkKeys[i] == g->legendMarkerID)
			legendMarkerID = insertTextMarker(mrk);
		else
			insertTextMarker(mrk);
	}

	QVector<int> l = g->lineMarkerKeys();
	for (i=0; i<(int)l.size(); i++){
		LineEnrichment* lmrk=(LineEnrichment*)g->arrow(l[i]);
		if (lmrk)
			addArrow(lmrk);
	}
	m_plot->replot();

    if (isPiePlot()){
        PieCurve *c = (PieCurve *)curve(0);
        c->updateBoundingRect();
    }
}

void Layer::plotBoxDiagram(Table *w, const QStringList& names, int startRow, int endRow)
{
	if (endRow < 0)
		endRow = w->rowCount() - 1;

	for (int j = 0; j <(int)names.count(); j++){
        BoxCurve *c = new BoxCurve(w, names[j], startRow, endRow);
        c->setData(QwtSingleArrayData(double(j+1), QwtArray<double>(), 0));
        c->loadData();

        c_keys.resize(++n_curves);
        c_keys[n_curves-1] = m_plot->insertCurve(c);
        c_type.resize(n_curves);
        c_type[n_curves-1] = Box;

        c->setPen(QPen(ColorBox::color(j), 1));
        c->setSymbol(QwtSymbol(QwtSymbol::NoSymbol, QBrush(), QPen(ColorBox::color(j), 1), QSize(7,7)));
	}

	TextEnrichment* mrk=(TextEnrichment*) m_plot->marker(legendMarkerID);
	if (mrk)
		mrk->setText(legendText());

	axisType[QwtPlot::xBottom] = ColHeader;
	m_plot->setAxisScaleDraw (QwtPlot::xBottom, new QwtTextScaleDraw(w->selectedYLabels()));
	m_plot->setAxisMaxMajor(QwtPlot::xBottom, names.count()+1);
	m_plot->setAxisMaxMinor(QwtPlot::xBottom, 0);

	axisType[QwtPlot::xTop] = ColHeader;
	m_plot->setAxisScaleDraw (QwtPlot::xTop, new QwtTextScaleDraw(w->selectedYLabels()));
	m_plot->setAxisMaxMajor(QwtPlot::xTop, names.count()+1);
	m_plot->setAxisMaxMinor(QwtPlot::xTop, 0);

	axesFormatInfo[QwtPlot::xBottom] = w->name();
	axesFormatInfo[QwtPlot::xTop] = w->name();
}

void Layer::setCurveStyle(int index, int s)
{
	QwtPlotCurve *c = curve(index);
	if (!c)
		return;

	if (s == 5)//ancient spline style in Qwt 4.2.0
	{
		s = QwtPlotCurve::Lines;
		c->setCurveAttribute(QwtPlotCurve::Fitted, true);
		c_type[index] = Spline;
	}
	else if (s == QwtPlotCurve::Lines)
		c->setCurveAttribute(QwtPlotCurve::Fitted, false);
	else if (s == 6)// Vertical Steps
	{
		s = QwtPlotCurve::Steps;
		c->setCurveAttribute(QwtPlotCurve::Inverted, true);
		c_type[index] = VerticalSteps;
	}
	else if (s == QwtPlotCurve::Steps)// Horizontal Steps
	{
		c->setCurveAttribute(QwtPlotCurve::Inverted, false);
		c_type[index] = HorizontalSteps;
	}
	c->setStyle((QwtPlotCurve::CurveStyle)s);
}

void Layer::setCurveSymbol(int index, const QwtSymbol& s)
{
	QwtPlotCurve *c = curve(index);
	if (!c)
		return;

	c->setSymbol(s);
}

void Layer::setCurvePen(int index, const QPen& p)
{
	QwtPlotCurve *c = curve(index);
	if (!c)
		return;

	c->setPen(p);
}

void Layer::setCurveBrush(int index, const QBrush& b)
{
	QwtPlotCurve *c = curve(index);
	if (!c)
		return;

	c->setBrush(b);
}

void Layer::openBoxDiagram(Table *w, const QStringList& l, int fileVersion)
{
    if (!w)
        return;

    int startRow = 0;
    int endRow = w->rowCount()-1;
    if (fileVersion >= 90)
    {
        startRow = l[l.count()-3].toInt();
        endRow = l[l.count()-2].toInt();
    }

	BoxCurve *c = new BoxCurve(w, l[2], startRow, endRow);
	c->setData(QwtSingleArrayData(l[1].toDouble(), QwtArray<double>(), 0));
	c->setData(QwtSingleArrayData(l[1].toDouble(), QwtArray<double>(), 0));
	c->loadData();

	c_keys.resize(++n_curves);
	c_keys[n_curves-1] = m_plot->insertCurve(c);
	c_type.resize(n_curves);
	c_type[n_curves-1] = Box;

	c->setMaxStyle(SymbolBox::style(l[16].toInt()));
	c->setP99Style(SymbolBox::style(l[17].toInt()));
	c->setMeanStyle(SymbolBox::style(l[18].toInt()));
	c->setP1Style(SymbolBox::style(l[19].toInt()));
	c->setMinStyle(SymbolBox::style(l[20].toInt()));

	c->setBoxStyle(l[21].toInt());
	c->setBoxWidth(l[22].toInt());
	c->setBoxRange(l[23].toInt(), l[24].toDouble());
	c->setWhiskersRange(l[25].toInt(), l[26].toDouble());
}

void Layer::disableTools()
{
	if (zoomOn())
		zoom(false);
	if (drawLineActive())
		drawLine(false);
	setActiveTool(NULL);
	if (m_range_selector)
		delete m_range_selector;
}

bool Layer::enableRangeSelectors(const QObject *status_target, const char *status_slot)
{
	if (m_range_selector)
		delete m_range_selector;
	m_range_selector = new RangeSelectorTool(this, status_target, status_slot);
	connect(m_range_selector, SIGNAL(changed()), this, SIGNAL(dataRangeChanged()));
	return true;
}

void Layer::setTextMarkerDefaults(int f, const QFont& font,
		const QColor& textCol, const QColor& backgroundCol)
{
	defaultMarkerFont = font;
	defaultMarkerFrame = f;
	defaultTextMarkerColor = textCol;
	defaultTextMarkerBackground = backgroundCol;
}

void Layer::setArrowDefaults(int lineWidth,  const QColor& c, Qt::PenStyle style,
		int headLength, int headAngle, bool fillHead)
{
	defaultArrowLineWidth = lineWidth;
	defaultArrowColor = c;
	defaultArrowLineStyle = style;
	defaultArrowHeadLength = headLength;
	defaultArrowHeadAngle = headAngle;
	defaultArrowHeadFill = fillHead;
}

QString Layer::parentPlotName()
{
	QWidget *w = (QWidget *)parent()->parent();
	return QString(w->name());
}

void Layer::guessUniqueCurveLayout(int& colorIndex, int& symbolIndex)
{
	colorIndex = 0;
	symbolIndex = 0;

	int curve_index = n_curves - 1;
	if (curve_index >= 0 && c_type[curve_index] == ErrorBars)
	{// find out the pen color of the master curve
		ErrorCurve *er = (ErrorCurve *)m_plot->curve(c_keys[curve_index]);
		DataCurve *master_curve = er->masterCurve();
		if (master_curve)
		{
			colorIndex = ColorBox::colorIndex(master_curve->pen().color());
			return;
		}
	}

	for (int i=0; i<n_curves; i++)
	{
		const QwtPlotCurve *c = curve(i);
		if (c && c->rtti() == QwtPlotItem::Rtti_PlotCurve)
		{
			int index = ColorBox::colorIndex(c->pen().color());
			if (index > colorIndex)
				colorIndex = index;

			QwtSymbol symb = c->symbol();
			index = SymbolBox::symbolIndex(symb.style());
			if (index > symbolIndex)
				symbolIndex = index;
		}
	}
	if (n_curves > 1)
		colorIndex = (++colorIndex)%16;
	if (colorIndex == 13) //avoid white invisible curves
		colorIndex = 0;

	symbolIndex = (++symbolIndex)%15;
	if (!symbolIndex)
		symbolIndex = 1;
}

void Layer::addFitCurve(QwtPlotCurve *c)
{
	if (c)
		m_fit_curves << c;
}

void Layer::deleteFitCurves()
{
	QList<int> keys = m_plot->curveKeys();
	foreach(QwtPlotCurve *c, m_fit_curves)
		removeCurve(curveIndex(c));

	m_plot->replot();
}

void Layer::plotSpectrogram(Matrix *m, CurveType type)
{
	if (type != GrayMap && type != ColorMap && type != ContourMap)
  		return;

  	Spectrogram *m_spectrogram = new Spectrogram(m);
  	if (type == GrayMap)
  		m_spectrogram->setGrayScale();
  	else if (type == ContourMap)
  		{
  	    m_spectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, false);
  	    m_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
  	    }
  	else if (type == ColorMap)
  	    {
  	    m_spectrogram->setDefaultColorMap();
  	    m_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
  	    }

  	c_keys.resize(++n_curves);
  	c_keys[n_curves-1] = m_plot->insertCurve(m_spectrogram);

  	c_type.resize(n_curves);
  	c_type[n_curves-1] = type;

  	QwtScaleWidget *rightAxis = m_plot->axisWidget(QwtPlot::yRight);
  	rightAxis->setColorBarEnabled(type != ContourMap);
  	rightAxis->setColorMap(m_spectrogram->data().range(), m_spectrogram->colorMap());

  	m_plot->setAxisScale(QwtPlot::xBottom, m->xStart(), m->xEnd());
  	m_plot->setAxisScale(QwtPlot::yLeft, m->yStart(), m->yEnd());

  	m_plot->setAxisScale(QwtPlot::yRight,
  	m_spectrogram->data().range().minValue(),
  	m_spectrogram->data().range().maxValue());
  	m_plot->enableAxis(QwtPlot::yRight, type != ContourMap);

  	m_plot->replot();
}

void Layer::restoreSpectrogram(ApplicationWindow *app, const QStringList& lst)
{
  	QStringList::const_iterator line = lst.begin();
  	QString s = (*line).trimmed();
  	QString matrixName = s.remove("<matrix>").remove("</matrix>");
  	Matrix *m = app->matrix(matrixName);
  	if (!m)
        return;

  	Spectrogram *sp = new Spectrogram(m);

	c_type.resize(++n_curves);
	c_type[n_curves-1] = Layer::ColorMap;
	c_keys.resize(n_curves);
	c_keys[n_curves-1] = m_plot->insertCurve(sp);

  	for (line++; line != lst.end(); line++)
    {
        QString s = *line;
        if (s.contains("<ColorPolicy>"))
        {
            int color_policy = s.remove("<ColorPolicy>").remove("</ColorPolicy>").trimmed().toInt();
            if (color_policy == Spectrogram::GrayScale)
                sp->setGrayScale();
            else if (color_policy == Spectrogram::Default)
                sp->setDefaultColorMap();
        }
        else if (s.contains("<ColorMap>"))
        {
            s = *(++line);
            int mode = s.remove("<Mode>").remove("</Mode>").trimmed().toInt();
            s = *(++line);
            QColor color1 = QColor(s.remove("<MinColor>").remove("</MinColor>").trimmed());
            s = *(++line);
            QColor color2 = QColor(s.remove("<MaxColor>").remove("</MaxColor>").trimmed());

            QwtLinearColorMap colorMap = QwtLinearColorMap(color1, color2);
            colorMap.setMode((QwtLinearColorMap::Mode)mode);

            s = *(++line);
            int stops = s.remove("<ColorStops>").remove("</ColorStops>").trimmed().toInt();
            for (int i = 0; i < stops; i++)
            {
                s = (*(++line)).trimmed();
                QStringList l = QStringList::split("\t", s.remove("<Stop>").remove("</Stop>"));
                colorMap.addColorStop(l[0].toDouble(), QColor(l[1]));
            }
            sp->setCustomColorMap(colorMap);
            line++;
        }
        else if (s.contains("<Image>"))
        {
            int mode = s.remove("<Image>").remove("</Image>").trimmed().toInt();
            sp->setDisplayMode(QwtPlotSpectrogram::ImageMode, mode);
        }
        else if (s.contains("<ContourLines>"))
        {
            int contours = s.remove("<ContourLines>").remove("</ContourLines>").trimmed().toInt();
            sp->setDisplayMode(QwtPlotSpectrogram::ContourMode, contours);
            if (contours)
            {
                s = (*(++line)).trimmed();
                int levels = s.remove("<Levels>").remove("</Levels>").toInt();
                sp->setLevelsNumber(levels);

                s = (*(++line)).trimmed();
                int defaultPen = s.remove("<DefaultPen>").remove("</DefaultPen>").toInt();
                if (!defaultPen)
                    sp->setDefaultContourPen(Qt::NoPen);
                else
                {
                    s = (*(++line)).trimmed();
                    QColor c = QColor(s.remove("<PenColor>").remove("</PenColor>"));
                    s = (*(++line)).trimmed();
                    int width = s.remove("<PenWidth>").remove("</PenWidth>").toInt();
                    s = (*(++line)).trimmed();
                    int style = s.remove("<PenStyle>").remove("</PenStyle>").toInt();
                    sp->setDefaultContourPen(QPen(c, width, Layer::getPenStyle(style)));
                }
            }
        }
        else if (s.contains("<ColorBar>"))
        {
            s = *(++line);
            int color_axis = s.remove("<axis>").remove("</axis>").trimmed().toInt();
            s = *(++line);
            int width = s.remove("<width>").remove("</width>").trimmed().toInt();

            QwtScaleWidget *colorAxis = m_plot->axisWidget(color_axis);
            if (colorAxis)
            {
                colorAxis->setColorBarWidth(width);
                colorAxis->setColorBarEnabled(true);
            }
            line++;
        }
		else if (s.contains("<Visible>"))
        {
            int on = s.remove("<Visible>").remove("</Visible>").trimmed().toInt();
            sp->setVisible(on);
        }
    }
}

bool Layer::validCurvesDataSize()
{
	if (!n_curves)
	{
		QMessageBox::warning(this, tr("Warning"), tr("There are no curves available on this plot!"));
		return false;
	}
	else
	{
		for (int i=0; i < n_curves; i++)
		{
			 QwtPlotItem *item = curve(i);
  	         if(item && item->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
  	         {
  	             QwtPlotCurve *c = (QwtPlotCurve *)item;
  	             if (c->dataSize() >= 2)
                    return true;
  	         }
  	    }
		QMessageBox::warning(this, tr("Error"),
		tr("There are no curves with more than two points on this plot. Operation aborted!"));
		return false;
	}
}

Layer::~Layer()
{
	setActiveTool(NULL);
	if (m_range_selector)
		delete m_range_selector;
	delete titlePicker;
	delete scalePicker;
	delete cp;
	delete m_plot;
}

void Layer::setAntialiasing(bool on, bool update)
{
	if (m_antialiasing == on)
		return;

	m_antialiasing = on;

	if (update)
	{
		QList<int> curve_keys = m_plot->curveKeys();
  		for (int i=0; i<(int)curve_keys.count(); i++)
  		{
  			QwtPlotItem *c = m_plot->curve(curve_keys[i]);
			if (c)
				c->setRenderHint(QwtPlotItem::RenderAntialiased, m_antialiasing);
		}
		QList<int> marker_keys = m_plot->markerKeys();
  		for (int i=0; i<(int)marker_keys.count(); i++)
  		{
  			QwtPlotMarker *m = m_plot->marker(marker_keys[i]);
			if (m)
				m->setRenderHint(QwtPlotItem::RenderAntialiased, m_antialiasing);
		}
		m_plot->replot();
	}
}

bool Layer::focusNextPrevChild ( bool )
{
	QList<int> mrkKeys = m_plot->markerKeys();
	int n = mrkKeys.size();
	if (n < 2)
		return false;

	int min_key = mrkKeys[0], max_key = mrkKeys[0];
	for (int i = 0; i<n; i++ )
	{
		if (mrkKeys[i] >= max_key)
			max_key = mrkKeys[i];
		if (mrkKeys[i] <= min_key)
			min_key = mrkKeys[i];
	}

	int key = selectedMarker;
	if (key >= 0)
	{
		key++;
		if ( key > max_key )
			key = min_key;
	} else
		key = min_key;

	cp->disableEditing();

	setSelectedMarker(key);
	return true;
}

QString Layer::axisFormatInfo(int axis)
{
	if (axis < 0 || axis > QwtPlot::axisCnt)
		return QString();
	else
		return axesFormatInfo[axis];
}

void Layer::updateCurveNames(const QString& oldName, const QString& newName, bool updateTableName)
{
    //update plotted curves list
	QList<int> keys = m_plot->curveKeys();
	for (int i=0; i<(int)keys.count(); i++){
		QwtPlotItem *it = m_plot->plotItem(keys[i]);
		if (!it)
            continue;
        if (it->rtti() != QwtPlotItem::Rtti_PlotCurve)
            continue;

        DataCurve *c = (DataCurve *)it;
        if (c->type() != Function && c->plotAssociation().contains(oldName))
            c->updateColumnNames(oldName, newName, updateTableName);
	}

    if (legendMarkerID >= 0 )
	{//update legend
		TextEnrichment * mrk = (TextEnrichment*) m_plot->marker(legendMarkerID);
		if (mrk)
		{
            QStringList lst = mrk->text().split("\n", QString::SkipEmptyParts);
            lst.replaceInStrings(oldName, newName);
			mrk->setText(lst.join("\n"));
			m_plot->replot();
		}
	}
}

void Layer::setCurveFullRange(int curveIndex)
{
	DataCurve *c = (DataCurve *)curve(curveIndex);
	if (c)
	{
		c->setFullRange();
		updatePlot();
		emit modified();
	}
}

DataCurve* Layer::masterCurve(ErrorCurve *er)
{
	QList<int> keys = m_plot->curveKeys();
	for (int i=0; i<(int)keys.count(); i++)
	{
		QwtPlotItem *it = m_plot->plotItem(keys[i]);
		if (!it)
            continue;
        if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
            continue;
        if (((PlotCurve *)it)->type() == Function)
            continue;

        if (((DataCurve *)it)->plotAssociation() == er->masterCurve()->plotAssociation())
			return (DataCurve *)it;
	}
	return 0;
}

DataCurve* Layer::masterCurve(const QString& xColName, const QString& yColName)
{
	QString master_curve = xColName + "(X)," + yColName + "(Y)";
	QList<int> keys = m_plot->curveKeys();
	for (int i=0; i<(int)keys.count(); i++)
	{
		QwtPlotItem *it = m_plot->plotItem(keys[i]);
		if (!it)
            continue;
        if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
            continue;
        if (((PlotCurve *)it)->type() == Function)
            continue;

        if (((DataCurve *)it)->plotAssociation() == master_curve)
			return (DataCurve *)it;
	}
	return 0;
}

void Layer::showCurve(int index, bool visible)
{
	QwtPlotItem *it = plotItem(index);
	if (it)
		it->setVisible(visible);

	emit modified();
}

int Layer::visibleCurves()
{
    int c = 0;
	QList<int> keys = m_plot->curveKeys();
	for (int i=0; i<(int)keys.count(); i++)
	{
		QwtPlotItem *it = m_plot->plotItem(keys[i]);
		if (it && it->isVisible())
            c++;
	}
	return c;
}

QPrinter::PageSize Layer::minPageSize(const QPrinter& printer, const QRect& r)
{
	double x_margin = 0.2/2.54*printer.logicalDpiX(); // 2 mm margins
	double y_margin = 0.2/2.54*printer.logicalDpiY();
    double w_mm = 2*x_margin + (double)(r.width())/(double)printer.logicalDpiX() * 25.4;
    double h_mm = 2*y_margin + (double)(r.height())/(double)printer.logicalDpiY() * 25.4;

    int w, h;
    if (w_mm/h_mm > 1)
    {
        w = (int)ceil(w_mm);
        h = (int)ceil(h_mm);
    }
    else
    {
        h = (int)ceil(w_mm);
        w = (int)ceil(h_mm);
    }

	QPrinter::PageSize size = QPrinter::A5;
	if (w < 45 && h < 32)
        size =  QPrinter::B10;
	else if (w < 52 && h < 37)
        size =  QPrinter::A9;
    else if (w < 64 && h < 45)
        size =  QPrinter::B9;
    else if (w < 74 && h < 52)
        size =  QPrinter::A8;
    else if (w < 91 && h < 64)
        size =  QPrinter::B8;
    else if (w < 105 && h < 74)
        size =  QPrinter::A7;
    else if (w < 128 && h < 91)
        size =  QPrinter::B7;
    else if (w < 148 && h < 105)
        size =  QPrinter::A6;
    else if (w < 182 && h < 128)
        size =  QPrinter::B6;
    else if (w < 210 && h < 148)
        size =  QPrinter::A5;
	else if (w < 220 && h < 110)
        size =  QPrinter::DLE;
	else if (w < 229 && h < 163)
        size =  QPrinter::C5E;
	else if (w < 241 && h < 105)
        size =  QPrinter::Comm10E;
    else if (w < 257 && h < 182)
        size =  QPrinter::B5;
	else if (w < 279 && h < 216)
        size =  QPrinter::Letter;
    else if (w < 297 && h < 210)
        size =  QPrinter::A4;
	else if (w < 330 && h < 210)
        size =  QPrinter::Folio;
	else if (w < 356 && h < 216)
        size =  QPrinter::Legal;
    else if (w < 364 && h < 257)
        size =  QPrinter::B4;
    else if (w < 420 && h < 297)
        size =  QPrinter::A3;
    else if (w < 515 && h < 364)
        size =  QPrinter::B3;
    else if (w < 594 && h < 420)
        size =  QPrinter::A2;
    else if (w < 728 && h < 515)
        size =  QPrinter::B2;
	else if (w < 841 && h < 594)
        size =  QPrinter::A1;
    else if (w < 1030 && h < 728)
        size =  QPrinter::B1;
	else if (w < 1189 && h < 841)
        size =  QPrinter::A0;
    else if (w < 1456 && h < 1030)
        size =  QPrinter::B0;

	return size;
}
