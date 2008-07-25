/***************************************************************************
    File                 : Table.cpp
    Project              : SciDAVis
    Description          : Table worksheet class
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
#include "Table.h"
#include "SortDialog.h"
#include "core/column/Column.h"
#include "lib/Interval.h"
#include "table/TableModel.h"
#include "core/datatypes/Double2StringFilter.h"

#include <QMessageBox>
#include <QDateTime>
#include <QTextStream>
#include <QClipboard>
#include <QApplication>
#include <QPainter>
#include <QEvent>
#include <QLayout>
#include <QPrintDialog>
#include <QLocale>
#include <QShortcut>
#include <QProgressDialog>
#include <QFile>

#include <Q3TextStream>
#include <q3paintdevicemetrics.h>
#include <q3dragobject.h>
#include <Q3TableSelection>
#include <Q3MemArray>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_sort_vector.h>

Table::Table(ScriptingEnv *env, const QString &fname,const QString &sep, int ignoredLines, bool renameCols,
			 bool stripSpaces, bool simplifySpaces, const QString& label,
			 QWidget* parent, const char* name, Qt::WFlags f)
	: TableView(label, parent, name,f), scripted(env)
{
	d_future_table = new future::Table(0, 0, 0, label);
	TableView::setTable(d_future_table);	
	d_future_table->setView(this);	
	importASCII(fname, sep, ignoredLines, renameCols, stripSpaces, simplifySpaces, true);
}

Table::Table(ScriptingEnv *env, int r, int c, const QString& label, QWidget* parent, const char* name, Qt::WFlags f)
	: TableView(label, parent, name,f), scripted(env)
{
	d_future_table = new future::Table(0, r, c, label);
	init(r,c);
}

void Table::init(int rows, int cols)
{
	TableView::setTable(d_future_table);	
	d_future_table->setView(this);	

	setBirthDate(d_future_table->creationTime().toString(Qt::LocalDate));

	connect(d_future_table, SIGNAL(columnsRemoved(int, int)), this, SLOT(handleColumnsRemoved(int, int)));
	connect(d_future_table, SIGNAL(rowsInserted(int, int)), this, SLOT(handleRowChange()));
	connect(d_future_table, SIGNAL(rowsRemoved(int, int)), this, SLOT(handleRowChange()));
	connect(d_future_table, SIGNAL(dataChanged(int, int, int, int)), this, SLOT(handleColumnChange(int, int, int, int)));
	connect(d_future_table, SIGNAL(columnsReplaced(int, int)), this, SLOT(handleColumnChange(int, int)));

	connect(d_future_table, SIGNAL(columnsInserted(int, int)), this, SLOT(handleChange()));
	connect(d_future_table, SIGNAL(columnsReplaced(int, int)), this, SLOT(handleChange()));
	connect(d_future_table, SIGNAL(columnsRemoved(int, int)), this, SLOT(handleChange()));
	connect(d_future_table, SIGNAL(rowsInserted(int, int)), this, SLOT(handleChange()));
	connect(d_future_table, SIGNAL(rowsRemoved(int, int)), this, SLOT(handleChange()));
	connect(d_future_table, SIGNAL(dataChanged(int, int, int, int)), this, SLOT(handleChange()));
	connect(d_future_table, SIGNAL(headerDataChanged(Qt::Orientation, int, int)), this, SLOT(handleChange()));
	connect(d_future_table, SIGNAL(recalculate()), this, SLOT(recalculate()));
}

void Table::handleChange()
{
    emit modifiedWindow(this);
}

void Table::handleColumnChange(int first, int count)
{
	for (int i=first; i<first+count; i++)
	    emit modifiedData(this, colName(i));
}

void Table::handleColumnChange(int top, int left, int bottom, int right)
{
	Q_UNUSED(top);
	Q_UNUSED(bottom);
	handleColumnChange(left, right-left+1);
}

void Table::handleColumnsRemoved(int first, int count)
{
	for (int i=first; i<first+count; i++)
	    emit removedCol(colName(i));
}

void Table::handleRowChange()
{
	for (int i=0; i<numCols(); i++)
		emit modifiedData(this, colName(i));
}

void Table::setBackgroundColor(const QColor& col)
{
	d_view_widget->setPaletteBackgroundColor( col );
}

void Table::setTextColor(const QColor& col)
{
	d_view_widget->setPaletteForegroundColor(col);
}

void Table::setTextFont(const QFont& fnt)
{
	d_view_widget->setFont(fnt);
}

void Table::setHeaderColor(const QColor& col)
{
	d_view_widget->horizontalHeader()->setPaletteForegroundColor (col);
}

void Table::setHeaderFont(const QFont& fnt)
{
	d_view_widget->horizontalHeader()->setFont(fnt);
}

void Table::exportPDF(const QString& fileName)
{
	print(fileName);
}

void Table::print()
{
	print(QString());
}

void Table::print(const QString& fileName)
{
	QPrinter printer;
	printer.setColorMode (QPrinter::GrayScale);

	if (!fileName.isEmpty())
	{
		printer.setCreator("SciDAVis");
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName(fileName);
	}
	else
	{
		QPrintDialog printDialog(&printer);
		if (printDialog.exec() != QDialog::Accepted)
			return;
	}

	printer.setFullPage( true );
	QPainter p;
	if ( !p.begin(&printer ) )
		return; // paint on printer
	int dpiy = printer.logicalDpiY();
	const int margin = (int) ( (1/2.54)*dpiy ); // 1 cm margins

	QHeaderView *hHeader = d_view_widget->horizontalHeader();
	QHeaderView *vHeader = d_view_widget->verticalHeader();

	int rows = numRows();
	int cols = numCols();
	int height = margin;
	int i, vertHeaderWidth = vHeader->width();
	int right = margin + vertHeaderWidth;

	// print header
	p.setFont(hHeader->font());
	QString header_label = d_view_widget->model()->headerData(0, Qt::Horizontal).toString();
	QRect br = p.boundingRect(br, Qt::AlignCenter, header_label);
	p.drawLine(right, height, right, height+br.height());
	QRect tr(br);

	for (i=0;i<cols;i++)
	{
		int w = columnWidth(i);
		tr.setTopLeft(QPoint(right,height));
		tr.setWidth(w);
		tr.setHeight(br.height());
		header_label = d_view_widget->model()->headerData(i, Qt::Horizontal).toString();
		p.drawText(tr, Qt::AlignCenter, header_label,-1);
		right += w;
		p.drawLine(right, height, right, height+tr.height());

		if (right >= printer.width()-2*margin )
			break;
	}

	p.drawLine(margin + vertHeaderWidth, height, right-1, height);//first horizontal line
	height += tr.height();
	p.drawLine(margin, height, right-1, height);

	// print table values
	for (i=0;i<rows;i++)
	{
		right = margin;
		QString cell_text = d_view_widget->model()->headerData(i, Qt::Horizontal).toString()+"\t";
		tr = p.boundingRect(tr, Qt::AlignCenter, cell_text);
		p.drawLine(right, height, right, height+tr.height());

		br.setTopLeft(QPoint(right,height));
		br.setWidth(vertHeaderWidth);
		br.setHeight(tr.height());
		p.drawText(br, Qt::AlignCenter, cell_text, -1);
		right += vertHeaderWidth;
		p.drawLine(right, height, right, height+tr.height());

		for(int j=0;j<cols;j++)
		{
			int w = columnWidth (j);
			cell_text = text(i,j)+"\t";
			tr = p.boundingRect(tr,Qt::AlignCenter,cell_text);
			br.setTopLeft(QPoint(right,height));
			br.setWidth(w);
			br.setHeight(tr.height());
			p.drawText(br, Qt::AlignCenter, cell_text, -1);
			right += w;
			p.drawLine(right, height, right, height+tr.height());

			if (right >= printer.width()-2*margin )
				break;
		}
		height += br.height();
		p.drawLine(margin, height, right-1, height);

		if (height >= printer.height()-margin )
		{
			printer.newPage();
			height = margin;
			p.drawLine(margin, height, right, height);
		}
	}
}

int Table::colX(int col)
{
	return d_future_table->colX(col);
}

int Table::colY(int col)
{
	return d_future_table->colY(col);
}

void Table::setPlotDesignation(SciDAVis::PlotDesignation pd)
{
	d_future_table->setSelectionAs(pd);
}

void Table::columnNumericFormat(int col, int *f, int *precision)
{
	if (column(col)->dataType() == SciDAVis::TypeDouble)
	{
		Double2StringFilter *pFilter = qobject_cast<Double2StringFilter*>(column(col)->outputFilter());
		if (pFilter)
		{
			*precision = pFilter->numDigits();	
			switch(pFilter->numericFormat())
			{
				case 'f':
					*f = 1;
					break;

				case 'e':
					*f = 2;
					break;

				default:
				case 'g':
					*f = 0;
					break;
			}
			return;
		}
	}
	*f = 0;
	*precision = 14;
}

void Table::columnNumericFormat(int col, char *f, int *precision)
{
	if (column(col)->dataType() == SciDAVis::TypeDouble)
	{
		Double2StringFilter *pFilter = qobject_cast<Double2StringFilter*>(column(col)->outputFilter());
		if (pFilter)
		{
			*precision = pFilter->numDigits();	
			*f = pFilter->numericFormat();
			return;
		}
	}
	*f = 'g';
	*precision = 14;
}

int Table::columnWidth(int col)
{
	return d_view_widget->columnWidth(col);
}

void Table::setColWidths(const QStringList& widths)
{
	for (int i=0;i<widths.count();i++)
		d_view_widget->setColumnWidth(i, widths[i].toInt());
}

void Table::setColumnTypes(const QStringList& ctl)
{
// TODO: obsolete, remove in 0.3.0
	int n = qMin((int)ctl.count(), numCols());
	for (int i=0; i<n; i++)
	{
		QStringList l = ctl[i].split(";");
		switch (l[0].toInt())
		{
			//	old enum: enum ColType{Numeric = 0, Text = 1, Date = 2, Time = 3, Month = 4, Day = 5};
			case 0:
				column(i)->setColumnMode(SciDAVis::Numeric);
				break;
			case 1:
				column(i)->setColumnMode(SciDAVis::Text);
				break;
			case 2:
			case 3:
			case 6:
				column(i)->setColumnMode(SciDAVis::DateTime);
				break;
			case 4:
				column(i)->setColumnMode(SciDAVis::Month);
				break;
			case 5:
				column(i)->setColumnMode(SciDAVis::Day);
				break;
		}
	}
}

QString Table::saveColumnWidths()
{
// TODO: obsolete, remove in 0.3.0
	QString s="ColWidth\t";
	for (int i=0;i<numCols();i++)
		s+=QString::number(columnWidth(i))+"\t";

	return s+"\n";
}

QString Table::saveColumnTypes()
{
// TODO: obsolete, remove in 0.3.0
	QString s="ColType";
	for (int i=0; i<numCols(); i++)
		s += "\t"+QString::number(column(i)->columnMode())+";0/6";
	return s+"\n";
}

// TODO: decide whether multiple formulas can be supported, otherwise make sure the formula is copied on row inserts
void Table::setCommands(const QStringList& com)
{
	for(int i=0; i<(int)com.size() && i<numCols(); i++)
		column(i)->setFormula(Interval<int>(0, numRows()-1), com.at(i).trimmed());
}

void Table::setCommand(int col, const QString& com)
{
	column(col)->setFormula(Interval<int>(0, numRows()-1), com.trimmed());
}

void Table::setCommands(const QString& com)
{
	QStringList lst = com.split("\t");
	lst.pop_front();
	setCommands(lst);
}

bool Table::calculate(int col, int startRow, int endRow)
{
	if (col < 0 || col >= numCols())
		return false;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	if (column(col)->formula(0).isEmpty())
	{
#if 0 // TODO: should an empty formula really mean 0, "", standard date? Wouldn't it be better to not change the cell?
		for (int i=startRow; i<=endRow; i++)
		{
			setText(i, col, "");
			column(i)->setValueAt(i, 0.0);
			column(i)->setDataTimeAt(i, QDateTime(QDate(1900,1,1), QTime(12,0,0,0)));
		}
#endif
		QApplication::restoreOverrideCursor();
		return true;
	}

	Script *colscript = scriptEnv->newScript(column(col)->formula(0), this,  QString("<%1>").arg(colName(col)));
	connect(colscript, SIGNAL(error(const QString&,const QString&,int)), scriptEnv, SIGNAL(error(const QString&,const QString&,int)));
	connect(colscript, SIGNAL(print(const QString&)), scriptEnv, SIGNAL(print(const QString&)));

	if (!colscript->compile())
	{
		QApplication::restoreOverrideCursor();
		return false;
	}
	if (endRow >= numRows())
		resizeRows(endRow + 1);

	colscript->setInt(col+1, "j");
	colscript->setInt(startRow+1, "sr");
	colscript->setInt(endRow+1, "er");
	QVariant ret;
	for (int i=startRow; i<=endRow; i++)
	{
		colscript->setInt(i+1,"i");
		ret = colscript->eval();
		if(ret.type() == QVariant::Double) 
		{
			int prec;
			char f;
			columnNumericFormat(col, &f, &prec);
			column(col)->setValueAt(i, ret.toDouble());
			setText(i, col, QLocale().toString(ret.toDouble(), f, prec));
		} 
		else if(ret.canConvert(QVariant::String))
			setText(i, col, ret.toString());
		else 
		{
			QApplication::restoreOverrideCursor();
			return false;
		}
	}

	QApplication::restoreOverrideCursor();
	return true;
}

bool Table::calculate()
{
	bool success = true;
	for (int col=firstSelectedColumn(); col<=lastSelectedColumn(); col++)
		if (!calculate(col, firstSelectedRow(), lastSelectedRow()))
			success = false;
	return success;
}

QString Table::saveCommands()
{
// TODO: obsolete, remove for 0.3.0, only needed for template saving
	QString s="<com>\n";
	for (int col=0; col<numCols(); col++)
		if (!column(col)->formula(0).isEmpty())
		{
			s += "<col nr=\""+QString::number(col)+"\">\n";
			s += column(col)->formula(0);
			s += "\n</col>\n";
		}
	s += "</com>\n";
	return s;
}

QString Table::saveComments()
{
// TODO: obsolete, remove for 0.3.0, only needed for template saving
	QString s = "Comments\t";
	for (int i=0; i<numCols(); i++)
	{
		s += column(i)->comment() + "\t";
	}
	return s + "\n";
}

QString Table::saveToString(const QString& geometry)
{
	QString s = "<table>\n";
	QString xml;
	QXmlStreamWriter writer(&xml);
	d_future_table->save(&writer);
	s += QString::number(xml.length()) + "\n"; // don't know if this is needed, can't hurt though
	s += xml + "\n";
	s += geometry + "\n";
	s +="</table>\n";
	return s;
}

QString Table::saveHeader()
{
// TODO: obsolete, remove for 0.3.0, only needed for template saving
	QString s = "header";
	for (int j=0; j<numCols(); j++)
	{
		switch (column(j)->plotDesignation())
		{
			case SciDAVis::X:
				s += "\t" + colLabel(j) + "[X]";
				break;
			case SciDAVis::Y:
				s += "\t" + colLabel(j) + "[Y]";
				break;
			case SciDAVis::Z:
				s += "\t" + colLabel(j) + "[Z]";
				break;
			case SciDAVis::xErr:
				s += "\t" + colLabel(j) + "[xEr]";
				break;
			case SciDAVis::yErr:
				s += "\t" + colLabel(j) + "[yEr]";
				break;
			default:
				s += "\t" + colLabel(j);
		}
	}
	return s += "\n";
}

int Table::firstXCol()
{
	for (int j=0; j<numCols(); j++)
	{
		if (column(j)->plotDesignation() == SciDAVis::X)
			return j;
	}
	return -1;
}

void Table::setColComment(int col, const QString& s)
{
	column(col)->setComment(s);
}

void Table::setColName(int col, const QString& text)
{
	if (col < 0 || col >= numCols())
		return;

	column(col)->setName(text);
}

QStringList Table::selectedColumns()
{
// TODO for 0.3.0: extended selection support, Column * lists
	QStringList names;
	for (int i=0; i<numCols(); i++)
	{
		if(isColumnSelected(i))
			names << name() + "_" + column(i)->name();
	}
	return names;
}

QStringList Table::YColumns()
{
// TODO for 0.3.0: Column * list
	QStringList names;
	for (int i=0;i<numCols();i++)
	{
		if(column(i)->plotDesignation() == SciDAVis::Y)
			names << name() + "_" + column(i)->name();
	}
	return names;
}

QStringList Table::selectedYColumns()
{
// TODO for 0.3.0: Column * list
	QStringList names;
	for (int i=0;i<numCols();i++)
	{
		if(isColumnSelected(i) && column(i)->plotDesignation() == SciDAVis::Y)
			names << name() + "_" + column(i)->name();
	}
	return names;
}

QStringList Table::selectedErrColumns()
{
// TODO for 0.3.0: Column * list
	QStringList names;
	for (int i=0;i<numCols();i++)
	{
		if (isColumnSelected(i) && 
				(column(i)->plotDesignation() == SciDAVis::xErr || 
				 column(i)->plotDesignation() == SciDAVis::yErr) )
			names << name() + "_" + column(i)->name();
	}
	return names;
}

QStringList Table::drawableColumnSelection()
{
// TODO for 0.3.0: Column * list
	QStringList names;
	for (int i=0; i<numCols(); i++)
	{
		if(isColumnSelected(i) && column(i)->plotDesignation() == SciDAVis::Y)
			names << name() + "_" + column(i)->name();
	}

	for (int i=0; i<numCols(); i++)
	{
		if (isColumnSelected(i) && 
				(column(i)->plotDesignation() == SciDAVis::xErr || 
				 column(i)->plotDesignation() == SciDAVis::yErr) )
			names << name() + "_" + column(i)->name();
	}
	return names;
}

QStringList Table::selectedYLabels()
{
// TODO for 0.3.0: Column * list
	QStringList names;
	for (int i=0;i <numCols(); i++)
	{
		if(isColumnSelected(i) && column(i)->plotDesignation() == SciDAVis::Y)
			names << column(i)->name();
	}
	return names;
}

QStringList Table::columnsList()
{
// TODO for 0.3.0: Column * list
	QStringList names;
	for (int i=0; i<numCols(); i++)
		names << name() +"_" + column(i)->name();

	return names;
}

int Table::numSelectedRows()
{
	return selectedRowCount();
}

int Table::selectedColsNumber()
{
	return selectedColumnCount();
}

QString Table::colName(int col)
{//returns the table name + horizontal header text
	if (col<0 || col >= numCols())
		return QString();

	return QString(name() + "_" + column(col)->name());
}

void Table::insertCols(int start, int count)
{
	if (start < 0)
		start = 0;

	QList<Column*> cols;
	for(int i=0; i<count; i++)
		cols << new Column(QString::number(i+1), SciDAVis::Numeric);
	d_future_table->insertColumns(start, cols);
}

void Table::insertCol()
{
	d_future_table->insertEmptyColumns();
}

void Table::insertRow()
{
	d_future_table->insertEmptyRows();
}

void Table::addCol(SciDAVis::PlotDesignation pd)
{
	d_future_table->addColumn();
	column(d_future_table->columnCount()-1)->setPlotDesignation(pd);
}

void Table::addColumns(int c)
{
	QList<Column*> cols;
	for(int i=0; i<c; i++)
		cols << new Column(QString::number(i+1), SciDAVis::Numeric);
	d_future_table->appendColumns(cols);
}

void Table::clearCol()
{
	d_future_table->clearSelectedColumns();
}

void Table::clearCell(int row, int col)
{
	setText(row, col, "");
}

void Table::deleteSelectedRows()
{
	d_future_table->removeSelectedRows();
}

void Table::cutSelection()
{
	d_future_table->cutSelection();
}

void Table::selectAllTable()
{
	selectAll();
}

void Table::deselect()
{
	d_view_widget->clearSelection();
}

void Table::clearSelection()
{
	d_future_table->clearSelectedCells();
}

void Table::copySelection()
{
	d_future_table->copySelection();
}

void Table::pasteSelection()
{
	d_future_table->pasteIntoSelection();
}

void Table::removeCol()
{
	d_future_table->removeSelectedColumns();
}

void Table::removeCol(const QStringList& list)
{
	foreach(QString name, list)
		d_future_table->removeColumns(colIndex(name), 1);
}

int Table::numRows()
{
	return d_future_table->rowCount();
}

int Table::numCols()
{
	return d_future_table->columnCount();
}

double Table::cell(int row, int col)
{
	return column(col)->valueAt(row);
}

void Table::setCell(int row, int col, double val)
{
	column(col)->setValueAt(row, val);
}

QString Table::text(int row, int col)
{
	return d_model->data(d_model->index(row, col), Qt::DisplayRole).toString();
}

void Table::setText(int row, int col, const QString & text)
{
	d_model->setData(d_model->index(row, col), text, Qt::DisplayRole);
}

void Table::importV0x0001XXHeader(QStringList header)
{
	QStringList col_label = QStringList();
	QList<SciDAVis::PlotDesignation> col_plot_type = QList<SciDAVis::PlotDesignation>();
	for (int i=0; i<header.count();i++)
	{
		if (header[i].isEmpty())
			continue;

		QString s = header[i].replace("_","-");
		if (s.contains("[X]"))
		{
			col_label << s.remove("[X]");
			col_plot_type << SciDAVis::X;
		}
		else if (s.contains("[Y]"))
		{
			col_label << s.remove("[Y]");
			col_plot_type << SciDAVis::Y;
		}
		else if (s.contains("[Z]"))
		{
			col_label << s.remove("[Z]");
			col_plot_type << SciDAVis::Z;
		}
		else if (s.contains("[xEr]"))
		{
			col_label << s.remove("[xEr]");
			col_plot_type << SciDAVis::xErr;
		}
		else if (s.contains("[yEr]"))
		{
			col_label << s.remove("[yEr]");
			col_plot_type << SciDAVis::yErr;
		}
		else
		{
			col_label << s;
			col_plot_type << SciDAVis::noDesignation;
		}
	}
	for (int i=0; i<col_label.count() && i<d_future_table->columnCount();i++)
	{
		column(i)->setName(col_label.at(i));
		column(i)->setPlotDesignation(col_plot_type.at(i));
	}
}

void Table::setHeader(QStringList header)
{
	for (int i=0; i<header.count() && i<d_future_table->columnCount();i++)
		column(i)->setName(header.at(i));
}

int Table::colIndex(const QString& name)
{
	// TODO for 0.3.0: remove all name concatenation with _ in favor of Column * pointers
	int pos=name.find("_",false);
	QString label=name.right(name.length()-pos-1);
	return d_future_table->columnIndex(column(label));
}

bool Table::noXColumn()
{
	return d_future_table->columnCount(SciDAVis::X) == 0;
}

bool Table::noYColumn()
{
	return d_future_table->columnCount(SciDAVis::Y) == 0;
}

// TODO: vvvvv

void Table::importMultipleASCIIFiles(const QString &fname, const QString &sep, int ignoredLines,
		bool renameCols, bool stripSpaces, bool simplifySpaces,
		int importFileAs)
{
	// TODO: port
#if 0
	QFile f(fname);
	Q3TextStream t( &f );// use a text stream
	if ( f.open(QIODevice::ReadOnly) ){
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		int i, rows = 1, cols = 0;
		int r = numRows();
		int c = numCols();
		for (i=0; i<ignoredLines; i++)
			t.readLine();

		QString s = t.readLine();//read first line after the ignored ones
		while (!t.atEnd()){
			t.readLine();
			rows++;
			qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
		}

		if (simplifySpaces)
			s = s.simplifyWhiteSpace();
		else if (stripSpaces)
			s = s.trimmed();
		QStringList line = s.split(sep);
		cols = (int)line.count();

		bool allNumbers = true;
		for (i=0; i<cols; i++)
		{//verify if the strings in the line used to rename the columns are not all numbers
			QLocale().toDouble(line[i], &allNumbers);
			if (!allNumbers)
				break;
		}

		if (renameCols && !allNumbers)
			rows--;

		QProgressDialog progress(this);
		int steps = int(rows/1000);
		progress.setRange(0, steps+1);
		progress.setWindowTitle("Reading file...");
		progress.setLabelText(fname);
		progress.setActiveWindow();

		QApplication::restoreOverrideCursor();

		if (!importFileAs)
			init (rows, cols);
		else if (importFileAs == 1){//new cols
			addColumns(cols);
			if (r < rows)
				d_table->setNumRows(rows);
		}
		else if (importFileAs == 2){//new rows
			if (c < cols)
				addColumns(cols-c);
			d_table->setNumRows(r+rows);
		}

		f.reset();
		for (i=0; i<ignoredLines; i++)
			t.readLine();

		int startRow = 0, startCol =0;
		if (importFileAs == 2)
			startRow = r;
		else if (importFileAs == 1)
			startCol = c;

		if (renameCols && !allNumbers)
		{//use first line to set the table header
			s = t.readLine();
			if (simplifySpaces)
				s = s.simplifyWhiteSpace();
			else if (stripSpaces)
				s = s.trimmed();

			line = s.split(sep, QString::SkipEmptyParts);
			int end = startCol+(int)line.count();
			for (i=startCol; i<end; i++)
				col_label[i] = QString::null;
			for (i=startCol; i<end; i++){
				comments[i] = line[i-startCol];
				s = line[i-startCol].replace("-","_").remove(QRegExp("\\W")).replace("_","-");
				int n = col_label.count(s);
				if(n){
					//avoid identical col names
					while (col_label.contains(s+QString::number(n)))
						n++;
					s += QString::number(n);
				}
				col_label[i] = s;
			}
		}
		d_table->blockSignals(true);
		setHeaderColType();

		for (i=0; i<steps; i++){
			if (progress.wasCanceled()){
				f.close();
				return;
			}

			for (int k=0; k<1000; k++){
				s = t.readLine();
				if (simplifySpaces)
					s = s.simplifyWhiteSpace();
				else if (stripSpaces)
					s = s.trimmed();
				line = s.split(sep);
				for (int j=startCol; j<numCols(); j++)
					setText(startRow + k, j, line[j-startCol]);
			}

			startRow += 1000;
			progress.setValue(i);
		}

		for (i=startRow; i<numRows(); i++){
			s = t.readLine();
			if (simplifySpaces)
				s = s.simplifyWhiteSpace();
			else if (stripSpaces)
				s = s.trimmed();
			line = s.split(sep);
			for (int j=startCol; j<numCols(); j++)
				setText(i, j, line[j-startCol]);
		}
		progress.setValue(steps+1);
		d_table->blockSignals(false);
		f.close();

		if (importFileAs)
		{
			for (i=0; i<numCols(); i++)
				emit modifiedData(this, colName(i));
		}
	}
#endif
}

void Table::importASCII(const QString &fname, const QString &sep, int ignoredLines,
		bool renameCols, bool stripSpaces, bool simplifySpaces, bool newTable)
{
	// TODO: port
#if 0
	QFile f(fname);
	if (f.open(QIODevice::ReadOnly)) //| QIODevice::Text | QIODevice::Unbuffered ))
	{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		Q3TextStream t(&f);//TODO: use QTextStream instead and find a way to make it read the end-of-line char correctly.
		//Opening the file with the above combination doesn't seem to help: problems on Mac OS X generated ASCII files!

		int i, c, rows = 1, cols = 0;
		for (i=0; i<ignoredLines; i++)
			t.readLine();

		QString s = t.readLine();//read first line after the ignored ones
		while ( !t.atEnd() ){
			t.readLine();
			rows++;
			qApp->processEvents(QEventLoop::ExcludeUserInput);
		}

		if (simplifySpaces)
			s = s.simplifyWhiteSpace();
		else if (stripSpaces)
			s = s.trimmed();

		QStringList line = s.split(sep);
		cols = (int)line.count();

		bool allNumbers = true;
		for (i=0; i<cols; i++)
		{//verify if the strings in the line used to rename the columns are not all numbers
			QLocale().toDouble(line[i], &allNumbers);
			if (!allNumbers)
				break;
		}

		if (renameCols && !allNumbers)
			rows--;
		int steps = int(rows/1000);

		QProgressDialog progress(this);
		progress.setWindowTitle("Reading file...");
		progress.setLabelText(fname);
		progress.setActiveWindow();
		progress.setAutoClose(true);
		progress.setAutoReset(true);
		progress.setRange(0, steps+1);

		QApplication::restoreOverrideCursor();

		QStringList oldHeader;
		if (newTable)
			init (rows, cols);
		else{
			if (numRows() != rows)
				d_table->setNumRows(rows);

			c = numCols();
			oldHeader = col_label;
			if (c != cols){
				if (c < cols)
					addColumns(cols - c);
				else{
					d_table->setNumCols(cols);
					for (int i=c-1; i>=cols; i--){
						emit removedCol(QString(name()) + "_" + oldHeader[i]);
						commands.removeLast();
						comments.removeLast();
						col_format.removeLast();
						col_label.removeLast();
						colTypes.removeLast();
						col_plot_type.removeLast();
					}
				}
			}
		}

		f.reset();
		for (i=0; i<ignoredLines; i++)
			t.readLine();

		if (renameCols && !allNumbers)
		{//use first line to set the table header
			s = t.readLine();
			if (simplifySpaces)
				s = s.simplifyWhiteSpace();
			else if (stripSpaces)
				s = s.trimmed();
			line = s.split(sep, QString::SkipEmptyParts);
			for (i=0; i<(int)line.count(); i++)
				col_label[i] = QString::null;

			for (i=0; i<(int)line.count(); i++)
			{
				comments[i] = line[i];
				s = line[i].replace("-","_").remove(QRegExp("\\W")).replace("_","-");
				int n = col_label.count(s);
				if(n)
				{
					//avoid identical col names
					while (col_label.contains(s+QString::number(n)))
						n++;
					s += QString::number(n);
				}
				col_label[i] = s;
			}
		}

		d_table->blockSignals(true);
		setHeaderColType();

		int start = 0;
		for (i=0; i<steps; i++)
		{
			if (progress.wasCanceled())
			{
				f.close();
				return;
			}

			start = i*1000;
			for (int k=0; k<1000; k++)
			{
				s = t.readLine();
				if (simplifySpaces)
					s = s.simplifyWhiteSpace();
				else if (stripSpaces)
					s = s.trimmed();
				line = s.split(sep);
				int lc = (int)line.count();
				if (lc > cols) {
					addColumns(lc - cols);
					cols = lc;
				}
				for (int j=0; j<cols && j<lc; j++)
					setText(start + k, j, line[j]);
			}
			progress.setValue(i);
			qApp->processEvents();
		}

		start = steps*1000;
		for (i=start; i<rows; i++)
		{
			s = t.readLine();
			if (simplifySpaces)
				s = s.simplifyWhiteSpace();
			else if (stripSpaces)
				s = s.trimmed();
			line = s.split(sep);
			int lc = (int)line.count();
			if (lc > cols) {
				addColumns(lc - cols);
				cols = lc;
			}
			for (int j=0; j<cols && j<lc; j++)
				setText(i, j, line[j]);
		}
		progress.setValue(steps+1);
		qApp->processEvents();
		d_table->blockSignals(false);
		f.close();

		if (!newTable)
		{
			if (cols > c)
				cols = c;
			for (i=0; i<cols; i++)
			{
				emit modifiedData(this, colName(i));
				if (colLabel(i) != oldHeader[i])
					emit changedColHeader(QString(name())+"_"+oldHeader[i],
							QString(name())+"_"+colLabel(i));
			}
		}
	}
#endif
}

bool Table::exportASCII(const QString& fname, const QString& separator,
		bool withLabels,bool exportSelection)
{
	// TODO: port
#if 0
	QFile f(fname);
	if ( !f.open( QIODevice::WriteOnly ) ){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(0, tr("ASCII Export Error"),
				tr("Could not write to file: <br><h4>"+fname+ "</h4><p>Please verify that you have the right to write to this location!").arg(fname));
		return false;
	}

	QString text;
	int i,j;
	int rows=numRows();
	int cols=numCols();
	int selectedCols = 0;
	int topRow = 0, bottomRow = 0;
	int *sCols;
	if (exportSelection){
		for (i=0; i<cols; i++){
			if (d_table->isColumnSelected(i))
				selectedCols++;
		}

		sCols = new int[selectedCols];
		int aux = 1;
		for (i=0; i<cols; i++){
			if (d_table->isColumnSelected(i)){
				sCols[aux] = i;
				aux++;
			}
		}

		for (i=0; i<rows; i++)
		{
			if (d_table->isRowSelected(i))
			{
				topRow = i;
				break;
			}
		}

		for (i=rows - 1; i>0; i--)
		{
			if (d_table->isRowSelected(i))
			{
				bottomRow = i;
				break;
			}
		}
	}

	if (withLabels)
	{
		QStringList header=colNames();
		QStringList ls=header.grep ( QRegExp ("\\D"));
		if (exportSelection)
		{
			for (i=1;i<selectedCols;i++)
			{
				if (ls.count()>0)
					text+=header[sCols[i]]+separator;
				else
					text+="C"+header[sCols[i]]+separator;
			}

			if (ls.count()>0)
				text+=header[sCols[selectedCols]] + "\n";
			else
				text+="C"+header[sCols[selectedCols]] + "\n";
		}
		else
		{
			if (ls.count()>0)
			{
				for (j=0; j<cols-1; j++)
					text+=header[j]+separator;
				text+=header[cols-1]+"\n";
			}
			else
			{
				for (j=0; j<cols-1; j++)
					text+="C"+header[j]+separator;
				text+="C"+header[cols-1]+"\n";
			}
		}
	}// finished writting labels

	if (exportSelection)
	{
		for (i=topRow;i<=bottomRow; i++)
		{
			for (j=1;j<selectedCols;j++)
				text+=text(i, sCols[j]) + separator;
			text+=text(i, sCols[selectedCols]) + "\n";
		}
		delete[] sCols;//free memory
	}
	else
	{
		for (i=0;i<rows;i++)
		{
			for (j=0;j<cols-1;j++)
				text+=text(i,j)+separator;
			text+=text(i,cols-1)+"\n";
		}
	}
	QTextStream t( &f );
	t << text;
	f.close();
#endif
	return true;
}

void Table::moveCurrentCell()
{
	// TODO: remove
#if 0
	int cols=numCols();
	int row=d_table->currentRow();
	int col=d_table->currentColumn();
	d_table->clearSelection (true);

	if (col+1<cols)
	{
		d_table->setCurrentCell(row, col+1);
		d_table->selectCells(row, col+1, row, col+1);
	}
	else
	{
		if(row+1 >= numRows())
			d_table->setNumRows(row + 11);

		d_table->setCurrentCell (row+1, 0);
		d_table->selectCells(row+1, 0, row+1, 0);
	}
#endif
}

void Table::customEvent(QEvent *e)
{
	if (e->type() == SCRIPTING_CHANGE_EVENT)
		scriptingChangeEvent((ScriptingChangeEvent*)e);
}

void Table::setNumRows(int rows)
{
	d_future_table->setRowCount(rows);
}

void Table::setNumCols(int cols)
{
	d_future_table->setColumnCount(cols);
}

void Table::resizeRows(int r)
{
	// TODO: obsolete
	int rows = numRows();
	if (rows == r)
		return;

	if (rows > r)
	{
		QString text= tr("Rows will be deleted from the table!");
		text+="<p>"+tr("Do you really want to continue?");
		int i,cols = numCols();
		switch( QMessageBox::information(this,tr("SciDAVis"), text, tr("Yes"), tr("Cancel"), 0, 1 ) )
		{
			case 0:
				QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
				setNumRows(r);
				for (i=0; i<cols; i++)
					emit modifiedData(this, colName(i));

				QApplication::restoreOverrideCursor();
				break;

			case 1:
				return;
				break;
		}
	}
	else
	{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		setNumRows(r);
		QApplication::restoreOverrideCursor();
	}

	emit modifiedWindow(this);
}

void Table::resizeCols(int c)
{
	// TODO: obsolete
#if 0
	int cols = numCols();
	if (cols == c)
		return;

	if (cols > c){
		QString text= tr("Columns will be deleted from the table!");
		text+="<p>"+tr("Do you really want to continue?");
		switch( QMessageBox::information(this,tr("SciDAVis"), text, tr("Yes"), tr("Cancel"), 0, 1 ) ){
			case 0: {
						QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
						Q3MemArray<int> columns(cols-c);
						for (int i=cols-1; i>=c; i--){
							QString name = colName(i);
							emit removedCol(name);
							columns[i-c]=i;

							commands.removeLast();
							comments.removeLast();
							col_format.removeLast();
							col_label.removeLast();
							colTypes.removeLast();
							col_plot_type.removeLast();
						}

						d_table->removeColumns(columns);
						QApplication::restoreOverrideCursor();
						break;
					}

			case 1:
					return;
					break;
		}
	}
	else{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		addColumns(c-cols);
		setHeaderColType();
		QApplication::restoreOverrideCursor();
	}
	emit modifiedWindow(this);
#endif
}

void Table::copy(Table *m)
{
	// TODO: port
#if 0
	for (int i=0; i<numRows(); i++)
	{
		for (int j=0; j<numCols(); j++)
			setText(i,j,m->text(i,j));
	}

	setColWidths(m->columnWidths());
	col_label = m->colNames();
	col_plot_type = m->plotDesignations();
	d_show_comments = m->commentsEnabled();
	comments = m->colComments();
	setHeaderColType();

	commands = m->getCommands();
	setColumnTypes(m->columnTypes());
	col_format = m->getColumnsFormat();
#endif
}

QString Table::saveAsTemplate(const QString& geometryInfo)
{
	QString s="<table>\t"+QString::number(numRows())+"\t";
	s+=QString::number(numCols())+"\n";
	s+=geometryInfo;
	s+=saveHeader();
	s+=saveColumnWidths();
	s+=saveCommands();
	s+=saveColumnTypes();
	s+=saveComments();
	return s;
}

void Table::restore(const QStringList& lst)
{
	// TODO: ...
#if 0
	QStringList l;
	QStringList::const_iterator i=lst.begin();

	l= (*i++).split("\t");
	l.remove(l.first());
	importV0x0001XXHeader(l);

	setColWidths((*i).right((*i).length()-9).split("\t", QString::SkipEmptyParts));
	i++;

	l = (*i++).split("\t");
	if (l[0] == "com")
	{
		l.remove(l.first());
		setCommands(l);
	} else if (l[0] == "<com>") {
		commands.clear();
		for (int col=0; col<numCols(); col++)
			commands << "";
		for (; i != lst.end() && *i != "</com>"; i++)
		{
			int col = (*i).mid(9,(*i).length()-11).toInt();
			QString formula;
			for (i++; i!=lst.end() && *i != "</col>"; i++)
				formula += *i + "\n";
			formula.truncate(formula.length()-1);
			commands[col] = formula;
		}
		i++;
	}

	l = (*i++).split("\t");
	l.remove(l.first());
	setColumnTypes(l);

	l = (*i++).split("\t");
	l.remove(l.first());
	setColComments(l);
#endif
}

void Table::clear()
{
	d_future_table->clear();
}

void Table::setColumnHeader(int index, const QString& label)
{
	// TODO: remove
#if 0
	Q3Header *head = d_table->horizontalHeader();

	if (d_show_comments)
	{
		QString s = label;

		int lines = d_table->columnWidth(index)/d_table->horizontalHeader()->fontMetrics().averageCharWidth();
		head->setLabel(index, s.remove("\n") + "\n" + QString(lines, '_') + "\n" + comments[index]);
	}
	else
		head->setLabel(index, label);
#endif
}

void Table::setNumericPrecision(int prec)
{
	// TODO: obsolete
#if 0
	d_numeric_precision = prec;
	for (int i=0; i<numCols(); i++)
		col_format[i] = "0/"+QString::number(prec);
#endif
}

QStringList Table::colNames()
{
	QStringList list;
	for (int i=0; i<d_future_table->columnCount(); i++)
		list << column(i)->name();
	return list;
}

QString Table::colLabel(int col)
{
	return column(col)->name();
}

SciDAVis::PlotDesignation Table::colPlotDesignation(int col)
{
	return column(col)->plotDesignation();
}

void Table::setColPlotDesignation(int col, SciDAVis::PlotDesignation d)
{
	column(col)->setPlotDesignation(d);
}

QList<int> Table::plotDesignations()
{
	QList<int> list;
	for (int i=0; i<d_future_table->columnCount(); i++)
		list << column(i)->plotDesignation();
	return list;
}

QStringList Table::getCommands()
{
	QStringList list;
	if (d_future_table->rowCount() < 1) 
		return list;
	for (int i=0; i<d_future_table->columnCount(); i++)
		list << column(i)->formula(0);
	return list;
}

QList<int> Table::columnTypes()
{
	QList<int> list;
	for (int i=0; i<d_future_table->columnCount(); i++)
		list << column(i)->columnMode();
	return list;
}

int Table::columnType(int col)
{
	return column(col)->columnMode();
}

void Table::setColumnTypes(QList<int> ctl)
{
	Q_ASSERT(ctl.size() == d_future_table->columnCount());
	for (int i=0; i<d_future_table->columnCount(); i++)
		column(i)->setColumnMode((SciDAVis::ColumnMode)ctl.at(i));
}

void Table::setColumnType(int col, SciDAVis::ColumnMode mode) 
{ 
	column(col)->setColumnMode(mode);
}

QString Table::columnFormat(int col)
{
	// TODO: obsolete
	return QString();
}

QStringList Table::getColumnsFormat()
{
	// TODO: obsolete
	return QStringList();
}

int Table::verticalHeaderWidth()
{
	return d_view_widget->verticalHeader()->width();
}

QString Table::colComment(int col)
{
	return column(col)->comment();
}

QStringList Table::colComments()
{
	QStringList list;
	for (int i=0; i<d_future_table->columnCount(); i++)
		list << column(i)->comment();
	return list;
}

void Table::setColComments(const QStringList& list)
{
	for (int i=0; i<d_future_table->columnCount(); i++)
		column(i)->setComment(list.at(i));
}

bool Table::commentsEnabled()
{
	return areCommentsShown();
}

