/***************************************************************************
	File                 : TableStatistics.cpp
	Project              : SciDAVis
--------------------------------------------------------------------
	Copyright            : (C) 2006 by Knut Franke
	Email (use @ for *)  : knut.franke*gmx.de
	Description          : Table subclass that displays statistics on
	                       columns or rows of another table

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
#include "TableStatistics.h"
#include "table/TableView.h"

#include <QList>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>

TableStatistics::TableStatistics(AbstractScriptingEngine *engine, QWidget *parent, Table *base, Type t, QList<int> targets)
	: Table(engine, 1, 1, "", parent, ""),
	m_base(base), m_type(t), m_targets(targets)
{
	// FIXME: Haven't found a set read-only method in Qt4 yet
	// m_table->setReadOnly(true);
	setCaptionPolicy(MyWidget::Both);
	if (m_type == row)
	{
		setName(QString(m_base->name())+"-"+tr("RowStats"));
		setWindowLabel(tr("Row Statistics of %1").arg(base->name()));
		setRowCount(m_targets.size());
		setColumnCount(9);
		setColName(0, tr("Row"));
		setColName(1, tr("Cols"));
		setColName(2, tr("Mean"));
		setColName(3, tr("StandardDev"));
		setColName(4, tr("Variance"));
		setColName(5, tr("Sum"));
		setColName(6, tr("Max"));
		setColName(7, tr("Min"));
		setColName(8, "N");

		// TODO
		//for (int i=0; i < 9; i++)
		//	setColumnType(i, Text);

		for (int i=0; i < m_targets.size(); i++)
			setText(i, 0, QString::number(m_targets[i]+1));
		update(m_base, QString::null);
	}
	else if (m_type == column)
	{
		setName(QString(m_base->name())+"-"+tr("ColStats"));
		setWindowLabel(tr("Column Statistics of %1").arg(base->name()));
		setRowCount(m_targets.size());
		setColumnCount(11);
		setColName(0, tr("Col"));
		setColName(1, tr("Rows"));
		setColName(2, tr("Mean"));
		setColName(3, tr("StandardDev"));
		setColName(4, tr("Variance"));
		setColName(5, tr("Sum"));
		setColName(6, tr("iMax"));
		setColName(7, tr("Max"));
		setColName(8, tr("iMin"));
		setColName(9, tr("Min"));
		setColName(10, "N");

		// TODO
		//for (int i=0; i < 11; i++)
		//	setColumnType(i, Text);

		for (int i=0; i < m_targets.size(); i++)
		{
			setText(i, 0, m_base->colLabel(m_targets[i]));
			update(m_base, m_base->colName(m_targets[i]));
		}
	}
	int w=9*(m_table_view->horizontalHeader())->sectionSize(0);
	int h;
	if (rowCount()>11)
		h=11*(m_table_view->verticalHeader())->sectionSize(0);
	else
		h=(rowCount()+1)*(m_table_view->verticalHeader())->sectionSize(0);
	setGeometry(50,50,w + 45, h + 45);

	setColPlotDesignation(0, SciDAVis::X);
	// TODO
	//setHeaderColType();
}

void TableStatistics::update(Table *t, const QString& colName)
{
	if (t != m_base) return;

	int j;
	if (m_type == row)
		for (int r=0; r < m_targets.size(); r++)
		{
			int cols=m_base->columnCount();
			int i = m_targets[r];
			int m = 0;
			for (j = 0; j < cols; j++)
				if (!m_base->text(i, j).isEmpty() && m_base->columnType(j) == SciDAVis::Numeric)
					m++;

			if (!m)
			{//clear row statistics
				for (j = 1; j<9; j++)
					setText(r, j, QString::null);
			}

			if (m > 0)
			{
				double *dat = new double[m];
				gsl_vector *y = gsl_vector_alloc (m);
				int aux = 0;
				for (j = 0; j<cols; j++)
				{
					QString text = m_base->text(i,j);
					if (!text.isEmpty() && m_base->columnType(j) == SciDAVis::Numeric)
					{					
						double val = m_base->cell(i,j);
						gsl_vector_set (y, aux, val);
						dat[aux] = val;
						aux++;
					}
				}
				double mean = gsl_stats_mean (dat, 1, m);
				double min, max;
				gsl_vector_minmax (y, &min, &max);

				setText(r, 1, QString::number(m_base->columnCount()));
				setText(r, 2, QString::number(mean));
				setText(r, 3, QString::number(gsl_stats_sd(dat, 1, m)));
				setText(r, 4, QString::number(gsl_stats_variance(dat, 1, m)));
				setText(r, 5, QString::number(mean*m));
				setText(r, 6, QString::number(max));
				setText(r, 7, QString::number(min));
				setText(r, 8, QString::number(m));

				gsl_vector_free (y);
				delete[] dat;
			}
		}
	else if (m_type == column)
		for (int c=0; c < m_targets.size(); c++)
			if (colName == QString(m_base->name())+"_"+text(c, 0))
			{
				int i = m_base->colIndex(colName);
				if (m_base->columnType(i) != SciDAVis::Numeric) return;

				int rows = m_base->rowCount();
				int start = -1, m = 0;
				for (j=0; j<rows; j++)
					if (!m_base->text(j,i).isEmpty())
					{
						m++;
						if (start<0) start=j;
					}

				if (!m)
				{//clear col statistics
					for (j = 1; j<11; j++)
						setText(c, j, QString::null);
					return;
				}

				if (start<0) return;

				double *dat = new double[m];
				gsl_vector *y = gsl_vector_alloc (m);

				int aux = 0, min_index = start, max_index = start;
				double val = m_base->cell(start, i);
				gsl_vector_set (y, 0, val);
				dat[0] = val;
				double min = val, max = val;
				for (j = start + 1; j<rows; j++)
				{
					if (!m_base->text(j, i).isEmpty())
					{
						aux++;
						val = m_base->cell(j, i);
						gsl_vector_set (y, aux, val);
						dat[aux] = val;
						if (val < min)
						{
							min = val;
							min_index = j;
						}
						if (val > max)
						{
							max = val;
							max_index = j;
						}
					}
				}
				double mean=gsl_stats_mean (dat, 1, m);

				setText(c, 1, "[1:"+QString::number(rows)+"]");
				setText(c, 2, QString::number(mean));
				setText(c, 3, QString::number(gsl_stats_sd(dat, 1, m)));
				setText(c, 4, QString::number(gsl_stats_variance(dat, 1, m)));
				setText(c, 5, QString::number(mean*m));
				setText(c, 6, QString::number(max_index + 1));
				setText(c, 7, QString::number(max));
				setText(c, 8, QString::number(min_index + 1));
				setText(c, 9, QString::number(min));
				setText(c, 10, QString::number(m));

				gsl_vector_free (y);
				delete[] dat;
			}

	// TODO
	/*
	for (int i=0; i<columnCount(); i++)
		emit modifiedData(this, Table::colName(i));
	*/
}

