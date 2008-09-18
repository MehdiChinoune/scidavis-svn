/***************************************************************************
    File                 : Table.cpp
    Project              : SciDAVis
    Description          : Aspect providing a spreadsheet table with column logic
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2008 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2006-2008 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2006-2007 Ion Vasilief (ion_vasilief*yahoo.fr)
                           (replace * with @ in the email addresses) 

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
#include "table/Table.h"
#include "core/Project.h"
#include "lib/macros.h"
#include "core/AbstractScript.h"
#include "core/AspectPrivate.h"
#include "table/TableModel.h"
#include "table/TableView.h"
#include "table/tablecommands.h"
#include "core/column/Column.h"
#include "core/AbstractFilter.h"
#include "core/datatypes/String2DoubleFilter.h"
#include "core/datatypes/Double2StringFilter.h"
#include "core/datatypes/DateTime2StringFilter.h"
#include "core/datatypes/String2DayOfWeekFilter.h"
#include "core/datatypes/String2MonthFilter.h"
#include "core/datatypes/Double2DateTimeFilter.h"
#include "core/datatypes/Double2MonthFilter.h"
#include "core/datatypes/Double2DayOfWeekFilter.h"
#include "core/datatypes/String2DateTimeFilter.h"
#include "core/datatypes/DateTime2DoubleFilter.h"
#include "core/datatypes/SimpleCopyThroughFilter.h"

#include <QItemSelectionModel>
#include <QTime>
#include <QtGlobal>
#include <QHBoxLayout>
#include <QShortcut>
#include <QApplication>
#include <climits> // for RAND_MAX
#include <QClipboard>

// TODO: move as much UI independent functionality back here form TableView

Table::Table(AbstractScriptingEngine *engine, int rows, int columns, const QString& name)
	: AbstractPart(name), scripted(engine)
{
	d_table_private = new Private(this);

	// set initial number of rows and columns
	QList<Column*> cols;
	for(int i=0; i<columns; i++)
	{
		Column * new_col = new Column(QString::number(i+1), SciDAVis::Numeric);
		new_col->setPlotDesignation(i == 0 ? SciDAVis::X : SciDAVis::Y);
		cols << new_col;
	}
	appendColumns(cols);
	setRowCount(rows);

	d_view = NULL; 
}

Table::~Table()
{
}

Column * Table::column(int index) const
{ 
	return d_table_private->column(index); 
}

Column * Table::column(const QString & name) const
{ 
	for (int i=0; i<columnCount(); i++)
	{
		if (d_table_private->column(i)->name() == name)
			return d_table_private->column(i);
	}

	return NULL;
}

QWidget *Table::view()
{
	if (!d_view)
	{
		d_view = new TableView(this); 
	}
	return d_view;
}

void Table::insertColumns(int before, QList<Column*> new_cols)
{
	if( new_cols.size() < 1 || before < 0 || before > columnCount()) return;
	WAIT_CURSOR;
	beginMacro(QObject::tr("%1: insert %2 column(s)").arg(name()).arg(new_cols.size()));
	int pos=before;
	foreach(Column* col, new_cols)
		insertChild(col, pos++);
	// remark: the TableInsertColumnsCmd will be created in completeAspectInsertion()
	endMacro();
	RESET_CURSOR;
}

void Table::removeColumns(int first, int count)
{
	if( count < 1 || first < 0 || first+count > columnCount()) return;
	WAIT_CURSOR;
	beginMacro(QObject::tr("%1: remove %2 column(s)").arg(name()).arg(count));
	QList<Column*> cols;
	for(int i=first; i<(first+count); i++)
		cols.append(d_table_private->column(i));
	// remark:  the TableRemoveColumnsCmd will be created in prepareAspectRemoval()
	foreach(Column* col, cols)
		removeChild(col);
	endMacro();
	RESET_CURSOR;
}

void Table::removeColumn(Column * col)
{
	removeColumns(columnIndex(col), 1);
}

void Table::removeRows(int first, int count)
{
	if( count < 1 || first < 0 || first+count > rowCount()) return;
	WAIT_CURSOR;
	beginMacro(QObject::tr("%1: remove %2 row(s)").arg(name()).arg(count));
	int end = d_table_private->columnCount();
	for(int col=0; col<end; col++)
		d_table_private->column(col)->removeRows(first, count);
	exec(new TableSetNumberOfRowsCmd(d_table_private, d_table_private->rowCount()-count));
	endMacro();
	RESET_CURSOR;
}

void Table::insertRows(int before, int count)
{
	if( count < 1 || before < 0 || before > rowCount()) return;
	WAIT_CURSOR;
	int new_row_count = rowCount() + count;
	beginMacro(QObject::tr("%1: insert %2 row(s)").arg(name()).arg(count));
	int end = d_table_private->columnCount();
	for(int col=0; col<end; col++)
		d_table_private->column(col)->insertRows(before, count);
	setRowCount(new_row_count);
	endMacro();
	RESET_CURSOR;
}

void Table::setRowCount(int new_size)
{
	if( (new_size < 0) || (new_size == d_table_private->rowCount()) ) return;
	WAIT_CURSOR;
	beginMacro(QObject::tr("%1: set the number of rows to %2").arg(name()).arg(new_size));
	if (new_size < d_table_private->rowCount())
	{
		int end = d_table_private->columnCount();
		for(int col=0; col<end; col++)
		{	
			Column *col_ptr = d_table_private->column(col);
			if (col_ptr->rowCount() > new_size)
				col_ptr->removeRows(new_size, col_ptr->rowCount() - new_size);
		}
	}
	exec(new TableSetNumberOfRowsCmd(d_table_private, new_size));
	endMacro();
	RESET_CURSOR;
}

int Table::columnCount() const
{
	return d_table_private->columnCount();
}

int Table::rowCount() const
{
	return d_table_private->rowCount();
}

int Table::columnCount(SciDAVis::PlotDesignation pd) const
{
	int count = 0;
	int cols = columnCount();
	for(int i=0; i<cols; i++)
		if(column(i)->plotDesignation() == pd) count++;
	
	return count;
}

void Table::setColumnCount(int new_size)
{
	int old_size = columnCount();
	if ( old_size == new_size || new_size < 0 )
		return;

	WAIT_CURSOR;
	if (new_size < old_size)
		removeColumns(new_size, old_size-new_size);
	else
	{
		QList<Column*> cols;
		for(int i=0; i<new_size-old_size; i++)
		{
			Column * new_col = new Column(QString::number(i+1), SciDAVis::Numeric);
			new_col->setPlotDesignation(SciDAVis::Y);
			cols << new_col;
		}
		appendColumns(cols);
	}
	RESET_CURSOR;
}
		
int Table::columnIndex(const Column * col) const 
{ 
	return d_table_private->columnIndex(col); 
}

void Table::clear()
{
	WAIT_CURSOR;
	beginMacro(QObject::tr("%1: clear").arg(name()));
	int cols = columnCount();
	for(int i=0; i<cols; i++)
		column(i)->clear();
	endMacro();
	RESET_CURSOR;
}

void Table::clearMasks()
{
	WAIT_CURSOR;
	beginMacro(QObject::tr("%1: clear all masks").arg(name()));
	int cols = columnCount();
	for(int i=0; i<cols; i++)
		column(i)->clearMasks();
	endMacro();
	RESET_CURSOR;
}

void Table::addColumn()
{
	WAIT_CURSOR;
	beginMacro(QObject::tr("%1: add column").arg(name()));
	setColumnCount(columnCount()+1);
	endMacro();
	RESET_CURSOR;
}

void Table::addColumns(int count)
{
	WAIT_CURSOR;
	beginMacro(QObject::tr("%1: add %2 column(s)").arg(name()).arg(count));
	setColumnCount(columnCount() + count);
	endMacro();
	RESET_CURSOR;
}

void Table::addRows(int count)
{
	WAIT_CURSOR;
	beginMacro(QObject::tr("%1: add %2 rows(s)").arg(name()).arg(count));
	exec(new TableSetNumberOfRowsCmd(d_table_private, rowCount() + count));
	endMacro();
	RESET_CURSOR;
}

QMenu *Table::createContextMenu()
{
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	QMenu *menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	static_cast<TableView *>(view())->createContextMenu(menu);
	
	return menu;
#else
	return NULL;
#endif
}

bool Table::fillProjectMenu(QMenu * menu)
{
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	return static_cast<TableView *>(view())->fillProjectMenu(menu);
#else
	return false;
#endif
}

		
void Table::moveColumn(int from, int to)
{
	beginMacro(tr("%1: move column %2 from position %3 to %4.").arg(name()).arg(d_table_private->column(from)->name()).arg(from+1).arg(to+1));
	moveChild(from, to);
	exec(new TableMoveColumnCmd(d_table_private, from, to));	
	endMacro();
}

void Table::copy(Table * other)
{
	WAIT_CURSOR;
	beginMacro(QObject::tr("%1: copy %2").arg(name()).arg(other->name()));
	
	removeColumns(0, columnCount());
	QList<Column *> columns;
	for (int i=0; i<other->columnCount(); i++)
	{
		Column * src_col = other->column(i);
		Column * new_col = new Column(src_col->name(), src_col->columnMode());
		new_col->copy(src_col);
		new_col->setPlotDesignation(src_col->plotDesignation());
		QList< Interval<int> > masks = src_col->maskedIntervals();
		foreach(Interval<int> iv, masks)
			new_col->setMasked(iv);
		QList< Interval<int> > formulas = src_col->formulaIntervals();
		foreach(Interval<int> iv, formulas)
			new_col->setFormula(iv, src_col->formula(iv.start()));
		columns.append(new_col);
	}
	appendColumns(columns);
	setCaptionSpec(other->captionSpec());
	setComment(other->comment());
	for (int i=0; i<columnCount(); i++)
		setColumnWidth(i, other->columnWidth(i));
	if (d_view) d_view->rereadSectionSizes();

	endMacro();
	RESET_CURSOR;
}

int Table::colX(int col)
{
	for(int i=col-1; i>=0; i--)
	{
		if (column(i)->plotDesignation() == SciDAVis::X)
			return i;
	}
	int cols = columnCount();
	for(int i=col+1; i<cols; i++)
	{
		if (column(i)->plotDesignation() == SciDAVis::X)
			return i;
	}
	return -1;
}

int Table::colY(int col)
{
	int cols = columnCount();
	// look to the right first
	for(int i=col+1; i<cols; i++)
	{
		if (column(i)->plotDesignation() == SciDAVis::Y)
			return i;
	}
	for(int i=col-1; i>=0; i--)
	{
		if (column(i)->plotDesignation() == SciDAVis::Y)
			return i;
	}
	return -1;
}

void Table::sortColumns(Column *leading, QList<Column*> cols, bool ascending)
{
	if(cols.isEmpty()) return;

	// the normal QPair comparison does not work properly with descending sorting
	// thefore we use our own compare functions
	class CompareFunctions{ 
		public:
			static bool doubleLess(const QPair<double, int>& a, const QPair<double, int>& b)
			{
				return a.first < b.first;
			}
			static bool doubleGreater(const QPair<double, int>& a, const QPair<double, int>& b)
			{
				return a.first > b.first;
			}
			static bool QStringLess(const QPair<QString, int>& a, const QPair<QString, int>& b)
			{
				return a < b;
			}
			static bool QStringGreater(const QPair<QString, int>& a, const QPair<QString, int>& b)
			{
				return a > b;
			}
			static bool QDateTimeLess(const QPair<QDateTime, int>& a, const QPair<QDateTime, int>& b)
			{
				return a < b;
			}
			static bool QDateTimeGreater(const QPair<QDateTime, int>& a, const QPair<QDateTime, int>& b)
			{
				return a > b;
			}
	};

	WAIT_CURSOR;
	beginMacro(tr("%1: sort column(s)").arg(name()));

	if(leading == 0) // sort separately
	{
		for(int i=0; i<cols.size(); i++)
		{
			Column* col = cols.at(i);

			if(col->dataType() == SciDAVis::TypeDouble)
			{
				int rows = col->rowCount();
				QList< QPair<double, int> > map;

				for(int j=0; j<rows; j++)
					map.append(QPair<double, int>(col->valueAt(j), j));
	
				if(ascending)
					qStableSort(map.begin(), map.end(), CompareFunctions::doubleLess);
				else
					qStableSort(map.begin(), map.end(), CompareFunctions::doubleGreater);

				QListIterator< QPair<double, int> > it(map);
				Column *temp_col = new Column("temp", col->columnMode());
				
				int k=0;
				// put the values in the right order into temp_col
				while(it.hasNext())
				{
					temp_col->copy(col, it.peekNext().second, k, 1);
					temp_col->setMasked(col->isMasked(it.next().second));
					k++;
				}
				// copy the sorted column
				col->copy(temp_col, 0, 0, rows);
				delete temp_col;
			}
			else if(col->dataType() == SciDAVis::TypeQString)
			{
				int rows = col->rowCount();
				QList< QPair<QString, int> > map;

				for(int j=0; j<rows; j++)
					map.append(QPair<QString, int>(col->textAt(j), j));
	
				if(ascending)
					qStableSort(map.begin(), map.end(), CompareFunctions::QStringLess);
				else
					qStableSort(map.begin(), map.end(), CompareFunctions::QStringGreater);

				QListIterator< QPair<QString, int> > it(map);
				Column *temp_col = new Column("temp", col->columnMode());
				
				int k=0;
				// put the values in the right order into temp_col
				while(it.hasNext())
				{
					temp_col->copy(col, it.peekNext().second, k, 1);
					temp_col->setMasked(col->isMasked(it.next().second));
					k++;
				}
				// copy the sorted column
				col->copy(temp_col, 0, 0, rows);
				delete temp_col;
			}
			else if(col->dataType() == SciDAVis::TypeQDateTime)
			{
				int rows = col->rowCount();
				QList< QPair<QDateTime, int> > map;

				for(int j=0; j<rows; j++)
					map.append(QPair<QDateTime, int>(col->dateTimeAt(j), j));
	
				if(ascending)
					qStableSort(map.begin(), map.end(), CompareFunctions::QDateTimeLess);
				else
					qStableSort(map.begin(), map.end(), CompareFunctions::QDateTimeGreater);

				QListIterator< QPair<QDateTime, int> > it(map);
				Column *temp_col = new Column("temp", col->columnMode());
				
				int k=0;
				// put the values in the right order into temp_col
				while(it.hasNext())
				{
					temp_col->copy(col, it.peekNext().second, k, 1);
					temp_col->setMasked(col->isMasked(it.next().second));
					k++;
				}
				// copy the sorted column
				col->copy(temp_col, 0, 0, rows);
				delete temp_col;
			}
		}
		
	}
	else // sort with leading column
	{
		if(leading->dataType() == SciDAVis::TypeDouble)
		{
			QList< QPair<double, int> > map;
			int rows = leading->rowCount();

			for(int i=0; i<rows; i++)
				map.append(QPair<double, int>(leading->valueAt(i), i));

			if(ascending)
				qStableSort(map.begin(), map.end(), CompareFunctions::doubleLess);
			else
				qStableSort(map.begin(), map.end(), CompareFunctions::doubleGreater);
			QListIterator< QPair<double, int> > it(map);

			for(int i=0; i<cols.size(); i++) 
			{
				Column *temp_col = new Column("temp", cols.at(i)->columnMode());
				it.toFront();
				int j=0;
				// put the values in the right order into temp_col
				while(it.hasNext())
				{
					temp_col->copy(cols.at(i), it.peekNext().second, j, 1);
					temp_col->setMasked(cols.at(i)->isMasked(it.next().second));
					j++;
				}
				// copy the sorted column
				cols.at(i)->copy(temp_col, 0, 0, rows);
				delete temp_col;
			}
		}
		else if(leading->dataType() == SciDAVis::TypeQString)
		{
			QList< QPair<QString, int> > map;
			int rows = leading->rowCount();

			for(int i=0; i<rows; i++)
				map.append(QPair<QString, int>(leading->textAt(i), i));

			if(ascending)
				qStableSort(map.begin(), map.end(), CompareFunctions::QStringLess);
			else
				qStableSort(map.begin(), map.end(), CompareFunctions::QStringGreater);
			QListIterator< QPair<QString, int> > it(map);

			for(int i=0; i<cols.size(); i++) 
			{
				Column *temp_col = new Column("temp", cols.at(i)->columnMode());
				it.toFront();
				int j=0;
				// put the values in the right order into temp_col
				while(it.hasNext())
				{
					temp_col->copy(cols.at(i), it.peekNext().second, j, 1);
					temp_col->setMasked(cols.at(i)->isMasked(it.next().second));
					j++;
				}
				// copy the sorted column
				cols.at(i)->copy(temp_col, 0, 0, rows);
				delete temp_col;
			}
		}
		else if(leading->dataType() == SciDAVis::TypeQDateTime)
		{
			QList< QPair<QDateTime, int> > map;
			int rows = leading->rowCount();

			for(int i=0; i<rows; i++)
				map.append(QPair<QDateTime, int>(leading->dateTimeAt(i), i));

			if(ascending)
				qStableSort(map.begin(), map.end(), CompareFunctions::QDateTimeLess);
			else
				qStableSort(map.begin(), map.end(), CompareFunctions::QDateTimeGreater);
			QListIterator< QPair<QDateTime, int> > it(map);

			for(int i=0; i<cols.size(); i++) 
			{
				Column *temp_col = new Column("temp", cols.at(i)->columnMode());
				it.toFront();
				int j=0;
				// put the values in the right order into temp_col
				while(it.hasNext())
				{
					temp_col->copy(cols.at(i), it.peekNext().second, j, 1);
					temp_col->setMasked(cols.at(i)->isMasked(it.next().second));
					j++;
				}
				// copy the sorted column
				cols.at(i)->copy(temp_col, 0, 0, rows);
				delete temp_col;
			}
		}
	}
	endMacro();
	RESET_CURSOR;
} // end of sortColumns()


QIcon Table::icon() const
{
	QIcon ico;
	ico.addPixmap(QPixmap(":/16x16/table.png"));
	ico.addPixmap(QPixmap(":/24x24/table.png"));
	ico.addPixmap(QPixmap(":/32x32/table.png"));
	return ico;
}

QString Table::text(int row, int col)
{
	Column * col_ptr = column(col);
	if(!col_ptr)
		return QString();
	if(col_ptr->isInvalid(row))
		return QString();

	AbstractSimpleFilter * out_fltr = col_ptr->outputFilter();
	out_fltr->input(0, col_ptr);
	return out_fltr->output(0)->textAt(row);
}

/* ============== signal handlers ===== */

