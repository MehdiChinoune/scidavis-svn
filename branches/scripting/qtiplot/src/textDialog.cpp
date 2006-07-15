#include "textDialog.h"
#include "colorButton.h"
#include "txt_icons.h"
#include "symbolDialog.h"
#include "application.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qbuttongroup.h>
#include <qfont.h>
#include <qfontdialog.h>
#include <qcolordialog.h>

static const char * lineSymbol_xpm[] = {
"16 16 4 1",
" 	c None",
".	c #8C2727",
"+	c #272787",
"@	c #FFFFFF",
"                ",
"                ",
"                ",
"      ...       ",
"  ............  ",
"      ...       ",
"                ",
"                ",
"                ",
"      +++       ",
"  +++++@++++++  ",
"      +++       ",
"                ",
"                ",
"                ",
"                "};

TextDialog::TextDialog(TextType type, QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
		setName( "TextDialog" );

    setCaption( tr( "QtiPlot - Text options" ) );
    setSizeGripEnabled( true );
	
	text_type = type;
	GroupBox1 = new QButtonGroup(3,QGroupBox::Horizontal, QString::null, this);

	new QLabel(tr( "Color" ), GroupBox1);
	
    colorBox = new ColorButton(GroupBox1);

	buttonOk = new QPushButton( GroupBox1);
    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );

	new QLabel(tr( "Font" ),GroupBox1);
	buttonFont = new QPushButton( GroupBox1, "buttonFont" );
    buttonFont->setText( tr( "&Font" ) );
    buttonFont->setAutoDefault( TRUE );

	buttonApply = new QPushButton( GroupBox1, "buttonApply" );
    buttonApply->setText( tr( "&Apply" ) );
    buttonApply->setAutoDefault( TRUE );
    buttonApply->setDefault( TRUE );
	
	if (text_type)
		{
		new QLabel(tr( "Alignement" ),GroupBox1, "TextLabel1_22",0);
		alignementBox = new QComboBox( FALSE, GroupBox1, "alignementBox" );
		alignementBox->insertItem( tr( "Center" ) );
		alignementBox->insertItem( tr( "Left" ) );
		alignementBox->insertItem( tr( "Right" ) );
		}
	else
		{
		new QLabel(tr( "Frame" ), GroupBox1, "TextLabel1",0 );
		backgroundBox = new QComboBox( FALSE, GroupBox1, "backgroundBox" );
		backgroundBox->insertItem( tr( "None" ) );
		backgroundBox->insertItem( tr( "Rectangle" ) );
		backgroundBox->insertItem( tr( "Shadow" ) );
		}

	buttonCancel = new QPushButton( GroupBox1, "buttonCancel" );
    buttonCancel->setText( tr( "&Cancel" ) );
    buttonCancel->setAutoDefault( TRUE );	

	if (text_type == TextMarker)
		{
		new QLabel(tr("Background"), GroupBox1, "TextLabel2",0 );
		backgroundBtn = new ColorButton(GroupBox1);

		connect(backgroundBtn, SIGNAL(clicked()), this, SLOT(pickBackgroundColor()));

		buttonDefault = new QPushButton( GroupBox1);
		buttonDefault->setText( tr( "Set &Default" ) );
		connect(buttonDefault, SIGNAL(clicked()), this, SLOT(setDefaultValues()));
		}

	QLabel* rotate=new QLabel(tr( "Rotate (deg.)" ),GroupBox1, "TextLabel1_2",0);
	rotate->hide();
	
    rotateBox = new QComboBox( FALSE, GroupBox1, "rotateBox" );
    rotateBox->insertItem( tr( "0" ) );
    rotateBox->insertItem( tr( "45" ) );
    rotateBox->insertItem( tr( "90" ) );
    rotateBox->insertItem( tr( "135" ) );
    rotateBox->insertItem( tr( "180" ) );
    rotateBox->insertItem( tr( "225" ) );
    rotateBox->insertItem( tr( "270" ) );
    rotateBox->insertItem( tr( "315" ) );
	rotateBox->setEditable (TRUE);
	rotateBox->setCurrentItem(0);
	rotateBox->hide();
	
	GroupBox2= new QButtonGroup(8, QGroupBox::Horizontal, QString::null,this, "GroupBox2" );

	if (text_type == TextMarker)
		{
		buttonCurve = new QPushButton( GroupBox2, "buttonCurve" ); 
		buttonCurve->setPixmap (QPixmap(lineSymbol_xpm));
		connect( buttonCurve, SIGNAL( clicked() ), this, SLOT(addCurve() ) );
		}

    buttonIndice = new QPushButton( GroupBox2, "buttonIndice" ); 
    buttonIndice->setPixmap (QPixmap(index_xpm));

    buttonExp = new QPushButton( GroupBox2, "buttonExp" );
    buttonExp->setPixmap (QPixmap(exp_xpm));

    buttonMinGreek = new QPushButton(QChar(0x3B1), GroupBox2, "buttonMinGreek" ); 
	buttonMinGreek->setMaximumWidth(40);

	buttonMajGreek = new QPushButton(QChar(0x393), GroupBox2, "buttonMajGreek" ); 
	buttonMajGreek->setMaximumWidth(40);

	QFont font = this->font();
	font.setBold(true);

    buttonB = new QPushButton(tr("B"), GroupBox2, "buttonB" ); 
    buttonB->setFont(font);
	buttonB->setMaximumWidth(40);

	font = this->font();
	font.setItalic(true);
    buttonI = new QPushButton(tr("It"), GroupBox2, "buttonI" );
	buttonI->setFont(font);
	buttonI->setMaximumWidth(40);

	font = this->font();
	font.setUnderline(true);

    buttonU = new QPushButton(tr("U"), GroupBox2, "buttonU" );
	buttonU->setFont(font);
	buttonU->setMaximumWidth(40);

    LineEdit = new QMultiLineEdit( this, "LineEdit" );

	setFocusPolicy(QWidget::StrongFocus);
	setFocusProxy(LineEdit);
	
	QVBoxLayout* vlayout = new QVBoxLayout(this,5,5, "vlayout");
    vlayout->addWidget(GroupBox1);
	vlayout->addWidget(GroupBox2);
	vlayout->addWidget(LineEdit);

    // signals and slots connections
	connect( colorBox, SIGNAL( clicked() ), this, SLOT( pickTextColor() ) );
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonApply, SIGNAL( clicked() ), this, SLOT( apply() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( buttonFont, SIGNAL( clicked() ), this, SLOT(customFont() ) );
	connect( buttonExp, SIGNAL( clicked() ), this, SLOT(addExp() ) );
	connect( buttonIndice, SIGNAL( clicked() ), this, SLOT(addIndex() ) );
	connect( buttonU, SIGNAL( clicked() ), this, SLOT(addUnderline() ) );
	connect( buttonI, SIGNAL( clicked() ), this, SLOT(addItalic() ) );
	connect( buttonB, SIGNAL( clicked() ), this, SLOT(addBold() ) );
	connect(buttonMinGreek, SIGNAL(clicked()), this, SLOT(showMinGreek()));
	connect(buttonMajGreek, SIGNAL(clicked()), this, SLOT(showMajGreek()));
}

