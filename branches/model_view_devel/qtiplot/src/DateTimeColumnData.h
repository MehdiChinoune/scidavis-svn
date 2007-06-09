/***************************************************************************
    File                 : DateTimeColumnData.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Hoener zu Siederdissen,
    Email (use @ for *)  : thzs*gmx.net
    Description          : Data source that stores a list of QDateTimes (implementation)

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

#ifndef DATECOLUMNDATA_H
#define DATECOLUMNDATA_H

#include "AbstractColumnData.h"
#include "AbstractDateTimeDataSource.h"
#include <QList>
#include <QDateTime>
#include <QDate>
#include <QTime>
class AbstractDoubleDataSource;
class AbstractStringDataSource;
class AbstractDateTimeDataSource;

//! Data source that stores a list of QDateTimes (implementation)
/**
  * This class stores a list of QDateTimes. It implements the
  * interfaces defined in AbstractColumnData, AbstractDataSource,
  * and AbstractDateTimeDataSource as well as the data type specific
  * writing functions. The stored data can also be accessed by
  * the functions of QList\<QDateTime\>.
  *
  * There is only minor flaw (IMHO) in QDateTime:
  * QDateTime(QDate(), QTime(11,10,0,0)).toString("hh:mm") will give and emtpy string.
  * QDateTime::fromString("11:10","hh:mm") gives QDateTime(QDate(1900,1,1), QTime(11,10,0,0)) 
  * though, so whenever the date is invalid, it is replace with QDate(1900,1,1).
  * 
  * \sa AbstractColumnData
  * \sa AbstractDataSource
  * \sa AbstractDateTimeDataSource
  */
class DateTimeColumnData : public AbstractColumnData, public AbstractDateTimeDataSource, public QList<QDateTime>
{
	Q_OBJECT

public:
	//! Dtor
	virtual ~DateTimeColumnData(){};
	//! Ctor
	DateTimeColumnData();
	//! Ctor
	DateTimeColumnData(const QList<QDateTime>& list);

	//! \name Data writing functions
	//@{
	//! Copy (if necessary convert) another vector of data
	/**
	 * StringColumnData: QString to QDateTime conversion using the format string
	 * DoubleColumnData: converted from the digits before the dot as 
	 * as the julian day and the digits after the dot as a fraction
	 * of a day (0 = noon).
	 * \return true if copying was successful, otherwise false
	 * \sa format(), setFormat()
	 */
	virtual bool copy(const AbstractDataSource * other);
	//! Set a row value from a string
	/**
	 * This method is smarter than QDateTime::fromString()
	 * as it tries out several format strings until it 
	 * gets a valid date if d_format does not match.
	 */
	virtual void setRowFromString(int row, const QString& string);
	//! Set the format String
	/**
	 * The default format string is "yyyy-MM-dd hh:mm:ss.zzz".
	 * \sa QDateTime::toString()
	 */
	virtual void setFormat(const QString& format);
	//! Resize the list
	virtual void setNumRows(int new_size);
	//! Set the column label
	virtual void setLabel(const QString& label);
	//! Set the column comment
	virtual void setComment(const QString& comment);
	//! Set the column plot designation
	virtual void setPlotDesignation(AbstractDataSource::PlotDesignation pd);
	//! Insert some empty rows
	virtual void insertEmptyRows(int before, int count);
	//! Remove 'count' rows starting from row 'first'
	virtual void removeRows(int first, int count);
	//! This must be called before the column is replaced by another
	virtual void notifyReplacement(AbstractDataSource * replacement);
	//@}
	
	//! \name Data reading functions
	//@{
	//! Return the column label
	virtual QString label() const;
	//! Return the column comment
	virtual QString comment() const;
	//! Return the column plot designation
	virtual AbstractDataSource::PlotDesignation plotDesignation() const;
	//! Return the value in row 'row' in its string representation
	virtual QString textAt(int row) const;
	//! Return the value in row 'row' as a double
	/**
	 * This returns the julian day + the time as a
	 * fraction of the day (0 = noon).
	 */ 
	virtual double valueAt(int row) const;
	//! Return the format string
	/**
	 * The default format string is "yyyy-MM-dd hh:mm:ss.zzz".
	 * \sa QDateTime::toString()
	 */
	virtual QString format() const;
	//! Return the list size
	virtual int numRows() const;
	//! Return the date part of row 'row'
	virtual QDate dateAt(int row) const;
	//! Return the time part of row 'row'
	virtual QTime timeAt(int row) const;
	//! Return the QDateTime in row 'row'
	virtual QDateTime dateTimeAt(int row) const;
	//@}

