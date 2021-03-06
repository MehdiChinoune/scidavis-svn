/***************************************************************************
    File                 : IntervalAttribute.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : A class representing an interval-based attribute

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
 *   along with m_intervals program; if not, write to the Free Software    *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef INTERVALATTRIBUTE_H
#define INTERVALATTRIBUTE_H

#include "Interval.h"
#include <QList>

//! A class representing an interval-based attribute
template<class T> class IntervalAttribute
{
	public:
		void setValue(Interval<int> i, T value) 
		{
			// first: subtract the new interval from all others
			QList< Interval<int> > temp_list;
			for(int c=0; c<m_intervals.size(); c++)
			{
				temp_list = Interval<int>::subtract(m_intervals.at(c), i);
				if(temp_list.isEmpty())
				{
					m_intervals.removeAt(c);
					m_values.removeAt(c--);
				}
				else 
				{
					m_intervals.replace(c, temp_list.at(0));
					if(temp_list.size()>1)
					{
						m_intervals.insert(c, temp_list.at(1));
						m_values.insert(c, m_values.at(c));
					}
				}
			}

			// second: try to merge the new interval with an old one 
			for(int c=0; c<m_intervals.size(); c++)
			{
				if( m_intervals.at(c).touches(i) &&
						m_values.at(c) == value )
				{
					m_intervals.replace(c, Interval<int>::merge(m_intervals.at(c), i));
					return;
				}
			}
			// if it could not be merged, just append it
			m_intervals.append(i);
			m_values.append(value);
		}

		// overloaded for convenience
		void setValue(int row, T value) 
		{
			setValue(Interval<int>(row, row), value);
		}

		T value(int row) const 
		{
			for(int c=m_intervals.size()-1; c>=0; c--)
			{
				if(m_intervals.at(c).contains(row))
					return m_values.at(c);
			}
			return T(); 
		}

		void insertRows(int before, int count)
		{
			QList< Interval<int> > temp_list;
			// first: split all intervals that contain 'before'
			for(int c=0; c<m_intervals.size(); c++)
			{
				if(m_intervals.at(c).contains(before))
				{
					temp_list = Interval<int>::split(m_intervals.at(c), before);
					m_intervals.replace(c, temp_list.at(0));
					if(temp_list.size()>1)
					{
						m_intervals.insert(c, temp_list.at(1));
						m_values.insert(c, m_values.at(c));
						c++;
					}
					
				}
			}
			// second: translate all intervals that start at 'before' or later
			for(int c=0; c<m_intervals.size(); c++)
			{
				if(m_intervals.at(c).start() >= before)
					m_intervals[c].translate(count);
			}
			
		}

		void removeRows(int first, int count)
		{
			QList< Interval<int> > temp_list;
			Interval<int> i(first, first+count-1);
			// first: remove the relevant rows from all intervals
			for(int c=0; c<m_intervals.size(); c++)
			{
				temp_list = Interval<int>::subtract(m_intervals.at(c), i);
				if(temp_list.isEmpty())
				{
					m_intervals.removeAt(c);
					m_values.removeAt(c--);
				}
				else 
				{
					m_intervals.replace(c, temp_list.at(0));
					if(temp_list.size()>1)
					{
						m_intervals.insert(c, temp_list.at(1));
						m_values.insert(c, m_values.at(c));
						c++;
					}
				}
			}
			// second: translate all intervals that start at 'first+count' or later
			for(int c=0; c<m_intervals.size(); c++)
			{
				if(m_intervals.at(c).start() >= first+count)
					m_intervals[c].translate(-count);
			}
			// third: merge as many intervals as possible
			QList<T> values_copy = m_values;
			QList< Interval<int> > intervals_copy = m_intervals;
			m_values.clear();
			m_intervals.clear();
			for(int c=0; c<intervals_copy.size(); c++)
			{
				i = intervals_copy.at(c);
				T value = values_copy.at(c);
				for(int cc=0; cc<m_intervals.size(); cc++)
				{
					if( m_intervals.at(cc).touches(i) &&
							m_values.at(cc) == value )
					{
						m_intervals.replace(cc, Interval<int>::merge(m_intervals.at(cc),i));
						return;
					}
				}
				// if it could not be merged, just append it
				m_intervals.append(i);
				m_values.append(value);
			}
		}

		void clear() { m_values.clear(); m_intervals.clear(); }

		QList< Interval<int> > intervals() const { return m_intervals; }
		QList<T> values() const { return m_values; }
		IntervalAttribute<T>& operator=(const IntervalAttribute<T>& other) 
		{
			m_intervals.clear();
			m_values.clear();
			foreach( Interval<int> iv, other.intervals())
				m_intervals.append(iv);
			foreach( T value, other.values())
				m_values.append(value);
			return *this;
		}

	private:
		QList<T> m_values;
		QList< Interval<int> > m_intervals;
};

//! A class representing an interval-based attribute (bool version)
template<> class IntervalAttribute<bool>
{
	public:
		IntervalAttribute<bool>() {}
		IntervalAttribute<bool>(QList< Interval<int> > intervals) : m_intervals(intervals) {}
		IntervalAttribute<bool>& operator=(const IntervalAttribute<bool>& other) 
		{
			m_intervals.clear();
			foreach( Interval<int> iv, other.intervals())
				m_intervals.append(iv);
			return *this;
		}

		void setValue(Interval<int> i, bool value=true) 
		{
			if(value) 
			{
				foreach(Interval<int> iv, m_intervals)
					if(iv.contains(i)) 
						return;

				Interval<int>::mergeIntervalIntoList(&m_intervals, i);
			} else { // unset
				Interval<int>::subtractIntervalFromList(&m_intervals, i);
			}
		}

		void setValue(int row, bool value) 
		{
			setValue(Interval<int>(row, row), value);
		}

		bool isSet(int row) const 
		{
			foreach(Interval<int> iv, m_intervals)
				if(iv.contains(row))
					return true;
			return false;
		}

		bool isSet(Interval<int> i) const 
		{
			foreach(Interval<int> iv, m_intervals)
				if(iv.contains(i))
					return true;
			return false;
		}

		void insertRows(int before, int count)
		{
			QList< Interval<int> > temp_list;
			int c;
			// first: split all intervals that contain 'before'
			for(c=0; c<m_intervals.size(); c++)
			{
				if(m_intervals.at(c).contains(before))
				{
					temp_list = Interval<int>::split(m_intervals.at(c), before);
					m_intervals.replace(c, temp_list.at(0));
					if(temp_list.size()>1)
						m_intervals.insert(c++, temp_list.at(1));
					
				}
			}
			// second: translate all intervals that start at 'before' or later
			for(c=0; c<m_intervals.size(); c++)
			{
				if(m_intervals.at(c).start() >= before)
					m_intervals[c].translate(count);
			}
			
		}

		void removeRows(int first, int count)
		{
			int c;
			// first: remove the relevant rows from all intervals
			Interval<int>::subtractIntervalFromList(&m_intervals, Interval<int>(first, first+count-1));
			// second: translate all intervals that start at 'first+count' or later
			for(c=0; c<m_intervals.size(); c++)
			{
				if(m_intervals.at(c).start() >= first+count)
					m_intervals[c].translate(-count);
			}
			// third: merge as many intervals as possible
			for(c=m_intervals.size()-1; c>=0; c--)
			{
				Interval<int> iv = m_intervals.takeAt(c);
				int size_before = m_intervals.size();
				Interval<int>::mergeIntervalIntoList(&m_intervals, iv);
				if(size_before == m_intervals.size()) // merge successful
					c--;
			}
		}

		QList< Interval<int> > intervals() const { return m_intervals; }

		void clear() { m_intervals.clear(); }

	private:
		QList< Interval<int> > m_intervals;
};

#endif