void Table::handleModeChange(const AbstractColumn * col)
{
	int index = columnIndex(static_cast<const Column *>(col));
	if(index != -1)
		d_table_private->updateHorizontalHeader(index, index);
}

void Table::handleDescriptionChange(const AbstractAspect * aspect)
{
	int index = columnIndex(static_cast<const Column *>(aspect));
	if(index != -1)
		d_table_private->updateHorizontalHeader(index, index);
}

void Table::handlePlotDesignationChange(const AbstractColumn * col)
{
	int index = columnIndex(static_cast<const Column *>(col));
	if(index != -1)
		d_table_private->updateHorizontalHeader(index, columnCount()-1);
}

void Table::handleDataChange(const AbstractColumn * col)
{
	int index = columnIndex(static_cast<const Column *>(col));
	if(index != -1)
		emit dataChanged(0, index, col->rowCount()-1, index);	
}

void Table::handleRowsAboutToBeInserted(const AbstractColumn * col, int before, int count)
{
	int new_size = col->rowCount() + count; 
	if(before <= col->rowCount() && new_size > rowCount())
		setRowCount(new_size);
}

void Table::handleRowsInserted(const AbstractColumn * col, int before, int count)
{
	Q_UNUSED(count);
	int index = columnIndex(static_cast<const Column *>(col));
	if(index != -1 && before <= col->rowCount())
		emit dataChanged(before, index, col->rowCount()-1, index);
}


