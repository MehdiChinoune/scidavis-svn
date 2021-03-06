/***************************************************************************
    File                 : ScalePicker.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Scale picker
                           
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
#include "ScalePicker.h"

#include <qwt_plot.h>
#include <qwt_scale_widget.h>
#include <qwt_text_label.h>

#include <QMouseEvent>

ScalePicker::ScalePicker(QwtPlot *plot):
    QObject(plot)
{
	refresh();
}

bool ScalePicker::eventFilter(QObject *object, QEvent *e)
{  
	if (!object->inherits("QwtScaleWidget"))
		return QObject::eventFilter(object, e);

	if ( e->type() == QEvent::MouseButtonDblClick )
    	{
		mouseDblClicked((const QwtScaleWidget *)object, ((QMouseEvent *)e)->pos());
        return true;
    	}

	if ( e->type() == QEvent::MouseButtonPress)
    	{
		const QMouseEvent *me = (const QMouseEvent *)e;	
		if (me->button()==Qt::LeftButton)
			{
			((QwtScaleWidget *)object)->setFocus();
			emit clicked();	
			return !(me->modifiers() & Qt::ShiftModifier) && !scaleTicksRect((const QwtScaleWidget *)object).contains(me->pos());
			}
		else if (me->button() == Qt::RightButton)
			{
			mouseRightClicked((const QwtScaleWidget *)object, me->pos());
			return true;
			}
    	}
	
	return QObject::eventFilter(object, e);
}

void ScalePicker::mouseDblClicked(const QwtScaleWidget *scale, const QPoint &pos) 
{
if (scaleRect(scale).contains(pos) ) 
	emit axisDblClicked(scale->alignment());
else
	{// Click on the title
    switch(scale->alignment())   
        {
        case QwtScaleDraw::LeftScale:
            {
			emit yAxisTitleDblClicked();
            break;
            }
        case QwtScaleDraw::RightScale:
            {
			emit rightAxisTitleDblClicked();
            break;
            }
        case QwtScaleDraw::BottomScale:
            {
			emit xAxisTitleDblClicked();
            break;
            }
        case QwtScaleDraw::TopScale:
            {
			emit topAxisTitleDblClicked();
            break;
            }
		}
	}
}

void ScalePicker::mouseRightClicked(const QwtScaleWidget *scale, const QPoint &pos) 
{
if (scaleRect(scale).contains(pos)) 
	emit axisRightClicked(scale->alignment());
else
	emit axisTitleRightClicked(scale->alignment());
}

// The rect of a scale without the title
QRect ScalePicker::scaleRect(const QwtScaleWidget *scale) const
{
int margin = 1; // pixels tolerance
QRect rect = scale->rect();
rect.setRect(rect.x() - margin, rect.y() - margin, rect.width() + 2 * margin, rect.height() +  2 * margin);

if (scale->title().text().isEmpty())
	return rect;

int dh = scale->title().textSize().height();
switch(scale->alignment())   
    {
    case QwtScaleDraw::LeftScale:
        {			
		rect.setLeft(rect.left() + dh);	
        break;
        }
    case QwtScaleDraw::RightScale:
        {
		rect.setRight(rect.right() - dh);
        break;
        }
    case QwtScaleDraw::BottomScale:
        {
		rect.setBottom(rect.bottom() - dh);
	    break;
        }
    case QwtScaleDraw::TopScale:
        {
		rect.setTop(rect.top() + dh);
        break;
        }
    }
return rect;
}

void ScalePicker::refresh()
{	
    for ( uint i = 0; i < QwtPlot::axisCnt; i++ )
    {
        QwtScaleWidget *scale = (QwtScaleWidget *)plot()->axisWidget(i);
        if ( scale )
            scale->installEventFilter(this);
    }
}

QRect ScalePicker::scaleTicksRect(const QwtScaleWidget *scale) const
{
	
	int majTickLength = scale->scaleDraw()->majTickLength();
	QRect rect = scale->rect();
	switch(scale->alignment())
	{
	case QwtScaleDraw::LeftScale:
		rect.setLeft(rect.right() - majTickLength);
	break;
	case QwtScaleDraw::RightScale:
		rect.setRight(rect.left() + majTickLength);
	break;
	case QwtScaleDraw::TopScale:
		rect.setTop(rect.bottom() - majTickLength);
	break;
	case QwtScaleDraw::BottomScale:
		rect.setBottom(rect.top() + majTickLength);
	break;
	}
	return rect;
}
