/***************************************************************************
    File                 : canvaspicker.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email                : ion_vasilief@yahoo.fr, thzs@gmx.net
    Description          : Canvas picker
                           
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
#include "canvaspicker.h"
#include "ImageMarker.h"
#include "LegendMarker.h"
#include "LineMarker.h"

#include <qpainter.h>
#include <q3paintdevicemetrics.h>
#include <qpixmapcache.h> 
#include <qcursor.h> 
#include <qapplication.h> 

#include <qwt_plot_canvas.h>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>
#include <Q3MemArray>
#include <QKeyEvent>
#include <QEvent>
#include <QMouseEvent>

//FIXME: All functionality disabled for now (needs port to Qwt5)

CanvasPicker::CanvasPicker(Graph *plot):
    QObject(plot)
{
#if false
	moved=false;
	movedGraph=false;
	pointSelected = false;
	plotWidget=plot->plotWidget();

    QwtPlotCanvas *canvas = plotWidget->canvas();
    canvas->installEventFilter(this);
#endif
}

bool CanvasPicker::eventFilter(QObject *object, QEvent *e)
{
#if false
    Q3MemArray<long> images=plot()->imageMarkerKeys();	
	Q3MemArray<long> texts=plot()->textMarkerKeys();
	Q3MemArray<long> lines=plot()->lineMarkerKeys();	
	
	if (object != (QObject *)plot()->plotWidget()->canvas())
        return false;

	bool moveRangeSelector=plot()->selectorsEnabled();
	bool pickerActivated=plot()->pickerActivated();
	bool dataCursorEnabled=plot()->enabledCursor();
	bool removePoint=plot()->removePointActivated();
	bool movePoint=plot()->movePointsActivated();
	
    switch(e->type())
    	{		
		case QEvent::FocusIn:
			{
			if (plot()->enabledCursor()) 
				plot()->showCursor(true);
			return true;
			}
		break;
			
		case QEvent::FocusOut:
			{
			if (plot()->enabledCursor()) 
				plot()->showCursor(true);
				{
				plotWidget->titleLabel()->repaint();
				plotWidget->replot();
				return true;
				}
			}
		break;
			
		case QEvent::Paint:
        {   
            // The inPaint guard protecs from producing endless paint events.
            static bool inPaint = false;
            if (plotWidget->canvas()->hasFocus() && !inPaint )
            {
            const QPaintEvent *pe = (const QPaintEvent *)e;
            inPaint = true;

            plotWidget->canvas()->repaint(pe->rect().x(), pe->rect().y(),
                    pe->rect().width(), pe->rect().height(), pe->erased());
            inPaint = false;
            return true; 
            }
        break;
        }
			
		case QEvent::MouseButtonPress:
			{
			const QMouseEvent *me = (const QMouseEvent *)e;	
			
			bool allAxisDisabled = true;
			for (int i=0; i < QwtPlot::axisCnt; i++)
				{
				if (plotWidget->axisEnabled(i))
					{
					allAxisDisabled = false;
					break;
					}
				}
			if (allAxisDisabled && plotWidget->margin() < 2 && plotWidget->lineWidth() < 2)
				{
				QRect r = plotWidget->canvas()->rect();
				r.addCoords(2, 2, -2, -2);
				if (!r.contains(me->pos()))
					emit highlightGraph();
				}
							
			emit selectPlot();			

			xMouse=me->pos().x();
			yMouse=me->pos().y();
			
			if (plot()->titleSelected())
				plot()->deselectTitle();	

			bool drawText=plot()->drawTextActive();
			bool select=false;
			//first perform all other operations than marker selection
			if (!drawText && !movePoint &&
				!dataCursorEnabled&& !pickerActivated &&
				!moveRangeSelector && !removePoint)	
				select=selectMarker(me->pos());
			
			if (!select)
				plot()->deselectMarker();
				
			if (removePoint || movePoint || 
				(plot()->translationInProgress() && !pickerActivated)) 
				pointSelected = plot()->selectPoint(me->pos());	
						
			if (dataCursorEnabled) 
				plot()->selectCurve(me->pos());			
				
			if (plot()->pickerActivated())
				{
				plot()->movedPicker(me->pos(), true);
				if (plot()->selectPeaksOn())
					{
					pointSelected = plot()->selectPoint(me->pos());	
					return true;
					}
				return true;
				}

			if (me->button()==Qt::LeftButton && (plot()->drawLineActive() || plot()->lineProfile()))
				{ 	
				startLinePoint= me->pos();
				plot()->copyCanvas(true);
				}
			
			if (me->button()==Qt::LeftButton && drawText)
				drawTextMarker(me->pos());		
			
			if (moveRangeSelector)
				{//moves quickly the active selector at the mouse selected point
				plot()->selectPoint(me->pos());	
				plot()->moveRangeSelector();
				}
				
			if (me->button()==Qt::RightButton && select)
				emit showMarkerPopupMenu();				
			return true;	
			}		
		break;
			
		case QEvent::MouseButtonDblClick:
			{	
			const QMouseEvent *me = (const QMouseEvent *)e;	
			if (plot()->translationInProgress() && pointSelected) 
				{
				if (!pickerActivated)
					plot()->startCurveTranslation();
				else
					plot()->translateCurveTo(me->pos());
				return false;
				}

			if (plot()->selectPeaksOn() && pointSelected && pickerActivated)
				{
				plot()->selectPeak(me->pos());
				return true;
				}

			if (movePoint || moveRangeSelector || pickerActivated || dataCursorEnabled)
				return false;
			
			if (removePoint) 
				{
				plot()->removePoint();
				return true;
				}

			long selectedMarker=plot()->selectedMarkerKey();				
		    if (selectedMarker<0)
				{
				if (plot()->isPiePlot())
					emit showPieDialog();
				else
					{
					const QMouseEvent *me = (const QMouseEvent *)e;	
					int dist;
					long curveKey = plotWidget->closestCurve(me->pos().x(), me->pos().y(), dist);
					if (dist < 10)
						emit showPlotDialog(curveKey);
					else if (plot()->curves() > 0)
						emit showPlotDialog(plot()->curveKey(0));
					}
				return true;
				}
			else
				{
				if (texts.contains(selectedMarker)>0)
					{
					emit viewTextDialog();
					return true;
					}			
				if (lines.contains(selectedMarker)>0)
					{
					emit viewLineDialog();
					return true;
					}
				if (images.contains(selectedMarker)>0)
					{
					emit viewImageDialog();
					return true;
					}
				}	
			}
		break;
			
		case QEvent::MouseMove:
		{
		if ( removePoint || moveRangeSelector || dataCursorEnabled )
				return false;
		
		const QMouseEvent *me = (const QMouseEvent *)e;
		QPoint pos = me->pos();
		long selectedMarker=plot()->selectedMarkerKey();
		
		if (plot()->drawLineActive())
			drawLineMarker(pos,true);
		else if (plot()->lineProfile())
			drawLineMarker(pos,false);		
		else if (plot()->movePointsActivated())
			plot()->move(pos); 	
		else if (plot()->pickerActivated())
			plot()->movedPicker(pos, false);		
		else if (selectedMarker>=0)
			moveMarker(pos);
		else if (!plot()->zoomOn() )
			{
			plotWidget->canvas()->setCursor(Qt::PointingHandCursor);
			movedGraph=true;
			emit moveGraph(pos);
			}
		return true;
		}
       break;
		
		case QEvent::MouseButtonRelease:
		{
			const QMouseEvent *me = (const QMouseEvent *)e;
			
			if (moved && !plot()->drawLineActive() && !plot()->lineProfile())
				releaseMarker();	
			else if (plot()->drawLineActive())
				{ 	
				LineMarker* mrk = new LineMarker(plotWidget);	
				mrk->setStartPoint(startLinePoint);
				mrk->setEndPoint(QPoint(me->x(), me->y()));

				mrk->setColor(Qt::black);
				mrk->setWidth(1);
				Qt::PenStyle style=Qt::SolidLine;
				mrk->setStyle(style);
				mrk->setEndArrow(true);
				mrk->setStartArrow(false);
				plot()->insertLineMarker(mrk);
				plotWidget->replot();
				}
			else if (plot()->lineProfile())
				{ 	
				QPoint endLinePoint=QPoint(me->x(),me->y());	
				if (endLinePoint == startLinePoint)
					return false;
				else
					{					
					LineMarker* mrk = new LineMarker(plotWidget);	
					mrk->setStartPoint(startLinePoint);
					mrk->setEndPoint(endLinePoint);
					mrk->setColor(Qt::red);
					mrk->setWidth(1);
					Qt::PenStyle style=Qt::SolidLine;
					mrk->setStyle(style);
					mrk->setEndArrow(false);
					mrk->setStartArrow(false);
				
					plot()->insertLineMarker(mrk);
					plotWidget->replot();
					emit calculateProfile(startLinePoint,endLinePoint);
					}
				}
			else if (movedGraph)
				{	
				plotWidget->canvas()->setCursor(Qt::arrowCursor);
				emit releasedGraph();
				movedGraph=false;
				}
		
		return true;			
		}
		break;
		
		 case QEvent::KeyPress:
        {	
			const int delta = 5;
			int key=((const QKeyEvent *)e)->key();
			
			if ((key == Qt::Key_Enter || key == Qt::Key_Return)&&
				plot()->translationInProgress() && pointSelected) 
				{
				if (!pickerActivated)
					plot()->startCurveTranslation();
				else
					plot()->translateCurveTo(plotWidget->canvas()->mapFromGlobal(QCursor::pos()));
				return true;
				}
				
			if ((key == Qt::Key_Enter || key == Qt::Key_Return) &&
				plot()->selectPeaksOn() && pointSelected && pickerActivated)
				{
				plot()->selectPeak(plotWidget->canvas()->mapFromGlobal(QCursor::pos()));
				return true;
				}

			if (key == Qt::Key_Tab)
				{
				plot()->selectNextMarker();
				return true;					
				}

			if (plot()->enabledCursor())
			{		
            switch(key)
				{
                case Qt::Key_Up:
						plot()->shiftCurveCursor(true);
                    return true;
                    
                case Qt::Key_Down:
                    	plot()->shiftCurveCursor(false);
                    return true;

                case Qt::Key_Right:
                case Qt::Key_Plus:
                    if ( plot()->selectedCurveID() < 0 )
                        plot()->shiftCurveCursor(true);
                    else
                        plot()->shiftPointCursor(true);
                    return true;

                case Qt::Key_Left:
                case Qt::Key_Minus:
                    if ( plot()->selectedCurveID() < 0 )
                        plot()->shiftCurveCursor(true);
                    else
                        plot()->shiftPointCursor(false);
					
                    return true;
					}
				} 
			
			//moving range selector				
			if (plot()->selectorsEnabled())
			{
            switch(key)
				{
                case Qt::Key_Up:
						plot()->shiftCurveSelector(true);
                    return true;
                    
                case Qt::Key_Down:
                    	plot()->shiftCurveSelector(true);
                    return true;

                case Qt::Key_Right:
                case Qt::Key_Plus:
						if (((const QKeyEvent *)e)->state ()==Qt::ControlModifier)
                        	plot()->moveRangeSelector(true);
						else
							plot()->shiftRangeSelector(true);
                    return true;

                case Qt::Key_Left:
                case Qt::Key_Minus:
                        if (((const QKeyEvent *)e)->state ()==Qt::ControlModifier)
                        	plot()->moveRangeSelector(false);
						else
							plot()->shiftRangeSelector(true);
					
                    return true;
					}
				} 
				
				// The following keys represent a direction, they are
                // organized on the keyboard.
				
				if (plot()->movePointsActivated())
				{
				switch(key)
					{
                case Qt::Key_1: 
                    plot()->moveBy(-delta, delta);
                    break;
                case Qt::Key_2:
                    plot()->moveBy(0, delta);
                    break;
                case Qt::Key_3: 
                    plot()->moveBy(delta, delta);
                    break;
                case Qt::Key_4:
                    plot()->moveBy(-delta, 0);
                    break;
                case Qt::Key_6: 
                    plot()->moveBy(delta, 0);
                    break;
                case Qt::Key_7:
                    plot()->moveBy(-delta, -delta);
                    break;
                case Qt::Key_8:
                    plot()->moveBy(0, -delta);
                    break;
                case Qt::Key_9:
                    plot()->moveBy(delta, -delta);
                    break;
					}
				}
				
			long selectedMarker=plot()->selectedMarkerKey();
			if (selectedMarker>=0)
				{
				int delta = 1;
				switch(key)
					{
					case Qt::Key_Left: 
						plot()->moveMarkerBy(-delta, 0);
                    break;
					case Qt::Key_Right:
						plot()->moveMarkerBy(delta, 0);
                    break;
					case Qt::Key_Up: 
						plot()->moveMarkerBy(0, -delta);
                    break;
					case Qt::Key_Down:
						plot()->moveMarkerBy(0, delta);
                    break;
					}
				}

			if (texts.contains(selectedMarker)>0 &&
				(key==Qt::Key_Enter|| key==Qt::Key_Return))
				{
				emit viewTextDialog();
				return true;
				}			
			if (lines.contains(selectedMarker)>0 &&
				(key==Qt::Key_Enter|| key==Qt::Key_Return))
				{
				emit viewLineDialog();
				return true;
				}
			if (images.contains(selectedMarker)>0 &&
				(key==Qt::Key_Enter|| key==Qt::Key_Return))
				{
				emit viewImageDialog();
				return true;
				}	
			}
			break;
							
        default:
            break;
    	}
    return QObject::eventFilter(object, e);
#endif
}

void CanvasPicker::releaseMarker()
{
#if false
Q3MemArray<long> images=plot()->imageMarkerKeys();	
Q3MemArray<long> texts=plot()->textMarkerKeys();
Q3MemArray<long> lines=plot()->lineMarkerKeys();

bool line = false, image = false;
	
long selectedMarker=plot()->selectedMarkerKey();
if (texts.contains(selectedMarker))
	{
	LegendMarker* mrkT = (LegendMarker*) plotWidget->marker(selectedMarker);
	mrkT->setOrigin(QPoint(xMrk,yMrk));	    
	}
else if (images.contains(selectedMarker))
	{
	image = true;

	ImageMarker* mrk = (ImageMarker*) plotWidget->marker(selectedMarker);
	mrk->setOrigin(QPoint(xMrk,yMrk));
	}
else if (lines.contains(selectedMarker))
	{
	line = true;

	LineMarker* mrk=(LineMarker*)plotWidget->marker(selectedMarker);
	int clw=plotWidget->canvas()->lineWidth();				
	mrk->setStartPoint(QPoint(startLinePoint.x() - clw, startLinePoint.y() - clw));
	mrk->setEndPoint(QPoint(endLinePoint.x() - clw, endLinePoint.y() - clw));
	}
	
plotWidget->replot();

if (line)
	plot()->highlightLineMarker(selectedMarker);
else if (image)
	plot()->highlightImageMarker(selectedMarker);
else
	plot()->highlightTextMarker(selectedMarker);
			
moved=false;
emit modified();	

QApplication::restoreOverrideCursor();	
#endif
}

void CanvasPicker::moveMarker(QPoint& point)
{
#if false
QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor), true);

Q3MemArray<long> images=plot()->imageMarkerKeys();	
Q3MemArray<long> texts=plot()->textMarkerKeys();
Q3MemArray<long> lines=plot()->lineMarkerKeys();
			
QPainter painter(plotWidget->canvas());
	
int w=plotWidget->canvas()->width();
int h=plotWidget->canvas()->height();
QPixmap pix(w,h,-1);
pix.fill( QColor(Qt::white));
QPixmapCache::find ("field",pix);
painter.drawPixmap(0,0,pix,0,0,-1,-1);
	
long selectedMarker=plot()->selectedMarkerKey();
if (lines.contains(selectedMarker))
	{
	LineMarker* mrk=(LineMarker*)plotWidget->marker(selectedMarker);
	
	int clw = plotWidget->canvas()->lineWidth();
	point.setX(point.x() + clw);
	point.setY(point.y() + clw);

	QPoint aux=mrk->startPoint();
    int x = aux.x()+point.x() - xMouse;
	int y = aux.y()+point.y() - yMouse;
			
	mrk->setStartPoint(QPoint(x,y));
	startLinePoint=QPoint(x,y);
		
	aux=mrk->endPoint();
	x=aux.x()+point.x()-xMouse;
	y=aux.y()+point.y()-yMouse;
	mrk->setEndPoint(QPoint(x,y));
	endLinePoint=QPoint(x,y);

	mrk->draw(&painter,0,0,QRect(0,0,0,0));
	}
else if (images.contains(selectedMarker))
	{
	ImageMarker* mrk=(ImageMarker*)plotWidget->marker(selectedMarker);

	xMrk+=point.x()-xMouse;
	yMrk+=point.y()-yMouse;

    painter.setRasterOp(Qt::NotROP);	
	painter.drawRect(QRect(QPoint(xMrk,yMrk),mrk->size()));
	}
else if (texts.contains(selectedMarker))
	{
	xMrk+=point.x()-xMouse;
	yMrk+=point.y()-yMouse;

	LegendMarker* mrk=(LegendMarker*)plotWidget->marker(selectedMarker);				
    painter.setRasterOp(Qt::NotROP);	
	painter.drawRect(QRect(QPoint(xMrk,yMrk),mrk->rect().size()));
	}
xMouse=point.x();
yMouse=point.y();
moved=true;
emit modified();	
#endif
}

void CanvasPicker::drawTextMarker(const QPoint& point)
{
#if false
LegendMarker mrkT(plotWidget);			
mrkT.setOrigin(point);
mrkT.setBackground(plot()->textMarkerDefaultFrame());
mrkT.setFont(plot()->defaultTextMarkerFont());
mrkT.setText(tr("enter your text here"));
plot()->insertTextMarker(&mrkT);		
plot()->drawText(false);
emit drawTextOff();
#endif
}

void CanvasPicker::drawLineMarker(const QPoint& point, bool endArrow)
{
#if false
plotWidget->replot();
LineMarker mrk(plotWidget);

int clw = plotWidget->canvas()->lineWidth();	
mrk.setStartPoint(QPoint(startLinePoint.x() + clw, startLinePoint.y() + clw));
mrk.setEndPoint(QPoint(point.x() + clw,point.y() + clw));

mrk.setWidth(1);
Qt::PenStyle style=Qt::SolidLine;
mrk.setStyle(style);
mrk.setEndArrow(endArrow);
mrk.setStartArrow(false);

if (endArrow)
	mrk.setColor(Qt::black);
else
	mrk.setColor(Qt::red);

QPainter painter(plot()->plotWidget()->canvas());
mrk.draw(&painter,0,0,QRect(0,0,0,0));		
#endif
}

// Selects and highlights the marker 
bool CanvasPicker::selectMarker(const QPoint& point)
{
#if false
Q3MemArray<long> images=plot()->imageMarkerKeys();	
Q3MemArray<long> texts=plot()->textMarkerKeys();
Q3MemArray<long> lines=plot()->lineMarkerKeys();
int n=texts.size();	
int m=lines.size();			
int p=images.size();

plot()->copyCanvas(true);
plotWidget->replot();

int i;
for (i=0;i<m;i++)
	{//line marker selected(first)?	
	LineMarker* mrkL=(LineMarker*) plotWidget->marker(lines[i]);
	if (mrkL)
		{
		int d=mrkL->width()+(int)floor(mrkL->headLength()*tan(M_PI*mrkL->headAngle()/180.0)+0.5);
		double dist=mrkL->dist(point.x(),point.y());
		if (dist <= d)
			{
			plot()->highlightLineMarker(lines[i]);
			return true;
			}
		}
	}

for (i=0;i<n;i++)
	{//text marker selected(second)?	
	LegendMarker* mrk=(LegendMarker*)plotWidget->marker(texts[i]);
	QRect mRect=mrk->rect();		
	if (mRect.contains(point))
		{
		plot()->highlightTextMarker(texts[i]);	
		QPoint origin=mRect.topLeft();	
		xMrk=origin.x(); yMrk=origin.y();			
		return true;
		}
	}

for (i=0;i<p;i++)
	{//image marker selected(last)?	
	ImageMarker* mrkI=(ImageMarker*)plotWidget->marker(images[i]);
	if (mrkI->rect().contains(point))
		{
		plot()->highlightImageMarker(images[i]);	
		QPoint origin=mrkI->getOrigin();
		xMrk=origin.x(); yMrk=origin.y();
		return true;
		}
	}
return false;
#endif
}