void Table::handleRowsAboutToBeRemoved(const AbstractColumn * col, int first, int count)
{
	Q_UNUSED(col);
	Q_UNUSED(first);
	Q_UNUSED(count);
}

void Table::handleRowsRemoved(const AbstractColumn * col, int first, int count)
{
	Q_UNUSED(count);
	int index = columnIndex(static_cast<const Column *>(col));
	if(index != -1)
		emit dataChanged(first, index, col->rowCount()-1, index);
}

/* ============== end of signal handlers ===== */

void Table::connectColumn(const Column* col)
{
	connect(col, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)), this, 
			SLOT(handleDescriptionChange(const AbstractAspect *)));
	connect(col, SIGNAL(plotDesignationChanged(const AbstractColumn *)), this, 
			SLOT(handlePlotDesignationChange(const AbstractColumn *)));
	connect(col, SIGNAL(modeChanged(const AbstractColumn *)), this, 
			SLOT(handleDataChange(const AbstractColumn *)));
	connect(col, SIGNAL(dataChanged(const AbstractColumn *)), this, 
			SLOT(handleDataChange(const AbstractColumn *)));
	connect(col, SIGNAL(modeChanged(const AbstractColumn *)), this, 
			SLOT(handleModeChange(const AbstractColumn *)));
	connect(col, SIGNAL(rowsAboutToBeInserted(const AbstractColumn *, int, int)), this, 
			SLOT(handleRowsAboutToBeInserted(const AbstractColumn *,int,int)));
	connect(col, SIGNAL(rowsInserted(const AbstractColumn *, int, int)), this, 
			SLOT(handleRowsInserted(const AbstractColumn *,int,int))); 
	connect(col, SIGNAL(rowsAboutToBeRemoved(const AbstractColumn *, int, int)), this, 
			SLOT(handleRowsAboutToBeRemoved(const AbstractColumn *,int,int))); 
	connect(col, SIGNAL(rowsRemoved(const AbstractColumn *, int, int)), this, 
			SLOT(handleRowsRemoved(const AbstractColumn *,int,int))); 
	connect(col, SIGNAL(maskingChanged(const AbstractColumn *)), this, 
			SLOT(handleDataChange(const AbstractColumn *))); 
}

