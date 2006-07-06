/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#include <qpainter.h>
#if QT_VERSION < 0x040000
#include <qpaintdevicemetrics.h>
#else
#include <qpaintengine.h>
#endif
#include "qwt_painter.h"
#include "qwt_legend_item.h"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_legend.h"
#include "qwt_rect.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_engine.h"
#include "qwt_text.h"
#include "qwt_text_label.h"
#include "qwt_math.h"

/*!
  \brief Print the plot to a \c QPaintDevice (\c QPrinter)
  This function prints the contents of a QwtPlot instance to
  \c QPaintDevice object. The size is derived from its device
  metrics.

  \param paintDev device to paint on, often a printer
  \param pfilter print filter
  \sa QwtPlot::print
  \sa QwtPlotPrintFilter
*/

void QwtPlot::print(QPaintDevice &paintDev,
   const QwtPlotPrintFilter &pfilter) const
{
#if QT_VERSION < 0x040000
    QPaintDeviceMetrics mpr(&paintDev);
    int w = mpr.width();
    int h = mpr.height();
#else
    int w = paintDev.width();
    int h = paintDev.height();
#endif

    QRect rect(0, 0, w, h);
    double aspect = double(rect.width())/double(rect.height());
    if ((aspect < 1.0))
        rect.setHeight(int(aspect*rect.width()));

    QPainter p(&paintDev);
    print(&p, rect, pfilter);
}

/*!
  \brief Paint the plot into a given rectangle.
  Paint the contents of a QwtPlot instance into a given rectangle.

  \param painter Painter
  \param plotRect Bounding rectangle
  \param pfilter Print filter
  \sa QwtPlotPrintFilter
*/
void QwtPlot::print(QPainter *painter, const QRect &plotRect,
        const QwtPlotPrintFilter &pfilter) const
{
    int axisId;

    if ( painter == 0 || !painter->isActive() ||
            !plotRect.isValid() || size().isNull() )
       return;

    painter->save();

    // All paint operations need to be scaled according to
    // the paint device metrics. 

    QwtPainter::setMetricsMap(this, painter->device());
    const QwtMetricsMap &metricsMap = QwtPainter::metricsMap();

    // It is almost impossible to integrate into the Qt layout
    // framework, when using different fonts for printing
    // and screen. To avoid writing different and Qt unconform
    // layout engines we change the widget attributes, print and 
    // reset the widget attributes again. This way we produce a lot of
    // useless layout events ...

    pfilter.apply((QwtPlot *)this);

    int baseLineDists[QwtPlot::axisCnt];
    if ( !(pfilter.options() & QwtPlotPrintFilter::PrintCanvasBackground) )
    {
        // In case of no background we set the backbone of
        // the scale on the frame of the canvas.

        for (axisId = 0; axisId < QwtPlot::axisCnt; axisId++ )
        {
            QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(axisId);
            if ( scaleWidget )
            {
                baseLineDists[axisId] = scaleWidget->margin();
                scaleWidget->setMargin(0);
            }
        }
    }
    // Calculate the layout for the print.

    int layoutOptions = QwtPlotLayout::IgnoreScrollbars 
        | QwtPlotLayout::IgnoreFrames;
    if ( !(pfilter.options() & QwtPlotPrintFilter::PrintMargin) )
        layoutOptions |= QwtPlotLayout::IgnoreMargin;
    if ( !(pfilter.options() & QwtPlotPrintFilter::PrintLegend) )
        layoutOptions |= QwtPlotLayout::IgnoreLegend;

    ((QwtPlot *)this)->plotLayout()->activate(this, 
        QwtPainter::metricsMap().deviceToLayout(plotRect), 
        layoutOptions);

    if ((pfilter.options() & QwtPlotPrintFilter::PrintTitle)
        && (!titleLabel()->text().isEmpty()))
    {
        printTitle(painter, plotLayout()->titleRect());
    }

    if ( (pfilter.options() & QwtPlotPrintFilter::PrintLegend)
        && legend() && !legend()->isEmpty() )
    {
        printLegend(painter, plotLayout()->legendRect());
    }

    for ( axisId = 0; axisId < QwtPlot::axisCnt; axisId++ )
    {
        QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(axisId);
        if (scaleWidget)
        {
            int baseDist = scaleWidget->margin();

            int startDist, endDist;
            scaleWidget->getBorderDistHint(startDist, endDist);

            printScale(painter, axisId, startDist, endDist,
                baseDist, plotLayout()->scaleRect(axisId));
        }
    }

    const QRect canvasRect = metricsMap.layoutToDevice(plotLayout()->canvasRect());

    // When using QwtPainter all sizes where computed in pixel
    // coordinates and scaled by QwtPainter later. This limits
    // the precision to screen resolution. A much better solution
    // is to scale the maps and print in unlimited resolution.

    QwtArray<QwtScaleMap> map(axisCnt);
    for (axisId = 0; axisId < axisCnt; axisId++)
    {
        map[axisId].setTransformation(axisScaleEngine(axisId)->transformation());

        const QwtScaleDiv &scaleDiv = *axisScaleDiv(axisId);
        map[axisId].setScaleInterval(scaleDiv.lBound(), scaleDiv.hBound());

        double from, to;
        if ( axisEnabled(axisId) )
        {
            const int sDist = axisWidget(axisId)->startBorderDist();
            const int eDist = axisWidget(axisId)->endBorderDist();
            const QRect &scaleRect = plotLayout()->scaleRect(axisId);

            if ( axisId == xTop || axisId == xBottom )
            {
                from = metricsMap.layoutToDeviceX(scaleRect.left() + sDist);
                to = metricsMap.layoutToDeviceX(scaleRect.right() - eDist);
            }
            else
            {
                from = metricsMap.layoutToDeviceY(scaleRect.bottom() - sDist);
                to = metricsMap.layoutToDeviceY(scaleRect.top() + eDist);
            }
        }
        else
        {
            const int margin = plotLayout()->canvasMargin(axisId);

            const QRect &canvasRect = plotLayout()->canvasRect();
            if ( axisId == yLeft || axisId == yRight )
            {
                from = metricsMap.layoutToDeviceX(canvasRect.bottom() - margin);
                to = metricsMap.layoutToDeviceX(canvasRect.top() + margin);
            }
            else
            {
                from = metricsMap.layoutToDeviceY(canvasRect.left() + margin);
                to = metricsMap.layoutToDeviceY(canvasRect.right() - margin);
            }
        }
        map[axisId].setPaintXInterval(from, to);
    }


    // The canvas maps are already scaled. 
    QwtPainter::setMetricsMap(painter->device(), painter->device());

    printCanvas(painter, canvasRect, map, pfilter);

    QwtPainter::resetMetricsMap();

    ((QwtPlot *)this)->plotLayout()->invalidate();

    // reset all widgets with their original attributes.
    if ( !(pfilter.options() & QwtPlotPrintFilter::PrintCanvasBackground) )
    {
        // restore the previous base line dists

        for (axisId = 0; axisId < QwtPlot::axisCnt; axisId++ )
        {
            QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(axisId);
            if ( scaleWidget  )
                scaleWidget->setMargin(baseLineDists[axisId]);
        }
    }

    pfilter.reset((QwtPlot *)this);

    painter->restore();
}

