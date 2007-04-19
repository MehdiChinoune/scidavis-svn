/***************************************************************************
    File                 : colorButton.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : A button used for color selection
                           
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
#include "colorButton.h"

#include <QPalette>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFrame>

/* XPM */
static const char * palette_xpm[] = {
"16 16 257 2",
"  	c None",
". 	c #00E5FE",
"+ 	c #00E4FE",
"@ 	c #00E3FE",
"# 	c #00E1FE",
"$ 	c #00DFFE",
"% 	c #00DDFE",
"& 	c #00E5FE",
"* 	c #00EFFD",
"= 	c #00F7FD",
"- 	c #00F9FD",
"; 	c #00F9FB",
"> 	c #00F7F9",
", 	c #00F2F0",
"' 	c #00DFD8",
") 	c #00B5B0",
"! 	c #005A53",
"~ 	c #002C26",
"{ 	c #000F0D",
"] 	c #000000",
"^ 	c #000000",
"/ 	c #000000",
"( 	c #000000",
"_ 	c #000000",
": 	c #000000",
"< 	c #000000",
"[ 	c #000000",
"} 	c #000000",
"| 	c #000000",
"1 	c #000000",
"2 	c #000000",
"3 	c #000000",
"4 	c #000000",
"5 	c #000000",
"6 	c #000000",
"7 	c #000000",
"8 	c #000000",
"9 	c #000000",
"0 	c #000000",
"a 	c #000000",
"b 	c #000000",
"c 	c #000000",
"d 	c #000000",
"e 	c #000000",
"f 	c #000000",
"g 	c #000000",
"h 	c #000000",
"i 	c #000000",
"j 	c #000000",
"k 	c #000000",
"l 	c #000000",
"m 	c #000000",
"n 	c #000000",
"o 	c #000000",
"p 	c #000000",
"q 	c #000000",
"r 	c #000000",
"s 	c #000000",
"t 	c #000000",
"u 	c #000000",
"v 	c #000000",
"w 	c #000000",
"x 	c #000000",
"y 	c #000000",
"z 	c #000000",
"A 	c #000000",
"B 	c #000000",
"C 	c #000000",
"D 	c #000000",
"E 	c #000000",
"F 	c #000000",
"G 	c #000000",
"H 	c #000000",
"I 	c #000000",
"J 	c #000000",
"K 	c #000000",
"L 	c #000000",
"M 	c #000000",
"N 	c #000000",
"O 	c #000000",
"P 	c #000B10",
"Q 	c #01202E",
"R 	c #01425E",
"S 	c #026C9A",
"T 	c #0288C3",
"U 	c #0299DB",
"V 	c #02A5EA",
"W 	c #02ACF4",
"X 	c #02AFF9",
"Y 	c #01B1FC",
"Z 	c #00B0FD",
"` 	c #00AFFE",
" .	c #00AEFE",
"..	c #00ABFE",
"+.	c #00A7FE",
"@.	c #00A5FE",
"#.	c #009FFE",
"$.	c #0099FE",
"%.	c #0094FE",
"&.	c #008EFE",
"*.	c #0087FE",
"=.	c #0080FE",
"-.	c #007AFE",
";.	c #0073FE",
">.	c #006FFE",
",.	c #006BFE",
"'.	c #0066FE",
").	c #005EFE",
"!.	c #0056FE",
"~.	c #004CFE",
"{.	c #003FFE",
"].	c #0037FE",
"^.	c #002EFE",
"/.	c #0023FE",
"(.	c #0019FE",
"_.	c #0012FE",
":.	c #000BFE",
"<.	c #0004FE",
"[.	c #0102FE",
"}.	c #0101FE",
"|.	c #0200FE",
"1.	c #0500FE",
"2.	c #0C01FE",
"3.	c #1A01FE",
"4.	c #2501FE",
"5.	c #2C01FE",
"6.	c #3301FE",
"7.	c #3D00FE",
"8.	c #4300FE",
"9.	c #4C00FE",
"0.	c #5600FE",
"a.	c #6100FE",
"b.	c #6E00FE",
"c.	c #7600FE",
"d.	c #7C00FE",
"e.	c #7D00FE",
"f.	c #7F00FE",
"g.	c #8300FE",
"h.	c #8C00FE",
"i.	c #9800FE",
"j.	c #A300FE",
"k.	c #AC00FE",
"l.	c #B600FE",
"m.	c #C200FE",
"n.	c #D602FE",
"o.	c #E703FE",
"p.	c #F503FE",
"q.	c #FA03FE",
"r.	c #FB07FE",
"s.	c #FC11FE",
"t.	c #FD1EFE",
"u.	c #FD2BFE",
"v.	c #FE35FE",
"w.	c #FE40FE",
"x.	c #FE48FE",
"y.	c #FE50FE",
"z.	c #FE59FE",
"A.	c #FE62FE",
"B.	c #FE6BFE",
"C.	c #FE73FE",
"D.	c #FE77FE",
"E.	c #FE7BFE",
"F.	c #FE85FE",
"G.	c #FE90FE",
"H.	c #FEA0FE",
"I.	c #FEB2FE",
"J.	c #FEC1FD",
"K.	c #FECDFC",
"L.	c #FEDFFA",
"M.	c #FEEEF7",
"N.	c #FEF1F4",
"O.	c #FEE7EA",
"P.	c #FEDEE1",
"Q.	c #FECED0",
"R.	c #FEBEC0",
"S.	c #FEB1B3",
"T.	c #FEA7A9",
"U.	c #FE9899",
"V.	c #FE8B8B",
"W.	c #FE8485",
"X.	c #FE7F7F",
"Y.	c #FE7C7C",
"Z.	c #FE7373",
"`.	c #FE6867",
" +	c #FE5D5B",
".+	c #FE524F",
"++	c #FE453D",
"@+	c #FE382C",
"#+	c #FE2C20",
"$+	c #FE2012",
"%+	c #FE1608",
"&+	c #FE1204",
"*+	c #FE1803",
"=+	c #FE2400",
"-+	c #FE3400",
";+	c #FE4600",
">+	c #FE5000",
",+	c #FF5900",
"'+	c #FE6400",
")+	c #FE6C01",
"!+	c #FE7702",
"~+	c #FE8603",
"{+	c #FE8D05",
"]+	c #FE9708",
"^+	c #FE9B0B",
"/+	c #FEA10F",
"(+	c #FEA418",
"_+	c #FEA71E",
":+	c #FEA923",
"<+	c #FEAA24",
"[+	c #FEAA24",
"}+	c #FEAB22",
"|+	c #FEAE1F",
"1+	c #FEB31C",
"2+	c #FCB619",
"3+	c #FEBD13",
"4+	c #FEC311",
"5+	c #FEC70C",
"6+	c #FECC0A",
"7+	c #FECF0B",
"8+	c #FED50D",
"9+	c #FED80C",
"0+	c #FEDD0A",
"a+	c #FEE208",
"b+	c #FEE807",
"c+	c #FEED05",
"d+	c #FEF103",
"e+	c #FEF702",
"f+	c #FEF902",
"g+	c #FEFB01",
"h+	c #FDFC01",
"i+	c #FBF902",
"j+	c #EFEF06",
"k+	c #D3DB0D",
"l+	c #BFCE13",
"m+	c #ADCC14",
"n+	c #9BCD14",
"o+	c #88D212",
"p+	c #74D80F",
"q+	c #67DB0E",
"r+	c #5CDF0C",
"s+	c #56E10B",
"t+	c #4DE30A",
"u+	c #3AEA08",
"v+	c #2CEF06",
"w+	c #20F304",
"x+	c #11F803",
"y+	c #08FB04",
"z+	c #FFFFFF",
"A+	c #03FD19",
"B+	c #02FE2E",
"C+	c #01FE43",
"D+	c #00FE55",
"E+	c #00FE6C",
"F+	c #00FE8A",
"G+	c #00FE95",
"H+	c #00FE9D",
"            N.R.X.&+            ",
"      N.N.N.N.R.X.-+,+~+        ",
"    q.H.K.L.N.R.X.>+~+;+{+      ",
"  q.q.q.F.H.K.R.X.&+,+{+5+e+    ",
"  q.q.q.q.E.J.R.X.&+)+/+b+0+8+  ",
"  q.q.q.q.w.B.B.X.'+{+a+8+4+1+  ",
"m.m.m.m.m.q.t.B.X.7+b+8+4+1+|+  ",
"f.f.f.f.f.f.f.f.a+d+9+4+1+|+[+  ",
"|.|./.].{.).-.+.[+[+[+[+[+[+[+  ",
"|.(.{.).,.&.Z m+l+[+[+[+[+[+[+  ",
"  ^.!.=.%.Z & q+o+m+[+[+[+[+[+  ",
"  ).=.@.Z % - w+t+o+m+[+[+[+    ",
"    ..% % @ = y+v+s+o+m+[+[+    ",
"      - - = - y+x+v+t+p+n+      ",
"        - - - y+y+x+w+          ",
"                                "};