void Table::disconnectColumn(const Column* col)
{
	disconnect(col, 0, this, 0);
}

QVariant Table::headerData(int section, Qt::Orientation orientation,int role) const
{
	return d_table_private->headerData(section, orientation, role);
}

void Table::completeAspectInsertion(AbstractAspect * aspect, int index)
{
	Column * column = qobject_cast<Column *>(aspect);
	if (!column) return;
	QList<Column*> cols;
	cols.append(column);
	exec(new TableInsertColumnsCmd(d_table_private, index, cols));
}

void Table::prepareAspectRemoval(AbstractAspect * aspect)
{
	Column * column = qobject_cast<Column *>(aspect);
	if (!column) return;
	int first = columnIndex(column);
	QList<Column*> cols;
	cols.append(column);
	exec(new TableRemoveColumnsCmd(d_table_private, first, 1, cols));
}

/* ========== loading and saving ============ */

void Table::save(QXmlStreamWriter * writer) const
{
	int cols = columnCount();
	int rows = rowCount();
	writer->writeStartElement("table");
	writeBasicAttributes(writer);
	writer->writeAttribute("columns", QString::number(cols));
	writer->writeAttribute("rows", QString::number(rows));
	writeCommentElement(writer);

	for (int col=0; col<cols; col++)
		column(col)->save(writer);
	for (int col=0; col<cols; col++)
	{
		writer->writeStartElement("column_width");
		writer->writeAttribute("column", QString::number(col));
		writer->writeCharacters(QString::number(columnWidth(col)));
		writer->writeEndElement();
	}
	writer->writeEndElement(); // "table"
}