/*!
  Print the title into a given rectangle.

  \param painter Painter
  \param rect Bounding rectangle
*/

void QwtPlot::printTitle(QPainter *painter, const QRect &rect) const
{
    painter->setFont(titleLabel()->font());

    const QColor color = 
#if QT_VERSION < 0x040000
        titleLabel()->palette().color(
            QPalette::Active, QColorGroup::Text);
#else
        titleLabel()->palette().color(
            QPalette::Active, QPalette::Text);
#endif

    painter->setPen(color);
    titleLabel()->text().draw(painter, rect);
}

/*!
  Print the legend into a given rectangle.

  \param painter Painter
  \param rect Bounding rectangle
*/

void QwtPlot::printLegend(QPainter *painter, const QRect &rect) const
{
    if ( !legend() || legend()->isEmpty() )
        return;

    QLayout *l = legend()->contentsWidget()->layout();
    if ( l == 0 || !l->inherits("QwtDynGridLayout") )
        return;

    QwtDynGridLayout *legendLayout = (QwtDynGridLayout *)l;

    uint numCols = legendLayout->columnsForWidth(rect.width());
#if QT_VERSION < 0x040000
    QValueList<QRect> itemRects = 
        legendLayout->layoutItems(rect, numCols);
#else
    QList<QRect> itemRects = 
        legendLayout->layoutItems(rect, numCols);
#endif

    int index = 0;

#if QT_VERSION < 0x040000
    QLayoutIterator layoutIterator = legendLayout->iterator();
    for ( QLayoutItem *item = layoutIterator.current(); 
        item != 0; item = ++layoutIterator)
    {
#else
    for ( int i = 0; i < legendLayout->count(); i++ )
    {
        QLayoutItem *item = legendLayout->itemAt(i);
#endif
        QWidget *w = item->widget();
        if ( w )
        {
            painter->save();
            painter->setClipping(true);
            QwtPainter::setClipRect(painter, itemRects[index]);

            printLegendItem(painter, w, itemRects[index]);

            index++;
            painter->restore();
        }
    }
}

/*!
  Print the legend item into a given rectangle.

  \param painter Painter
  \param w Widget representing a legend item
  \param rect Bounding rectangle
*/

void QwtPlot::printLegendItem(QPainter *painter, 
    const QWidget *w, const QRect &rect) const
{
    if ( w->inherits("QwtLegendItem") )
    {
        QwtLegendItem *item = (QwtLegendItem *)w;

        painter->setFont(item->font());
        item->drawItem(painter, rect);
    }
}

/*!
  \brief Paint a scale into a given rectangle.
  Paint the scale into a given rectangle.

  \param painter Painter
  \param axisId Axis
  \param startDist Start border distance
  \param endDist End border distance
  \param baseDist Base distance
  \param rect Bounding rectangle
*/

void QwtPlot::printScale(QPainter *painter,
    int axisId, int startDist, int endDist, int baseDist, 
    const QRect &rect) const
{
    if (!axisEnabled(axisId))
        return;

    const QwtScaleWidget *scaleWidget = axisWidget(axisId);
    if ( scaleWidget->isColorBarEnabled() 
        && scaleWidget->colorBarWidth() > 0)
    {
        const QwtMetricsMap map = QwtPainter::metricsMap();

        const QRect r = map.layoutToScreen(rect);
        scaleWidget->drawColorBar(painter, scaleWidget->colorBarRect(r));

        const int off = scaleWidget->colorBarWidth() + scaleWidget->spacing();
        if ( scaleWidget->scaleDraw()->orientation() == Qt::Horizontal )
            baseDist += map.screenToLayoutY(off);
        else
            baseDist += map.screenToLayoutX(off);
    }

    QwtScaleDraw::Alignment align;
    int x, y, w;

    switch(axisId)
    {
        case yLeft:
        {
            x = rect.right() - baseDist + 1;
            y = rect.y() + startDist;
            w = rect.height() - startDist - endDist;
            align = QwtScaleDraw::LeftScale;
            break;
        }
        case yRight:
        {
            x = rect.left() + baseDist;
            y = rect.y() + startDist;
            w = rect.height() - startDist - endDist;
            align = QwtScaleDraw::RightScale;
            break;
        }
        case xTop:
        {
            x = rect.left() + startDist;
            y = rect.bottom() - baseDist + 1;
            w = rect.width() - startDist - endDist;
            align = QwtScaleDraw::TopScale;
            break;
        }
        case xBottom:
        {
            x = rect.left() + startDist;
            y = rect.top() + baseDist;
            w = rect.width() - startDist - endDist;
            align = QwtScaleDraw::BottomScale;
            break;
        }
        default:
            return;
    }

    scaleWidget->drawTitle(painter, align, rect);

    painter->save();
    painter->setFont(scaleWidget->font());

    QPen pen = painter->pen();
    pen.setWidth(scaleWidget->penWidth());
    painter->setPen(pen);

    QwtScaleDraw *sd = (QwtScaleDraw *)scaleWidget->scaleDraw();
    const QPoint sdPos = sd->pos();
    const int sdLength = sd->length();

    sd->move(x, y);
    sd->setLength(w);

#if QT_VERSION < 0x040000
    sd->draw(painter, scaleWidget->palette().active());
#else
    QPalette palette = scaleWidget->palette();
    palette.setCurrentColorGroup(QPalette::Active);
    sd->draw(painter, palette);
#endif
    // reset previous values
    sd->move(sdPos); 
    sd->setLength(sdLength); 

    painter->restore();
}

/*!
  Print the canvas into a given rectangle.

  \param painter Painter
  \param map Maps mapping between plot and paint device coordinates
  \param canvasRect Bounding rectangle
  \param pfilter Print filter
  \sa QwtPlotPrintFilter
*/

void QwtPlot::printCanvas(QPainter *painter, const QRect &canvasRect,
    const QwtArray<QwtScaleMap> &map, const QwtPlotPrintFilter &pfilter) const
{
    if ( pfilter.options() & QwtPlotPrintFilter::PrintCanvasBackground )
    {
        painter->setPen(Qt::NoPen);

        QBrush bgBrush;
#if QT_VERSION >= 0x040000
            bgBrush = canvas()->palette().brush(backgroundRole());
#else
        QColorGroup::ColorRole role =
            QPalette::backgroundRoleFromMode( backgroundMode() ); 
        bgBrush = canvas()->colorGroup().brush( role );
#endif
        painter->setBrush(bgBrush);
        
        int x1 = 0;
        int x2 = 0;
        int y1 = 0;
        int y2 = 0;

#if QT_VERSION >= 0x040000
        switch(painter->device()->paintEngine()->type())
        {
            case QPaintEngine::PostScript:
                x2 = 1;
                y2 = 1;
                break;
            default:;
        }
#endif

        const QwtMetricsMap map = QwtPainter::metricsMap();
        x1 = map.screenToLayoutX(x1);
        x2 = map.screenToLayoutX(x2);
        y1 = map.screenToLayoutY(y1);
        y2 = map.screenToLayoutY(y2);

        QwtPainter::drawRect(painter, 
            canvasRect.x() + x1, canvasRect.y() + y1, 
            canvasRect.width() - x2, canvasRect.height() - y2); 
    }
    else
    {
        // Paint the canvas borders instead.
        painter->setPen(QPen(Qt::black));
        painter->setBrush(QBrush(Qt::NoBrush));
        QwtPainter::drawRect(painter, canvasRect); 
    }


    painter->setClipping(true);
    QwtPainter::setClipRect(painter, canvasRect);

    drawItems(painter, canvasRect, map, pfilter);
}