ColorButton::ColorButton(QWidget *parent) : QWidget(parent)
{
	init();
}

void ColorButton::init()
{
	int btn_size = 28;
	selectButton = new QPushButton(QPixmap(palette_xpm), QString(), this);
	selectButton->setMinimumWidth(btn_size);
	selectButton->setMinimumHeight(btn_size);

	display = new QFrame(this);
	display->setLineWidth(2);
	display->setFrameStyle (QFrame::Panel | QFrame::Sunken);
	display->setMinimumHeight(btn_size);
	display->setMinimumWidth(3*btn_size);
	display->setAutoFillBackground(true);
	setColor(QColor(Qt::white));
	
	QHBoxLayout *l = new QHBoxLayout(this);
	l->setMargin( 0 );
	l->addWidget( display );
	l->addWidget( selectButton );

	setMaximumWidth(4*btn_size);
	setMaximumHeight(btn_size);

	connect(selectButton, SIGNAL(clicked()), this, SIGNAL(clicked()));
}

void ColorButton::setColor(const QColor& c)
{
	QPalette pal;
	pal.setColor(QPalette::Window, c);
	display->setPalette(pal);
}

QColor ColorButton::color() const
{
	return display->palette().color(QPalette::Window);
}

QSize ColorButton::sizeHint () const
{
	return QSize(4*btn_size, btn_size);
}