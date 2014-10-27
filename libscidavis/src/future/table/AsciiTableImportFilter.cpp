/***************************************************************************
    File                 : AsciiTableImportFilter.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2009 Knut Franke
    Email (use @ for *)  : Knut.Franke*gmx.net
    Description          : Import an ASCII file as Table.

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

#include "table/AsciiTableImportFilter.h"
#include "table/future_Table.h"
#include "lib/IntervalAttribute.h"
#include "core/column/Column.h"
#include "core/datatypes/String2DoubleFilter.h"

#include <QTextStream>
#include <QStringList>

QStringList AsciiTableImportFilter::fileExtensions() const
{
	return QStringList() << "txt" << "csv" << "dat";
}

AbstractAspect * AsciiTableImportFilter::importAspect(QIODevice * input)
{
	QTextStream stream(input);
	QStringList row, column_names;
	int i;
	// This is more efficient than it looks. The string lists are handed as-is to Column's
	// constructor, and thanks to implicit sharing the actual data is not copied.
	QList<QStringList> data;
	QList< IntervalAttribute<bool> > invalid_cells;

	// skip ignored lines
	for (i=0; i<d_ignored_lines; i++)
		stream.readLine();

	// read first row
	if (d_simplify_whitespace)
		row = stream.readLine().simplified().split(d_separator);
	else if (d_trim_whitespace)
		row = stream.readLine().trimmed().split(d_separator);
	else
		row = stream.readLine().split(d_separator);

	// initialize data and determine column names
	for (int i=0; i<row.size(); i++) {
		data << QStringList();
		invalid_cells << IntervalAttribute<bool>();
	}
	if (d_first_row_names_columns)
		column_names = row;
	else
		for (i=0; i<row.size(); ++i) {
			column_names << QString::number(i+1);
			data[i] << row[i];
		}

	// read rest of data
	while (!stream.atEnd()) {
		if (d_simplify_whitespace)
			row = stream.readLine().simplified().split(d_separator);
		else if (d_trim_whitespace)
			row = stream.readLine().trimmed().split(d_separator);
		else
			row = stream.readLine().split(d_separator);

		for (i=0; i<row.size() && i<data.size(); ++i)
			data[i] << row[i];
		// some rows might have too few columns (re-use value of i from above loop)
		for (; i<data.size(); ++i) {
			invalid_cells[i].setValue(data[i].size(), true);
			data[i] << "";
		}
	}

	// build a Table from the gathered data
	QList<Column*> cols;
	for (i=0; i<data.size(); ++i)
	{
		Column *new_col;
		if (d_convert_to_numeric) {
			Column * string_col = new Column(column_names[i], data[i], invalid_cells[i]);
			String2DoubleFilter * filter = new String2DoubleFilter;
			filter->setNumericLocale(d_numeric_locale);
			filter->input(0, string_col);
			new_col = new Column(column_names[i], SciDAVis::Numeric);
			new_col->copy(filter->output(0));
			delete filter;
			delete string_col;
		} else
			new_col = new Column(column_names[i], data[i], invalid_cells[i]);
		if (i == 0) 
			new_col->setPlotDesignation(SciDAVis::X);
		else
			new_col->setPlotDesignation(SciDAVis::Y);
		cols << new_col;
	}
	// renaming will be done by the kernel
	future::Table * result = new future::Table(0, 0, 0, tr("Table"));
	result->appendColumns(cols);
	return result;
}