bool Table::load(XmlStreamReader * reader)
{
	if(reader->isStartElement() && reader->name() == "table") 
	{
		setColumnCount(0);
		setRowCount(0);
		setComment("");

		if (!readBasicAttributes(reader)) return false;

		// read dimensions
		bool ok1, ok2;
		int rows, cols;
		rows = reader->readAttributeInt("rows", &ok1);
		cols = reader->readAttributeInt("columns", &ok2);
		if(!ok1 || !ok2) 
		{
			reader->raiseError(tr("invalid row or column count"));
			return false;
		}

		setRowCount(rows);
		// read child elements
		while (!reader->atEnd()) 
		{
			reader->readNext();

			if (reader->isEndElement()) break;

			if (reader->isStartElement()) 
			{
				if (reader->name() == "comment")
				{
					if (!readCommentElement(reader)) return false;
				}
				else if(reader->name() == "column")
				{
					Column * column = new Column(tr("Column %1").arg(1), SciDAVis::Text);
					if (!column->load(reader))
					{
						setColumnCount(0);
						return false;
					}
					QList<Column *> columns;
					columns.append(column);
					appendColumns(columns);
				}
				else if(reader->name() == "column_width")
				{
					if (!readColumnWidthElement(reader)) return false;
				}
				else // unknown element
				{
					reader->raiseWarning(tr("unknown element '%1'").arg(reader->name().toString()));
					if (!reader->skipToEndElement()) return false;
				}
			} 
		}
		if (cols != columnCount())
			reader->raiseWarning(tr("columns attribute and number of read columns do not match"));
	}
	else // no table element
		reader->raiseError(tr("no table element found"));

	return !reader->hasError();
}

