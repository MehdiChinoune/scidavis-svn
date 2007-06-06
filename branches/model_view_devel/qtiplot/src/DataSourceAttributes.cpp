/***************************************************************************
    File                 : DataSourceAttributes.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Hoener zu Siederdissen,
    Email (use @ for *)  : thzs*gmx.net
    Description          : Base class for data sources supporting interval-based attributes

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

#include "DataSourceAttributes.h"

bool DataSourceAttributes::isRowValid(int row) const 
{ 
	foreach(Interval iv, d_valid_intervals)
		if(iv.contains(row))
			return true;
	return false;
}

bool DataSourceAttributes::isValid(Interval i) const
{ 
	foreach(Interval iv, d_valid_intervals)
		if(iv.contains(i))
			return true;
	return false;
}

bool DataSourceAttributes::isSelected(int row) const 
{ 
	foreach(Interval iv, d_selected_intervals)
		if(iv.contains(row))
			return true;
	return false; 
}

QList<Interval> DataSourceAttributes::selectedIntervals() const 
{ 
	return d_selected_intervals; 
}

QString DataSourceAttributes::formula(int row) const 
{ 
	for(int c=d_formula_intervals.size()-1; c>=0; c--)
	{
		// find the last formula used for that row
		if(d_formula_intervals.at(c).contains(row))
			return d_formulas.at(c);
	}
	return QString(); 
}

bool DataSourceAttributes::isRowMasked(int row) const
{ 
	foreach(Interval iv, d_masked_intervals)
		if(iv.contains(row))
			return true;
	return false;
}

bool DataSourceAttributes::isMasked(Interval i) const
{ 
	foreach(Interval iv, d_masked_intervals)
		if(iv.contains(i))
			return true;
	return false;
}

void DataSourceAttributes::clearValidity() 
{ 
	d_valid_intervals.clear(); 
}

void DataSourceAttributes::clearSelections() 
{ 
	d_selected_intervals.clear(); 
}

void DataSourceAttributes::clearMasks() 
{ 
	d_masked_intervals.clear(); 
}

void DataSourceAttributes::clearFormulas() 
{ 
	d_formula_intervals.clear(); 
	d_formulas.clear(); 
}

void DataSourceAttributes::setValid(Interval i, bool valid)
{ 
	if(valid)
	{
		foreach(Interval iv, d_valid_intervals)
			if(iv.contains(i)) 
				return;

		Interval::mergeIntervalIntoList(&d_valid_intervals, i);
	}
	else // set invalid
	{
		Interval::subtractIntervalFromList(&d_valid_intervals, i);
	}
}

void DataSourceAttributes::setSelected(Interval i, bool select)
{ 
	if(select)
	{
		foreach(Interval iv, d_selected_intervals)
			if(iv.contains(i)) 
				return;

		Interval::mergeIntervalIntoList(&d_selected_intervals, i);
	}
	else // deselect
	{
		Interval::subtractIntervalFromList(&d_selected_intervals, i);
	}
}

void DataSourceAttributes::setMasked(Interval i, bool mask)
{ 
	if(mask)
	{
		foreach(Interval iv, d_masked_intervals)
			if(iv.contains(i)) 
				return;

		Interval::mergeIntervalIntoList(&d_masked_intervals, i);
	}
	else // unmask
	{
		Interval::subtractIntervalFromList(&d_masked_intervals, i);
	}
}

void DataSourceAttributes::setFormula(Interval i, QString formula)
{
	// first: subtract the new interval from all others
	QList<Interval> temp_list;
	for(int c=0; c<d_formula_intervals.size(); c++)
	{
		temp_list = Intervall::subtract(d_formula_intervals.at(c), i);
		if(temp_list.isEmpty())
		{
			d_formula_intervals.removeAt(c);
			d_formulas.removeAt(c--);
		}
		else 
		{
			d_formula_intervals.replace(c, temp_list.at(0));
			if(temp_list.size()>1)
			{
				d_formula_intervals.insert(c, temp_list.at(1));
				d_formulas.insert(c, d_formulas.at(c));
			}
		}
	}
	
	// second: try to merge the new interval with an old one 
	for(int c=0; c<d_formula_intervals.size(); c++)
	{
		if( d_formula_intervals.at(c).touches(i) &&
			d_formulas.at(c) == formula )
		{
			d_formula_intervals.replace(c, Interval::merge(d_formula_intervals.at(c), i));
			return;
		}
	}
	// if it could not be merged, just append it
	d_formula_intervals.append(i);
	d_formulas.append(formula);
}


