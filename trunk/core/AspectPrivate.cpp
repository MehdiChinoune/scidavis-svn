/***************************************************************************
    File                 : AspectPrivate.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Private data managed by AbstractAspect.

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
#include "AbstractAspect.h"
#include "AspectPrivate.h"
#include <QRegExp>

AbstractAspect::Private::Private(AbstractAspect * owner, const QString& name)
	: d_name(name), d_caption_spec("%n%C{ - }%c"), d_owner(owner), d_parent(0)
{
	d_creation_time = QDateTime::currentDateTime();
}

AbstractAspect::Private::~Private()
{
	foreach(AbstractAspect * child, d_children)
		delete child;
}

void AbstractAspect::Private::addChild(AbstractAspect* child)
{
	insertChild(d_children.count(), child);
}

void AbstractAspect::Private::insertChild(int index, AbstractAspect* child)
{
	emit d_owner->aspectAboutToBeAdded(d_owner, index);
	d_children.insert(index, child);
	// Always remove from any previous parent before adding to a new one!
	// Can't handle this case here since two undo commands have to be created.
	Q_ASSERT(child->d_aspect_private->d_parent == 0);
	child->d_aspect_private->d_parent = d_owner;
	QObject::connect(child, SIGNAL(aspectDescriptionChanged(AbstractAspect *)), 
			d_owner, SIGNAL(aspectDescriptionChanged(AbstractAspect *)));
	QObject::connect(child, SIGNAL(aspectAboutToBeAdded(AbstractAspect *, int)), 
			d_owner, SIGNAL(aspectAboutToBeAdded(AbstractAspect *, int)));
	QObject::connect(child, SIGNAL(aspectAboutToBeRemoved(AbstractAspect *, int)), 
			d_owner, SIGNAL(aspectAboutToBeRemoved(AbstractAspect *, int)));
	QObject::connect(child, SIGNAL(aspectAdded(AbstractAspect *, int)), 
			d_owner, SIGNAL(aspectAdded(AbstractAspect *, int)));
	QObject::connect(child, SIGNAL(aspectRemoved(AbstractAspect *, int)), 
			d_owner, SIGNAL(aspectRemoved(AbstractAspect *, int)));
	QObject::connect(child, SIGNAL(aspectAboutToBeRemoved(AbstractAspect *)), 
			d_owner, SIGNAL(aspectAboutToBeRemoved(AbstractAspect *)));
	QObject::connect(child, SIGNAL(aspectAdded(AbstractAspect *)), 
			d_owner, SIGNAL(aspectAdded(AbstractAspect *)));
	emit d_owner->aspectAdded(d_owner, index);
	emit child->aspectAdded(child);
}

int AbstractAspect::Private::indexOfChild(const AbstractAspect *child) const
{
	for(int i=0; i<d_children.size(); i++)
		if(d_children.at(i) == child) return i;
	return -1;
}

int AbstractAspect::Private::removeChild(AbstractAspect* child)
{
	int index = indexOfChild(child);
	Q_ASSERT(index != -1);
	emit d_owner->aspectAboutToBeRemoved(d_owner, index);
	emit child->aspectAboutToBeRemoved(child);
	d_children.removeAll(child);
	QObject::disconnect(child, 0, d_owner, 0);
	child->d_aspect_private->d_parent = 0;
	emit d_owner->aspectRemoved(d_owner, index);
	return index;
}

int AbstractAspect::Private::childCount() const
{
	return d_children.count();
}

AbstractAspect* AbstractAspect::Private::child(int index)
{
	Q_ASSERT(index >= 0 && index <= childCount());
	return d_children.at(index);
}

QString AbstractAspect::Private::name() const
{
	return d_name;
}

void AbstractAspect::Private::setName(const QString &value)
{
	emit d_owner->aspectDescriptionAboutToChange(d_owner);
	d_name = value;
	emit d_owner->aspectDescriptionChanged(d_owner);
}

QString AbstractAspect::Private::comment() const
{
	return d_comment;
}

void AbstractAspect::Private::setComment(const QString &value)
{
	emit d_owner->aspectDescriptionAboutToChange(d_owner);
	d_comment = value;
	emit d_owner->aspectDescriptionChanged(d_owner);
}

QString AbstractAspect::Private::captionSpec() const
{
	return d_caption_spec;
}

void AbstractAspect::Private::setCaptionSpec(const QString &value)
{
	emit d_owner->aspectDescriptionAboutToChange(d_owner);
	d_caption_spec = value;
	emit d_owner->aspectDescriptionChanged(d_owner);
}

void AbstractAspect::Private::setCreationTime(const QDateTime &time)
{
	d_creation_time = time;
}

int AbstractAspect::Private::indexOfMatchingBrace(const QString &str, int start)
{
	int result = str.indexOf('}', start);
	if (result < 0)
		result = start;
	return result;
}

QString AbstractAspect::Private::caption() const
{
	QString result = d_caption_spec;
	QRegExp magic("%(.)");
	for(int pos=magic.indexIn(result, 0); pos >= 0; pos=magic.indexIn(result, pos)) {
		QString replacement;
		int length;
		switch(magic.cap(1).at(0).toAscii()) {
			case '%': replacement = "%"; length=2; break;
			case 'n': replacement = d_name; length=2; break;
			case 'c': replacement = d_comment; length=2; break;
			case 't': replacement = d_creation_time.toString(); length=2; break;
			case 'C':
						 length = indexOfMatchingBrace(result, pos) - pos + 1;
						 replacement = d_comment.isEmpty() ? "" : result.mid(pos+3, length-4);
						 break;
		}
		result.replace(pos, length, replacement);
		pos += replacement.size();
	}
	return result;
}

QDateTime AbstractAspect::Private::creationTime() const
{
	return d_creation_time;
}