void TableStatistics::renameCol(const QString &from, const QString &to)
{
	if (m_type == row) return;
	for (int c=0; c < m_targets.size(); c++)
		if (from == QString(m_base->name())+"_"+text(c, 0))
		{
			setText(c, 0, to.section('_', 1, 1));
			return;
		}
}

void TableStatistics::removeCol(const QString &col)
{
	/* TODO
	if (m_type == row)
	{
		update(m_base, col);
		return;
	}
	for (int c=0; c < m_targets.size(); c++)
		if (col == QString(m_base->name())+"_"+text(c, 0))
		{
			m_targets.remove(m_targets.at(c));
			m_table->removeRow(c);
			return;
		}
	*/
}

QString TableStatistics::saveToString(const QString &geometry)
{
	/* TODO
	QString s = "<TableStatistics>\n";
	s += QString(name())+"\t";
	s += QString(m_base->name())+"\t";
	s += QString(m_type == row ? "row" : "col") + "\t";
	s += birthDate()+"\n";
	s += "Targets";
	for (QList<int>::iterator i=m_targets.begin(); i!=m_targets.end(); ++i)
		s += "\t" + QString::number(*i);
	s += "\n";
	s += geometry;
	s += saveHeader();
	s += saveColumnWidths();
	s += saveCommands();
	s += saveColumnTypes();
	s += saveComments();
	s += "WindowLabel\t" + windowLabel() + "\t" + QString::number(captionPolicy()) + "\n";
	return s + "</TableStatistics>\n";
	*/
}