void TextDialog::showMinGreek()
{
symbolDialog *greekLetters = new symbolDialog(symbolDialog::minGreek, this,"greekLetters",
											  false, WStyle_Tool|WDestructiveClose);
connect(greekLetters, SIGNAL(addLetter(const QString&)), this, SLOT(addSymbol(const QString&)));

QFont fnt = f;
fnt.setPointSize(14);
greekLetters->setFont(fnt);
greekLetters->show();
greekLetters->setActiveWindow();
}

void TextDialog::showMajGreek()
{
symbolDialog *greekLetters = new symbolDialog(symbolDialog::majGreek, this, "greekLetters",
											  false, WStyle_Tool|WDestructiveClose);
connect(greekLetters, SIGNAL(addLetter(const QString&)), this, SLOT(addSymbol(const QString&)));

QFont fnt = f;
fnt.setPointSize(14);
greekLetters->setFont(fnt);
greekLetters->show();
greekLetters->setActiveWindow();
}

void TextDialog::addSymbol(const QString& letter)
{
LineEdit->insert(letter);
}

void TextDialog::addCurve()
{
int line=0, col=0;
LineEdit->getCursorPosition (&line,&col);
LineEdit->insert("\\c{}");
LineEdit->setCursorPosition (line,col+3,FALSE);
}

void TextDialog::addUnderline()
{
int line=0, col=0;
if (LineEdit->hasMarkedText())
	{	
	QString markedText=LineEdit->markedText ();
	LineEdit->insert("<u>"+markedText);
	LineEdit->getCursorPosition (&line,&col);
	LineEdit->insertAt ("</u>",line,col,FALSE);
	LineEdit->setCursorPosition (line,col,FALSE);
	}
else
	{	
	LineEdit->getCursorPosition (&line,&col);
	LineEdit->insert("<u></u>");
	LineEdit->setCursorPosition (line,col+3,FALSE);
	}
}

void TextDialog::addItalic()
{
int line=0, col=0;

if (LineEdit->hasMarkedText())
	{	
	QString markedText=LineEdit->markedText ();
	LineEdit->insert("<i>"+markedText);
	LineEdit->getCursorPosition (&line,&col);
	LineEdit->insertAt ("</i>",line,col,FALSE);
	LineEdit->setCursorPosition (line,col,FALSE);
	}
else
	{	
	LineEdit->getCursorPosition (&line,&col);
	LineEdit->insert("<i></i>");
	LineEdit->setCursorPosition (line,col+3,FALSE);
	}
}

