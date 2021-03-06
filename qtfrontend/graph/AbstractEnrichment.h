/***************************************************************************
    File                 : AbstractEnrichment.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Abstract enrichement class for 2D Graphs

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
#ifndef ABSTRACT_ENRICHMENT_H
#define ABSTRACT_ENRICHMENT_H

#include <qwt_plot.h>
#include <qwt_plot_marker.h>

/*!\brief Objects you can add on top of Graphs.
 *
 * \section future_plans Future Plans
 * Enrichments (usually) don't really mark a specific point in a plot and they don't
 * use the symbol/label functionality of QwtPlotMarker. Instead, it would make sense to provide a
 * unified move/resize (or even general affine transformations via QMatrix) interface and support for
 * positioning them either at fixed plot coordinates (like QwtPlotMarker) or at a fixed drawing
 * position within a QwtPlot (like a QWidget child); leaving the choice of positioning policy to the
 * user.
 * If AbstractEnrichment would inherit from both QWidget and QwtPlotItem (which
 * is luckily no QObject) and provide a unified drawing framework, its instances could be added
 * directly to Graph without the need for a dummy Layer in between.
 * Could also help to avoid the hack in Graph::updateMarkersBoundingRect().
 *
 * \sa LineEnrichment, TextEnrichment, ImageEnrichment
 */
class AbstractEnrichment: public QwtPlotMarker
{
public:
	AbstractEnrichment();

	//! Return bounding rectangle in paint coordinates.
	virtual QRect rect() const {return QRect(m_pos, m_size);};
	//! Set value (position) and #m_size, giving everything in paint coordinates.
	virtual void setRect(int x, int y, int w, int h);

	//! Return bounding rectangle in plot coordinates.
	virtual QwtDoubleRect boundingRect() const;
	//! Set position (xValue() and yValue()), right and bottom values giving everything in plot coordinates.
	virtual void setBoundingRect(double left, double top, double right, double bottom);

	double right(){return m_x_right;};
	double bottom(){return m_y_bottom;};

	//! Return position in paint coordinates.
	QPoint origin() const { return m_pos; };
	//! Set QwtPlotMarker::value() in paint coordinates.
	void setOrigin(const QPoint &p);

    //! Return #m_size.
	QSize size() {return m_size;};
	//! Set #m_size.
	void setSize(const QSize& size);

	virtual void updateBoundingRect();

private:
    QRect calculatePaintingRect();
	//! The right side position in scale coordinates.
	double m_x_right;
    //! The bottom side position in scale coordinates.
    double m_y_bottom;
    //! The position in paint coordiantes.
	QPoint m_pos;
	//! The size (in paint coordinates).
	QSize m_size;
};

#endif // ifndef ABSTRACT_ENRICHMENT_H
