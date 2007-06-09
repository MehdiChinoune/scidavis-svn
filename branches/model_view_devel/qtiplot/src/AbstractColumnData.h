/***************************************************************************
    File                 : AbstractColumnData.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Hoener zu Siederdissen,
    Email (use @ for *)  : thzs*gmx.net
    Description          : Writing interface for column based data

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

#ifndef ABSTRACTCOLUMNDATA_H
#define ABSTRACTCOLUMNDATA_H

#include "AbstractDataSource.h"
#include "IntervalAttribute.h"
class QString;

//! Writing interface for column-based data
/**
  This is an abstract base class for column-based data, 
  i.e. mathematically a 1D vector or technically a 1D array or list.
  It only defines the writing interface and has no data members itself. 
  The reading interface is defined in AbstractDataSource and
  classes derived from it.

  An instance of a subclass of this class represents a column
  in a TableDataModel or as a data source for plots and filters.
  In the latter case the filter/fit function must display the 
  data in a table that is read-only to the user or at least 
  does not support undo/redo. If instances of a subclass of AbstractColumnData
  are managed by something that supports undo, the write access
  provided by AbstractColumnData must only be available to the managing
  class itself and to its commands (QUndoCommand subclasses).
  This is for example the case for DoubleColumnData and Table.

  AbstractColumnData defines write-access methods which do not require
  knowledge of the type of data being handled. Classes derived from 
  this one will store a vector with entries of one certain data type, 
  e.g. double, QString, or QDateTime. To determine the data type of a 
  class derived from this, use qobject_cast or QObject::inherits().

  This class also implements functions to assign formulas to
  intervals of rows.
  */
class AbstractColumnData : public QObject
{
	Q_OBJECT

public:
	//! Dtor
	virtual ~AbstractColumnData(){};

	//! Copy (if necessary convert) another vector of data
	/**
	 * \return true if copying was successful, otherwise false
	 * False means the column hast been filled with a
	 * standard value in some or all rows and some or
	 * all data could not be converted to the stored data type.
	 */
	virtual bool copy(const AbstractDataSource * other) = 0;
	//! Set a row value from a string
	virtual void setRowFromString(int row, const QString& string) = 0;
	//! Resize the data vector
	virtual void setNumRows(int new_size) = 0;
	//! Set the column label
	virtual void setLabel(const QString& label) = 0; 
	//! Set the column comment
	virtual void setComment(const QString& comment) = 0;
	//! Set the column plot designation
	virtual void setPlotDesignation(AbstractDataSource::PlotDesignation pd) = 0;
	//! Insert some empty (or initialized with zero) rows
	virtual void insertEmptyRows(int before, int count) = 0;
	//! Remove 'count' rows starting from row 'first'
	virtual void removeRows(int first, int count) = 0;
	//! This must be called before the column is replaced by another
	virtual void notifyReplacement(AbstractDataSource * replacement) = 0;

	//! \name IntervalAttribute related functions
	//@{
	//! Clear all validity information
	virtual void clearValidity() = 0;
	//! Clear all selection information
	virtual void clearSelections() = 0;
	//! Clear all masking information
	virtual void clearMasks() = 0;
	//! Set an interval valid or invalid
	/**
	 * \param i the interval
	 * \param valid true: set valid, false: set invalid
	 */ 
	virtual void setValid(Interval<int> i, bool valid = true) = 0;
	//! Select of deselect a certain interval
	/**
	 * \param i the interval
	 * \param select true: select, false: deselect
	 */ 
	virtual void setSelected(Interval<int> i, bool select = true) = 0;
	//! Set an interval masked
	/**
	 * \param i the interval
	 * \param mask true: mask, false: unmask
	 */ 
	virtual void setMasked(Interval<int> i, bool mask = true) = 0;
	//@}
	
	//! \name Formula related functions
	//@{
	//! Return the formula associated with row 'row' 	 
	QString formula(int row) const { return d_formulas.value(row); }
	//! Set a formula string for an interval of rows
	void setFormula(Interval<int> i, QString formula) { d_formulas.setValue(i, formula); }
	//! Return the intervals that have associated formulas
	/**
	 * This can be used to make a list of formulas with their intervals.
	 * Here is some example code:
	 *
	 * <code>
	 * QStringList list;<br>
	 * QList< Interval<int> > ivs = my_column.formulaIntervals();<br>
	 * foreach(Interval<int> iv, ivs)<br>
	 * &nbsp;&nbsp;list << QString(iv.toString() + ": " + my_column.formula(iv.start()));<br>
	 * </code>
	 */
	QList< Interval<int> > formulaIntervals() const { return d_formulas.intervals(); }
	//! Clear all formulas
	void clearFormulas();
	//@}

protected:
	IntervalAttribute<QString> d_formulas;
};

#endif