void TextDialog::addBold()
{
int line=0, col=0;
	
if (LineEdit->hasMarkedText())
	{	
	QString markedText=LineEdit->markedText ();
	LineEdit->insert("<b>"+markedText);
	LineEdit->getCursorPosition (&line,&col);
	LineEdit->insertAt ("</b>",line,col,FALSE);
	LineEdit->setCursorPosition (line,col,FALSE);
	}
else
	{	
	LineEdit->getCursorPosition (&line,&col);
	LineEdit->insert("<b></b>");
	LineEdit->setCursorPosition (line,col+3,FALSE);
	}
}

void TextDialog::addIndex()
{
int line=0, col=0;
LineEdit->getCursorPosition (&line,&col);
	
if (LineEdit->hasMarkedText())
	{	
	QString markedText=LineEdit->markedText ();
	LineEdit->insert("<sub>"+markedText);
	LineEdit->getCursorPosition (&line,&col);
	LineEdit->insertAt ("</sub>",line,col,FALSE);
	LineEdit->setCursorPosition (line,col,FALSE);
	}
else
	{	
	LineEdit->insert("<sub></sub>");
	LineEdit->setCursorPosition (line,col+5,FALSE);
	}
}

void TextDialog::addExp()
{
int line, col;
LineEdit->getCursorPosition (&line,&col);

if (LineEdit->hasMarkedText())
	{	
	QString markedText=LineEdit->markedText ();
	LineEdit->insert("<sup>"+markedText);
	LineEdit->getCursorPosition (&line,&col);
	LineEdit->insertAt ("</sup>",line,col,FALSE);
	LineEdit->setCursorPosition (line,col,FALSE);
	}
else
	{	
	LineEdit->insert("<sup></sup>");
	LineEdit->setCursorPosition (line,col+5,FALSE);
	}
}

void TextDialog::apply()
{
if (text_type)
	{
	emit changeAlignment(alignment());
	emit changeText(LineEdit->text());
	emit changeColor(colorBox->color());
	}
else
	emit values(LineEdit->text(),0, backgroundBox->currentItem(), 
				f, colorBox->color(), backgroundBtn->color());
}

void TextDialog::setDefaultValues()
{
ApplicationWindow *app = (ApplicationWindow *)this->parent();
if (!app)
	return;

app->setLegendDefaultSettings(backgroundBox->currentItem(), f, 
							  colorBox->color(), backgroundBtn->color());
}

void TextDialog::accept()
{
apply();
close();
}

void TextDialog::setBackgroundType(int bkg)
{
backgroundBox->setCurrentItem(bkg);
}

int TextDialog::alignment()
{
int align=-1;
switch (alignementBox->currentItem())
	{
	case 0:
		align=Qt::AlignHCenter;
	break;

	case 1:
		align=Qt::AlignLeft;
	break;
	
	case 2:
		align=Qt::AlignRight;
	break;
	}
return align;
}

void TextDialog::setAlignment(int align)
{	
if (align==Qt::AlignHCenter)
	alignementBox->setCurrentItem(0);
else if (align==Qt::AlignLeft)
	alignementBox->setCurrentItem(1);
else if (align==Qt::AlignRight)
	alignementBox->setCurrentItem(2);
else
	return;
}

void TextDialog::customFont()
{
bool okF;
QFont fnt = QFontDialog::getFont( &okF, f, this);
if (okF && fnt != f)
	{
	f = fnt;
	
	fnt.setPointSize(12);
	LineEdit->setFont(fnt);

	emit changeFont (f);
	}
}

void TextDialog::setAngle(int angle)
{
rotateBox->setEditText(QString::number(angle));
}

void TextDialog::setText(const QString & t)
{
LineEdit->insertAt (t, 0, 0, TRUE );
}

void TextDialog::setTextColor(QColor c)
{
  colorBox->setColor(c);
}

void TextDialog::pickTextColor()
{
QColor c = QColorDialog::getColor( colorBox->color(), this);
if ( !c.isValid() || c ==  colorBox->color() )
	return;

colorBox->setColor ( c ) ;
}

void TextDialog::setBackgroundColor(QColor c)
{
  backgroundBtn->setColor(c);
}

void TextDialog::pickBackgroundColor()
{
QColor c = QColorDialog::getColor( backgroundBtn->color(), this);
if ( !c.isValid() || c ==  backgroundBtn->color() )
	return;

backgroundBtn->setColor ( c ) ;
}

void TextDialog::setFont(const QFont& fnt)
{
f = fnt; 

QFont auxf = f;
auxf.setPointSize(12);
LineEdit->setFont(auxf);
}

TextDialog::~TextDialog()
{
}