bool Table::readColumnWidthElement(XmlStreamReader * reader)
{
	Q_ASSERT(reader->isStartElement() && reader->name() == "column_width");
	bool ok;
	int col = reader->readAttributeInt("column", &ok);
	if(!ok)
	{
		reader->raiseError(tr("invalid or missing column index"));
		return false;
	}
	QString str = reader->readElementText();
	int value = str.toInt(&ok);
	if(!ok)
	{
		reader->raiseError(tr("invalid column width"));
		return false;
	}
	if (d_view)
		d_view->setColumnWidth(col, value);
	else
		setColumnWidth(col, value);
	return true;
}

/* ========== end of loading and saving ============ */

void Table::setColumnWidth(int col, int width) 
{ 
	d_table_private->setColumnWidth(col, width); 
}

int Table::columnWidth(int col) const 
{ 
	return d_table_private->columnWidth(col); 
}


/* ========================== Table::Private ====================== */

Column * Table::Private::column(int index) const		
{ 
	return d_columns.value(index); 
}

void Table::Private::replaceColumns(int first, QList<Column*> new_cols)
{
	if( (first < 0) || (first + new_cols.size() > d_column_count) )
		return;

	int count = new_cols.size();
	emit d_owner->columnsAboutToBeReplaced(first, new_cols.count());
	for(int i=0; i<count; i++)
	{
		int rows = new_cols.at(i)->rowCount();
		if(rows > d_row_count)
			setRowCount(rows); 

		if(d_columns.at(first+i))
			d_columns.at(first+i)->notifyReplacement(new_cols.at(i));

		d_columns[first+i] = new_cols.at(i);
		d_owner->connectColumn(new_cols.at(i));
	}
	updateHorizontalHeader(first, first+count-1);
	emit d_owner->columnsReplaced(first, new_cols.count());
	emit d_owner->dataChanged(0, first, d_row_count-1, first+count-1);
}

