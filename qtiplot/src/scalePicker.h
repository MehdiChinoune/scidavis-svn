/***************************************************************************
    File                 : scalePicker.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email                : ion_vasilief@yahoo.fr, thzs@gmx.net
    Description          : Scale and title picker classes
                           
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
#include <qobject.h>

class QwtPlot;
class QwtScaleWidget;
class QwtTextLabel;
class QLabel;
#include <QPoint>
class QRect;
	
//! Scale picker
class ScalePicker: public QObject
{
    Q_OBJECT
public:
    ScalePicker(QwtPlot *plot);
    virtual bool eventFilter(QObject *, QEvent *);

	//! The rect of a scale without the title
	QRect scaleRect(const QwtScaleWidget *) const;

	void mouseDblClicked(const QwtScaleWidget *, const QPoint &);
	void mouseClicked(const QwtScaleWidget *scale, const QPoint &pos) ;
	void mouseRightClicked(const QwtScaleWidget *scale, const QPoint &pos);

	void refresh();
	
	QwtPlot *plot() { return (QwtPlot *)parent(); }

signals:
	void clicked();

	void axisRightClicked(int);
	void axisTitleRightClicked(int);

	void axisDblClicked(int);
	void axisTitleDblClicked(int);

	void xAxisTitleDblClicked();
	void yAxisTitleDblClicked();
	void rightAxisTitleDblClicked();
	void topAxisTitleDblClicked();
	
	void moveGraph(const QPoint&);
	void releasedGraph();
	void highlightGraph();

private:
	bool movedGraph;
	QPoint presspos;
};

class TitlePicker: public QObject
{
    Q_OBJECT
public:
    TitlePicker(QwtPlot *plot);
    virtual bool eventFilter(QObject *, QEvent *);

signals:
	void clicked();
    void doubleClicked();
	void removeTitle();
	void showTitleMenu();

	// moving and highlighting the plot parent
	void moveGraph(const QPoint&);
	void releasedGraph();
	void highlightGraph();

protected:
	QwtTextLabel *title;
	bool movedGraph;
	QPoint presspos;
};