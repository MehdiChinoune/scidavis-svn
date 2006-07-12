/***************************************************************************
    File                 : scaleDraws.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email                : ion_vasilief@yahoo.fr, thzs@gmx.net
    Description          : Extension to QwtScaleDraw
                           
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
#ifndef SCALEDRAWS_H
#define SCALEDRAWS_H

#include "parser.h"

#include <qmessagebox.h>
#include <qpainter.h>
#include <qdatetime.h>

#include <qwt_painter.h>
#include <qwt_scale_draw.h>
#include <qwt_text.h>

//FIXME: All functionality disabled for now (needs port to Qwt5)

//! Extension to QwtScaleDraw
class ScaleDraw: public QwtScaleDraw
{
	public:		
		ScaleDraw(int width)
		{
		#if false
			d_lineWidth = width;
			formula_string = QString();
		#endif
		};
		ScaleDraw()
		{
		#if false
			d_lineWidth = 1;
			formula_string = QString();
		#endif
		};
		virtual ~ScaleDraw(){};

		QString formulaString() {return formula_string;};
		void setFormulaString(const QString& formula) {formula_string = formula;};

		virtual QwtText label(double value) const
		{
		#if false
			if (!formula_string.isEmpty())
			{
				double lbl=0.0;
				try
				{
					MyParser parser;
					if (formula_string.contains("x"))
						parser.DefineVar("x", &value);
					else if (formula_string.contains("y"))
						parser.DefineVar("y", &value);

					parser.SetExpr(formula_string.ascii());
					lbl=parser.Eval();
				}
				catch(mu::ParserError &)
				{
					return QString();
				}
				return QwtScaleDraw::label(lbl);
			}
			else
				return QwtScaleDraw::label(value);
		#endif
		};

		virtual void draw(QPainter *p) const
		{
		#if false
			p->save();

			QPen pen = p->pen();
			pen.setWidth(d_lineWidth);
			p->setPen(pen);

			QwtScaleDraw::draw(p);	

			p->restore();
		#endif
		};

		void drawTick(QPainter *p, double val, int len) const
		{
		#if false
			bool print = p->device()->isExtDev();

			if (options() && !print)
				QwtScaleDraw::drawTick(p, val, len);
			else if (!options())
			{//axes without backbone
				if ( len <= 0 )
					return;

				const int tval = transform(val);
				const int xorg = x();
				const int yorg = y();	

				switch(orientation())
				{
					case Qt::DockLeft:
						QwtPainter::drawLine(p, xorg + 1, tval, xorg - len, tval);
						break;

					case Qt::DockRight:
						QwtPainter::drawLine(p, xorg, tval, xorg + len, tval);
						break;

					case Qt::DockBottom: 
						QwtPainter::drawLine(p, tval, yorg, tval, yorg + len);
						break;

					case Qt::DockTop:
						QwtPainter::drawLine(p, tval, yorg + 1, tval, yorg - len);
						break;

					default:
						break;
				}
			}
			else if (print)
			{
				if ( len <= 0 )
					return;

				const int tval = transform(val);
				const int xorg = x();
				const int yorg = y();	

				switch(orientation())
				{
					case Qt::DockLeft:
						{
							if (!options())
								QwtPainter::drawLine(p, xorg + 1, tval, xorg - len, tval);
							else
								QwtPainter::drawLine(p, xorg, tval, xorg - len, tval);
							break;
						}

					case Qt::DockRight:
						{
							if (!options())
								QwtPainter::drawLine(p, xorg, tval, xorg + len, tval);
							else
							{
								const int bw = QMAX (p->pen().width() / 2, 1);
								QwtPainter::drawLine(p, xorg + bw, tval, xorg + len, tval);
							}
							break;
						}

					case Qt::DockBottom: 
						{
							const int bw = p->pen().width() / 2;
							QwtPainter::drawLine(p, tval, yorg + bw, tval, yorg + len);
							break;
						}

					case Qt::DockTop:
						QwtPainter::drawLine(p, tval, yorg, tval, yorg - len);
						break;

					default:
						break;
				}
			}
		#endif
		};

		void drawBackbone(QPainter *p) const
		{   
		#if false
			const int pw = p->pen().width();
			const int pw2 = p->pen().width() % 2;

			if (pw ==1 || !pw2)
			{
				const int xorg = x();
				const int yorg = y();
				const int l = length();

				switch(orientation())
				{
					case Qt::DockLeft:
						{
							const int bw2 = (pw - 1) / 2;
							QwtPainter::drawLine(p, xorg - bw2, yorg, xorg - bw2, yorg + l - 1);
							break;
						}

					case Qt::DockRight:
						{
							int bw2 = pw / 2;
							if (p->device()->isExtDev() && pw == 1)
								bw2 = 1;

							QwtPainter::drawLine(p, xorg + bw2, yorg, xorg + bw2, yorg + l - 1);
							break;
						}

					case Qt::DockTop:
						{
							const int bw2 = (pw - 1) / 2;
							QwtPainter::drawLine(p, xorg, yorg - bw2, xorg + l - 1, yorg - bw2);
							break;
						}

					case Qt::DockBottom:
						{
							const int bw2 = pw / 2;
							QwtPainter::drawLine(p, xorg, yorg + bw2, xorg + l - 1, yorg + bw2);
							break;
						}

					case Round:
						break;
				}
			}
			else
				QwtScaleDraw::drawBackbone(p);
		#endif
		};

		uint lineWidth(){return d_lineWidth;};

		void setLineWidth(uint width)
		{
		#if false
			if (d_lineWidth != width)
				d_lineWidth = width;
		#endif
		};

	private:
		uint d_lineWidth;
		QString formula_string;
};

//! Extension to QwtScaleDraw
class QwtNoLabelsScaleDraw: public ScaleDraw
{
	public:
		QwtNoLabelsScaleDraw(int linewidth){setLineWidth(linewidth);};
		~QwtNoLabelsScaleDraw(){};

		virtual QwtText label(double) const
		{
			return QwtText(QString());
		};
};

//! Extension to QwtScaleDraw
class QwtTextScaleDraw: public ScaleDraw
{
	public:
		QwtTextScaleDraw(const QStringList& list, int linewidth)
		{
		#if false
			labels = list; 
			setLineWidth(linewidth);			
		#endif
		};

		~QwtTextScaleDraw(){};

		virtual QwtText label(double value) const
		{
		#if false
			int index=qRound(value);
			if (floor(value) == value && index > 0 && index <= (int)labels.count())
				return labels[index - 1];
			else
				return "";
		#endif
		};

		QStringList labelsList(){return labels;};

	private:
		QStringList labels;
};

//! Extension to QwtScaleDraw
class TimeScaleDraw: public ScaleDraw
{
	public:
		TimeScaleDraw(const QTime& t, const QString& format, int linewidth)
		{
		#if false
			t_origin = t; 
			t_format = format;
			setLineWidth(linewidth);
		#endif
		};

		~TimeScaleDraw(){};

		QString origin() 
		{
		#if false
			return t_origin.toString ( "hh:mm:ss.zzz" );
		#endif
		};

		QString timeFormat() {return t_format;};

		virtual QwtText label(double value) const
		{
		#if false
			QTime t = t_origin.addMSecs ( (int)value );		
			return t.toString ( t_format );
		#endif
		};

	private:
		QTime t_origin;
		QString t_format;
};

//! Extension to QwtScaleDraw
class DateScaleDraw: public ScaleDraw
{
	public:
		DateScaleDraw(const QDate& t, const QString& format, int linewidth)
		{
		#if false
			t_origin = t; 
			t_format = format;
			setLineWidth(linewidth);			
		#endif
		};

		~DateScaleDraw(){};

		QString origin() 
		{
		#if false
			return t_origin.toString ( Qt::ISODate );
		#endif
		};

		QString format() {return t_format;};

		virtual QwtText label(double value) const
		{
		#if false
			QDate t = t_origin.addDays ( (int)value );
			return t.toString ( t_format );
		#endif
		};

	private:
		QDate t_origin;
		QString t_format;
};

//! Extension to QwtScaleDraw
class QwtSupersciptsScaleDraw: public ScaleDraw
{
	public:
		QwtSupersciptsScaleDraw(const QFont& fnt, const QColor& col){d_font = fnt; d_color = col;};
		~QwtSupersciptsScaleDraw(){};

		virtual QwtText label(double value) const
		{
		#if false
			char f;
			int prec, fieldwidth;
			labelFormat(f, prec, fieldwidth);

			double lval = ScaleDraw::label(value).toDouble();
			QString txt;
			txt.setNum (lval, 'e', prec);

			QStringList list = QStringList::split ( "e", txt, false );
			if (list[0].toDouble() == 0.0)
				return "0";

			QString s= list[1];
			int l = s.length();
			QChar sign = s[0];

			s.remove (sign);

			while (l>1 && s.startsWith ("0", false))
			{
				s.remove ( 0, 1 );
				l = s.length();
			}

			if (sign == '-')
				s.prepend(sign);

			if (list[0] == "1")
				return "10<sup>" + s + "</sup>";
			else
				return list[0] + "x10<sup>" + s + "</sup>";
		#endif
		};

		//! Draws the number label for a major scale tick
		void drawLabel(QPainter *p, double val) const
		{
		#if false
			QPoint pos;
			int alignment;
			double rotation;
			labelPlacement(QFontMetrics(d_font), val, pos, alignment, rotation);

			if ( alignment )
			{
				const QString txt = label(val);
				if ( !txt.isEmpty() )
				{
					QMatrix m = labelWorldMatrix(d_font, pos, alignment, rotation, txt);

					p->save();
#ifndef QT_NO_TRANSFORMATIONS
					p->setWorldMatrix(m, true);
#else
					p->translate(m.dx(), m.dy());
#endif

					QwtText *ltxt = labelText(val);
					QRect br = ltxt->boundingRect();
					QRect rect = QRect (0, 0, br.width(), br.height());

					ltxt->draw (p, rect);
					delete ltxt;

					p->restore();
				}
			}
		#endif
		};

		QwtText *labelText(double value) const
		{
		#if false
			const QString lbl = label(value);
			return QwtText::makeText(lbl, labelAlignment(), d_font, d_color);
		#endif
		};

		QRect labelBoundingRect(const QFontMetrics &fm, double val) const
		{
		#if false
			char f;
			int prec, fieldwidth;
			labelFormat(f, prec, fieldwidth);

			QString zeroString;
			if ( fieldwidth > 0 )
				zeroString.fill('0', fieldwidth);

			const QString lbl = label(val);

			const QString &txt = fm.width(zeroString) > fm.width(lbl) 
				? zeroString : lbl;
			if ( txt.isEmpty() )
				return QRect(0, 0, 0, 0);

			QwtText *ltxt = labelText(val);

			QRect br;
			QPoint pos;
			int alignment;
			double rotation;

			labelPlacement(fm, val, pos, alignment, rotation);
			if ( alignment )
			{
				const int w = ltxt->boundingRect().width();
				const int h = ltxt->boundingRect().height();

				QMatrix m = labelWorldMatrix(fm, pos, alignment, rotation, lbl);
				br = QwtMetricsMap::translate(m, QRect(0, 0, w, h));
				br.moveBy(-pos.x(), -pos.y());
			}

			delete ltxt;
			return br;
		#endif
		};

		//! Return the world matrix for painting the label 
		QMatrix labelWorldMatrix(const QFontMetrics &,
				const QPoint &pos, int alignment, 
#ifdef QT_NO_TRANSFORMATIONS
				double,
#else
				double rotation, 
#endif
				const QString &txt) const
		{
		#if false
			QwtText *ltxt = QwtText::makeText(txt, alignment, d_font, d_color);
			QRect br = ltxt->boundingRect();

			const int w = br.width();
			const int h = br.height();

			int x, y;
			if ( alignment & Qt::AlignLeft )
				x = -w;
			else if ( alignment & Qt::AlignRight )
				x = 0 - w % 2;
			else // Qt::AlignHCenter
				x = -(w / 2);

			if ( alignment & Qt::AlignTop )
				y =  - h ;
			else if ( alignment & Qt::AlignBottom )
				y = 0;
			else // Qt::AlignVCenter
				y = - (h/2);

			QMatrix m;
			m.translate(pos.x(), pos.y());
#ifndef QT_NO_TRANSFORMATIONS
			m.rotate(rotation);
#endif
			m.translate(x, y);

			delete ltxt;
			return m;
		#endif
		};

		void setFont(const QFont& fnt){d_font = fnt;};

	private:
		QFont d_font;
		QColor d_color;
};

#endif