void Table::Private::insertColumns(int before, QList<Column*> cols)
{
	int count = cols.count();

	if( (count < 1) || (before > d_column_count) )
		return;

	Q_ASSERT(before >= 0);

	int i, rows;
	for(i=0; i<count; i++)
	{
		rows = cols.at(i)->rowCount();
		if(rows > d_row_count)
			setRowCount(rows); 
	}

	emit d_owner->columnsAboutToBeInserted(before, cols);
	for(int i=count-1; i>=0; i--)
	{
		d_columns.insert(before, cols.at(i));
		d_owner->connectColumn(cols.at(i));
		d_column_widths.insert(before, Table::global("default_column_width").toInt());
	}
	d_column_count += count;
	updateHorizontalHeader(before, before+count-1);
	emit d_owner->columnsInserted(before, cols.count());
}

void Table::Private::removeColumns(int first, int count)
{
	if( (count < 1) || (first >= d_column_count) )
		return;

	Q_ASSERT(first >= 0);

	Q_ASSERT(first+count <= d_column_count);

	emit d_owner->columnsAboutToBeRemoved(first, count);
	for(int i=count-1; i>=0; i--)
	{
		d_owner->disconnectColumn(d_columns.at(first));
		d_columns.removeAt(first);
		d_column_widths.removeAt(first);
	}
	d_column_count -= count;
	emit d_owner->columnsRemoved(first, count);
	updateHorizontalHeader(first, d_column_count-1);
}

void Table::Private::appendColumns(QList<Column*> cols)
{
	insertColumns(d_column_count, cols);
}

void Table::Private::moveColumn(int from, int to)
{
	if( from < 0 || from >= d_column_count) return;
	if( to < 0 || to >= d_column_count) return;
	
	d_columns.move(from, to);
	d_column_widths.move(from, to);
	updateHorizontalHeader(qMin(from, to), qMax(from, to));
	emit d_owner->dataChanged(0, from, d_row_count-1, from);
	emit d_owner->dataChanged(0, to, d_row_count-1, to);
	if (d_owner->d_view) d_owner->d_view->rereadSectionSizes();
}

void Table::Private::setRowCount(int rows)
{
	int diff = rows - d_row_count;
	int old_row_count = d_row_count;
	if(diff == 0) 
		return;

	if(diff > 0)
	{
		emit d_owner->rowsAboutToBeInserted(d_row_count, diff);
		d_row_count = rows;
		updateVerticalHeader(d_row_count - diff);
		emit d_owner->rowsInserted(old_row_count, diff);
	}
	else
	{
		emit d_owner->rowsAboutToBeRemoved(rows, -diff);
		d_row_count = rows;
		emit d_owner->rowsRemoved(rows, -diff);
	}
}

QString Table::Private::columnHeader(int col)
{
	return headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
}

int Table::Private::numColsWithPD(SciDAVis::PlotDesignation pd)
{
	int count = 0;
	
	for (int i=0; i<d_column_count; i++)
		if(d_columns.at(i)->plotDesignation() == pd)
			count++;
	
	return count;
}

