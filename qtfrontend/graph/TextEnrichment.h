/***************************************************************************
    File                 : TextEnrichment.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : TextEnrichment marker (extension to QwtPlotMarker)

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
#ifndef TEXT_ENRICHMENT_H
#define TEXT_ENRICHMENT_H

#include <qfont.h>
#include <qpen.h>

#include <qwt_plot.h>
#include <qwt_array.h>
#include <qwt_text.h>

#include "Plot.h"
#include "AbstractEnrichment.h"

/**
 * \brief A piece of text to be drawn on a Plot.
 *
 * Contrary to its name, TextEnrichment is not just used for the plot legend,
 * but for any kind of text; particularly also for the "Add Text" tool.
 * Accordingly, it is also referred to as "TextMarker" by other classes.
 *
 * \section future_plans Future Plans
 * Rename to TextMarker (or maybe TextEnrichment; see documentation of ImageMarker for details).
 *
 * \sa ImageMarker, ArrowMarker
 */
class TextEnrichment: public AbstractEnrichment
{
public:
    TextEnrichment(Plot *);
	~TextEnrichment();

	//! The kinds of frame a TextEnrichment can draw around the Text.
	enum FrameStyle{None = 0, Line = 1, Shadow=2};

	QString text(){return m_text->text();};
	void setText(const QString& s);

	//! Bounding rectangle in paint coordinates.
	QRect rect() const;
	//! Bounding rectangle in plot coordinates.
	virtual QwtDoubleRect boundingRect() const;

	void setOrigin(const QPoint & p);

	//! Sets the position of the top left corner in axis coordinates
	void setOriginCoord(double x, double y);

	//! Keep the markers on screen each time the scales are modified by adding/removing curves
	void updateOrigin();

	QColor textColor(){return m_text->color();};
	void setTextColor(const QColor& c);

	QColor backgroundColor(){return m_text->backgroundBrush().color();};
	void setBackgroundColor(const QColor& c);

	int frameStyle(){return m_frame;};
	void setFrameStyle(int style);

	QFont font(){return m_text->font();};
	void setFont(const QFont& font);

	int angle(){return m_angle;};
	void setAngle(int ang){m_angle=ang;};

private:
	void draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRect &r) const;

	void drawFrame(QPainter *p, int type, const QRect& rect) const;
	void drawSymbols(QPainter *p, const QRect& rect,
					QwtArray<long> height, int symbolLineLength) const;
	void drawTextEnrichments(QPainter *p, const QRect& rect,
					QwtArray<long> height, int symbolLineLength) const;
	void drawVector(QPainter *p, int x, int y, int l, int curveIndex) const;

	QwtArray<long> itemsHeight(int y, int symbolLineLength, int &width, int &height) const;
	int symbolsMaxLineLength() const;
	QString parse(const QString& str) const;

protected:
	//! Parent plot
	Plot *m_plot;

	//! Frame type
	int m_frame;

	//! Rotation angle: not implemented yet
	int m_angle;

	//! Pointer to the QwtText object
	QwtText* m_text;

	//! TopLeft position in pixels
	QPoint m_pos;

	//!Distance between symbols and legend text
	int hspace;

	//!Distance between frame and content
	int left_margin, top_margin;
};

#endif // ifndef TEXT_ENRICHMENT_H