	//! \name IntervalAttribute related reading functions
	//@{
	//! Return whether a certain row contains a valid value 	 
	virtual bool isValid(int row) const { return d_validity.isSet(row); } 	 
	//! Return whether a certain interval of rows contains only valid values 	 
	virtual bool isValid(Interval<int> i) const { return d_validity.isSet(i); } 	 
	//! Return all intervals of valid rows
	virtual QList< Interval<int> > validIntervals() const { return d_validity.intervals(); } 	 
	//! Return whether a certain row is selected 	 
	virtual bool isSelected(int row) const { return d_selection.isSet(row); } 	 
	//! Return whether a certain interval of rows is fully selected
	virtual bool isSelected(Interval<int> i) const { return d_selection.isSet(i); } 	 
	//! Return all selected intervals 	 
	virtual QList< Interval<int> > selectedIntervals() const { return d_selection.intervals(); } 	 
	//! Return whether a certain row is masked 	 
	virtual bool isMasked(int row) const { return d_masking.isSet(row); } 	 
	//! Return whether a certain interval of rows rows is fully masked 	 
	virtual bool isMasked(Interval<int> i) const { return d_masking.isSet(i); }
	//! Return all intervals of masked rows
	virtual QList< Interval<int> > maskedIntervals() const { return d_masking.intervals(); } 	 
	//@}
	
	//! \name IntervalAttribute related writing functions
	//@{
	//! Clear all validity information
	virtual void clearValidity()
	{
		emit validityAboutToChange(this);	
		d_validity.clear();
		emit validityChanged(this);	
	}
	//! Clear all selection information
	virtual void clearSelections()
	{
		emit selectionAboutToChange(this);	
		d_selection.clear();
		emit selectionChanged(this);	
	}
	//! Clear all masking information
	virtual void clearMasks()
	{
		emit maskingAboutToChange(this);	
		d_masking.clear();
		emit maskingChanged(this);	
	}
	//! Set an interval valid or invalid
	/**
	 * \param i the interval
	 * \param valid true: set valid, false: set invalid
	 */ 
	virtual void setValid(Interval<int> i, bool valid = true)
	{
		emit validityAboutToChange(this);	
		d_validity.setValue(i, valid);
		emit validityChanged(this);	
	}
	//! Select of deselect a certain interval
	/**
	 * \param i the interval
	 * \param select true: select, false: deselect
	 */ 
	virtual void setSelected(Interval<int> i, bool select = true)
	{
		emit selectionAboutToChange(this);	
		d_selection.setValue(i, select);
		emit selectionChanged(this);	
	}
	//! Set an interval masked
	/**
	 * \param i the interval
	 * \param mask true: mask, false: unmask
	 */ 
	virtual void setMasked(Interval<int> i, bool mask = true)
	{
		emit maskingAboutToChange(this);	
		d_masking.setValue(i, mask);
		emit maskingChanged(this);	
	}
	//@}
	
protected:
	IntervalAttribute<bool> d_validity;
	IntervalAttribute<bool> d_selection;
	IntervalAttribute<bool> d_masking;
	//! The column label
	QString d_label;
	//! The column comment
	QString d_comment;
	//! The plot designation
	AbstractDataSource::PlotDesignation d_plot_designation;
	//! Format string for QDateTime::toString()
	QString d_format;

	//! Convert and copy a double column data source
	bool copyDoubleDataSource(const AbstractDoubleDataSource * other);
	//! Convert and copy a string column data source
	/**
	 * \sa setRowFromString()
	 */
	bool copyStringDataSource(const AbstractStringDataSource * other);
	//! Copy another QDateTime column data source
	bool copyDateTimeDataSource(const AbstractDateTimeDataSource * other);

private:
	//! Read a datetime from a string
	static QDateTime dateTimeFromString(const QString& string, const QString& format);

};

#endif