void Table::Private::updateVerticalHeader(int start_row)
{
	int current_size = d_vertical_header_data.size(), i;
	for(i=start_row; i<current_size; i++)
		d_vertical_header_data.replace(i, QString::number(i+1));
	for(; i<d_row_count; i++)
		d_vertical_header_data << QString::number(i+1);
	emit d_owner->headerDataChanged(Qt::Vertical, start_row, d_row_count -1);	
}

void Table::Private::updateHorizontalHeader(int start_col, int end_col)
{
	if (start_col > end_col) return;

	while(d_horizontal_header_data.size() < d_column_count)
		d_horizontal_header_data << QString();

	if(numColsWithPD(SciDAVis::X)>1)
	{
		int x_cols = 0;
		for (int i=0; i<d_column_count; i++)
		{
			if (d_columns.at(i)->plotDesignation() == SciDAVis::X)
				composeColumnHeader(i, d_columns.at(i)->name()+"[X" + QString::number(++x_cols) +"]");
			else if (d_columns.at(i)->plotDesignation() == SciDAVis::Y)
			{
				if(x_cols>0)
					composeColumnHeader(i, d_columns.at(i)->name()+"[Y"+ QString::number(x_cols) +"]");
				else
					composeColumnHeader(i, d_columns.at(i)->name()+"[Y]");
			}
			else if (d_columns.at(i)->plotDesignation() == SciDAVis::Z)
			{
				if(x_cols>0)
					composeColumnHeader(i, d_columns.at(i)->name()+"[Z"+ QString::number(x_cols) +"]");
				else
					composeColumnHeader(i, d_columns.at(i)->name()+"[Z]");
			}
			else if (d_columns.at(i)->plotDesignation() == SciDAVis::xErr)
			{
				if(x_cols>0)
					composeColumnHeader(i, d_columns.at(i)->name()+"[xEr"+ QString::number(x_cols) +"]");
				else
					composeColumnHeader(i, d_columns.at(i)->name()+"[xEr]");
			}
			else if (d_columns.at(i)->plotDesignation() == SciDAVis::yErr)
			{
				if(x_cols>0)
					composeColumnHeader(i, d_columns.at(i)->name()+"[yEr"+ QString::number(x_cols) +"]");
				else
					composeColumnHeader(i, d_columns.at(i)->name()+"[yEr]");
			}
			else
				composeColumnHeader(i, d_columns.at(i)->name());
		}
	}
	else
	{
		for (int i=0; i<d_column_count; i++)
		{
			if (d_columns.at(i)->plotDesignation() == SciDAVis::X)
				composeColumnHeader(i, d_columns.at(i)->name()+"[X]");
			else if(d_columns.at(i)->plotDesignation() == SciDAVis::Y)
				composeColumnHeader(i, d_columns.at(i)->name()+"[Y]");
			else if(d_columns.at(i)->plotDesignation() == SciDAVis::Z)
				composeColumnHeader(i, d_columns.at(i)->name()+"[Z]");
			else if(d_columns.at(i)->plotDesignation() == SciDAVis::xErr)
				composeColumnHeader(i, d_columns.at(i)->name()+"[xEr]");
			else if(d_columns.at(i)->plotDesignation() == SciDAVis::yErr)
				composeColumnHeader(i, d_columns.at(i)->name()+"[yEr]");
			else
				composeColumnHeader(i, d_columns.at(i)->name());
		}
	}
	emit d_owner->headerDataChanged(Qt::Horizontal, start_col, end_col);	
}

void Table::Private::composeColumnHeader(int col, const QString& label)
{
	if (col >= d_horizontal_header_data.size())
		d_horizontal_header_data << label;
	else
		d_horizontal_header_data.replace(col, label);
}

QVariant Table::Private::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch(orientation) {
		case Qt::Horizontal:
			switch(role) {
				case Qt::DisplayRole:
				case Qt::ToolTipRole:
				case Qt::EditRole:
					return d_horizontal_header_data.at(section);
				case Qt::DecorationRole:
					return d_columns.at(section)->icon();
				case TableModel::CommentRole:
					return d_columns.at(section)->comment();
			}
		case Qt::Vertical:
			switch(role) {
				case Qt::DisplayRole:
				case Qt::ToolTipRole:
					return d_vertical_header_data.at(section);
			}
	}
	return QVariant();
}

