/***************************************************************************
    File                 : AxesDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : General plot options dialog

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
#include "AxesDialog.h"
#include "lib/TextDialog.h"
#include "lib/ColorBox.h"
#include "Layer.h"
#include "Plot.h"
#include "core/MyParser.h"
#include "lib/ColorButton.h"
#include "lib/TextFormatButtons.h"

#include <QColorDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QLayout>
#include <QMessageBox>
#include <QFontDialog>
#include <QDate>
#include <QList>
#include <QListWidget>
#include <QVector>
#include <QGroupBox>
#include <QRadioButton>

#include <qwt_plot.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>

#ifndef M_PI
#define M_PI	3.141592653589793238462643
#endif

AxesDialog::AxesDialog( QWidget* parent, Qt::WFlags fl )
: QDialog( parent, fl )
{
	QPixmap image4( ":/image4.xpm" );
		QPixmap image5( ":/image5.xpm" );
		QPixmap image6( ":/image6.xpm" );
		QPixmap image7( ":/image7.xpm" );
		setWindowTitle( tr( "General Plot Options" ) );

		generalDialog = new QTabWidget();

		initScalesPage();
		initGridPage();
		initAxesPage();
		initFramePage();

		QHBoxLayout * bottomButtons = new QHBoxLayout();
		bottomButtons->addStretch();

		buttonApply = new QPushButton();
		buttonApply->setText( tr( "&Apply" ) );
		bottomButtons->addWidget( buttonApply );

		buttonOk = new QPushButton();
		buttonOk->setText( tr( "&OK" ) );
		buttonOk->setDefault( true );
		bottomButtons->addWidget( buttonOk );

		buttonCancel = new QPushButton();
		buttonCancel->setText( tr( "&Cancel" ) );
		bottomButtons->addWidget( buttonCancel );

		QVBoxLayout * mainLayout = new QVBoxLayout(this);
		mainLayout->addWidget(generalDialog);
		mainLayout->addLayout(bottomButtons);

		resize(minimumSize());

		for (int i=0;i<4;i++)
			titles<<"";

		lastPage = scalesPage;

		connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
		connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
		connect( buttonApply, SIGNAL( clicked() ), this, SLOT(updatePlot() ) );
		connect( generalDialog, SIGNAL( currentChanged ( QWidget * ) ),
  	             this, SLOT(pageChanged ( QWidget * ) ) );
}

void AxesDialog::initScalesPage()
{
	scalesPage = new QWidget();

		QGroupBox * middleBox = new QGroupBox(QString());
		QGridLayout * middleLayout = new QGridLayout(middleBox);

		middleLayout->addWidget(new QLabel(tr( "From" )), 0, 0);
		boxStart = new QLineEdit();
		middleLayout->addWidget( boxStart, 0, 1 );

		middleLayout->addWidget(new QLabel(tr( "To" )), 1, 0);
		boxEnd = new QLineEdit();
		middleLayout->addWidget( boxEnd, 1, 1);

		boxScaleTypeLabel = new QLabel(tr( "Type" ));
		boxScaleType = new QComboBox();
		boxScaleType->addItem(tr( "linear" ) );
		boxScaleType->addItem(tr( "logarithmic" ) );
		middleLayout->addWidget( boxScaleTypeLabel, 2, 0);
		middleLayout->addWidget( boxScaleType, 2, 1);

		btnInvert = new QCheckBox();
		btnInvert->setText( tr( "Inverted" ) );
		btnInvert->setChecked(false);
		middleLayout->addWidget( btnInvert, 3, 1 );

		middleLayout->setRowStretch( 4, 1 );

		QGroupBox * rightBox = new QGroupBox(QString());
		QGridLayout * rightLayout = new QGridLayout(rightBox);
		QWidget * stepWidget = new QWidget();
		QVBoxLayout * stepWidgetLayout = new QVBoxLayout( stepWidget );

		btnStep = new QRadioButton(rightBox);
		btnStep->setText( tr( "Step" ) );
		btnStep->setChecked(true);
		rightLayout->addWidget( btnStep, 0, 0 );

		boxStep = new QLineEdit();
		stepWidgetLayout->addWidget( boxStep );
		boxUnit = new QComboBox();
		boxUnit->hide();
		stepWidgetLayout->addWidget( boxUnit );

		rightLayout->addWidget( stepWidget, 0, 1 );

		btnMajor = new QRadioButton(rightBox);
		btnMajor->setText( tr( "Major Ticks" ) );
		rightLayout->addWidget( btnMajor, 1, 0);

		boxMajorValue = new QSpinBox();
		boxMajorValue->setDisabled(true);
		rightLayout->addWidget( boxMajorValue, 1, 1);

		minorBoxLabel = new QLabel( tr( "Minor Ticks" ));
		rightLayout->addWidget( minorBoxLabel, 2, 0);

		boxMinorValue = new QComboBox();
		boxMinorValue->setEditable(true);
		boxMinorValue->addItems(QStringList()<<"0"<<"1"<<"4"<<"9"<<"14"<<"19");
		rightLayout->addWidget( boxMinorValue, 2, 1);

		rightLayout->setRowStretch( 3, 1 );

		QPixmap image0(":/bottom_scl.xpm" );
		QPixmap image1(":/left_scl.xpm" );
		QPixmap image2(":/top_scl.xpm" );
  	    QPixmap image3(":/right_scl.xpm" );

		axesList = new QListWidget();
		axesList->addItem( new QListWidgetItem(image0, tr( "Bottom" )));
		axesList->addItem( new QListWidgetItem(image1, tr( "Left" )));
		axesList->addItem( new QListWidgetItem(image2, tr( "Top" )));
  	    axesList->addItem( new QListWidgetItem(image3,  tr( "Right" )));
		axesList->setIconSize(image0.size());
		axesList->setCurrentRow(0);

		// calculate a sensible width for the items list
		// (default QListWidget size is 256 which looks too big)
		QFontMetrics fm(axesList->font());
		int width = 32,i;
		for(i=0 ; i<axesList->count() ; i++)
			if( fm.width(axesList->item(i)->text()) > width)
				width = fm.width(axesList->item(i)->text());

		axesList->setMaximumWidth( axesList->iconSize().width() + width + 50 );
		// resize the list to the maximum width
		axesList->resize(axesList->maximumWidth(),axesList->height());

		QHBoxLayout* mainLayout = new QHBoxLayout(scalesPage);
		mainLayout->addWidget(axesList);
		mainLayout->addWidget(middleBox);
		mainLayout->addWidget(rightBox);

		generalDialog->addTab(scalesPage, tr( "Scale" ));

		connect(btnInvert,SIGNAL(clicked()), this, SLOT(updatePlot()));
		connect(axesList,SIGNAL(currentRowChanged(int)), this, SLOT(updateScale()));
		connect(boxScaleType,SIGNAL(activated(int)), this, SLOT(updateMinorTicksList(int)));

		connect(btnStep, SIGNAL(toggled(bool)), boxStep, SLOT(setEnabled(bool)));
		connect(btnStep, SIGNAL(toggled(bool)), boxUnit, SLOT(setEnabled(bool)));
		connect(btnMajor, SIGNAL(toggled(bool)), boxMajorValue, SLOT(setEnabled(bool)));
}

void AxesDialog::initGridPage()
{
	gridPage = new QWidget();

		QGroupBox * rightBox = new QGroupBox(QString());
		QGridLayout * rightLayout = new QGridLayout(rightBox);

		boxMajorGrid = new QCheckBox();
		boxMajorGrid->setText( tr( "Major Grids" ) );
		boxMajorGrid->setChecked(true);
		rightLayout->addWidget( boxMajorGrid, 0, 1);

		boxMinorGrid = new QCheckBox();
		boxMinorGrid->setText( tr( "Minor Grids" ) );
		boxMinorGrid->setChecked(false);
		rightLayout->addWidget( boxMinorGrid, 0, 2);

		rightLayout->addWidget( new QLabel(tr( "Line Color" )), 1, 0 );

		boxColorMajor = new ColorBox(0);
		rightLayout->addWidget( boxColorMajor, 1, 1);

		boxColorMinor = new ColorBox(0);
		boxColorMinor->setDisabled(true);
		rightLayout->addWidget( boxColorMinor, 1, 2);

		rightLayout->addWidget( new QLabel(tr( "Line Type" )), 2, 0 );

		boxTypeMajor = new QComboBox();
		boxTypeMajor->addItem("_____");
		boxTypeMajor->addItem("- - -");
		boxTypeMajor->addItem(".....");
		boxTypeMajor->addItem("_._._");
		boxTypeMajor->addItem("_.._..");
		rightLayout->addWidget( boxTypeMajor, 2, 1);

		boxTypeMinor = new QComboBox();
		boxTypeMinor->addItem("_____");
		boxTypeMinor->addItem("- - -");
		boxTypeMinor->addItem(".....");
		boxTypeMinor->addItem("_._._");
		boxTypeMinor->addItem("_.._..");
		boxTypeMinor->setDisabled(true);
		rightLayout->addWidget( boxTypeMinor, 2, 2);

		rightLayout->addWidget( new QLabel(tr( "Thickness" )), 3, 0 );

		boxWidthMajor = new QSpinBox();
		boxWidthMajor->setRange(1,20);
		boxWidthMajor->setValue(1);
		rightLayout->addWidget( boxWidthMajor, 3, 1);

		boxWidthMinor = new QSpinBox();
		boxWidthMinor->setRange(1,20);
		boxWidthMinor->setValue(1);
		boxWidthMinor->setDisabled(true);
		rightLayout->addWidget( boxWidthMinor, 3, 2);

		rightLayout->addWidget( new QLabel(tr( "Axes" )), 4, 0 );

      	boxGridXAxis = new QComboBox();
        boxGridXAxis->insertItem(tr("Bottom"));
        boxGridXAxis->insertItem(tr("Top"));
        rightLayout->addWidget( boxGridXAxis, 4, 1);

        boxGridYAxis = new QComboBox();
        boxGridYAxis->insertItem(tr("Left"));
        boxGridYAxis->insertItem(tr("Right"));
        rightLayout->addWidget( boxGridYAxis, 4, 2);

		rightLayout->addWidget( new QLabel(tr( "Additional lines" )), 5, 0);

		boxXLine = new QCheckBox();
		boxXLine->setText( tr( "X=0" ) );
		boxXLine->setDisabled(true);
		rightLayout->addWidget( boxXLine, 5, 1);

		boxYLine = new QCheckBox();
		boxYLine->setText( tr( "Y=0" ) );
		rightLayout->addWidget( boxYLine, 5, 2);

		rightLayout->setRowStretch( 6, 1 );
		rightLayout->setColumnStretch( 4, 1 );

		QPixmap image2(":/image2.xpm" );
		QPixmap image3(":/image3.xpm" );

		axesGridList = new QListWidget();
		axesGridList->addItem( new QListWidgetItem(image3, tr( "Horizontal" )) );
		axesGridList->addItem( new QListWidgetItem(image2, tr( "Vertical" )) );
		axesGridList->setIconSize(image3.size());
		axesGridList->setCurrentRow(-1);

		// calculate a sensible width for the items list
		// (default QListWidget size is 256 which looks too big)
		QFontMetrics fm(axesGridList->font());
		int width = 32,i;
		for(i=0 ; i<axesGridList->count() ; i++)
			if( fm.width(axesGridList->item(i)->text()) > width)
		width = fm.width(axesGridList->item(i)->text());
		axesGridList->setMaximumWidth( axesGridList->iconSize().width() + width + 50 );
		// resize the list to the maximum width
		axesGridList->resize(axesGridList->maximumWidth(),axesGridList->height());

		QHBoxLayout* mainLayout2 = new QHBoxLayout(gridPage);
		mainLayout2->addWidget(axesGridList);
		mainLayout2->addWidget(rightBox);

		generalDialog->addTab( gridPage, tr( "Grid" ) );

	//grid page slot connections
	connect(axesGridList,SIGNAL(itemSelectionChanged()),this, SLOT(setGridOptions()));
	connect(boxMajorGrid,SIGNAL(toggled(bool)), this, SLOT(majorGridEnabled(bool)));
	connect(boxMinorGrid,SIGNAL(toggled(bool)), this, SLOT(minorGridEnabled(bool)));
	connect(boxColorMajor,SIGNAL(activated(int)),this, SLOT(updateGrid(int)));
	connect(boxColorMinor,SIGNAL(activated(int)),this, SLOT(updateGrid(int)));
	connect(boxTypeMajor,SIGNAL(activated(int)),this, SLOT(updateGrid(int)));
	connect(boxTypeMinor,SIGNAL(activated(int)),this, SLOT(updateGrid(int)));
	connect(boxWidthMajor,SIGNAL(valueChanged (int)),this, SLOT(updateGrid(int)));
	connect(boxWidthMinor,SIGNAL(valueChanged (int)),this, SLOT(updateGrid(int)));
	connect(boxXLine,SIGNAL(clicked()),this, SLOT(updatePlot()));
	connect(boxYLine,SIGNAL(clicked()),this, SLOT(updatePlot()));
}

void AxesDialog::initAxesPage()
{
	//axes page
	QPixmap image4(":/image4.xpm" );
	QPixmap image5(":/image5.xpm" );
	QPixmap image6(":/image6.xpm" );
	QPixmap image7(":/image7.xpm" );

	axesPage = new QWidget();

	axesTitlesList = new QListWidget();
	axesTitlesList->addItem( new QListWidgetItem(image4, tr("Bottom")));
	axesTitlesList->addItem( new QListWidgetItem(image5, tr("Left")));
	axesTitlesList->addItem( new QListWidgetItem(image6, tr("Top")));
	axesTitlesList->addItem( new QListWidgetItem(image7, tr("Right")));
	axesTitlesList->setIconSize(image6.size());
	axesTitlesList->setMaximumWidth((int)(image6.width()*1.5));
	axesTitlesList->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	axesTitlesList->setCurrentRow(-1);

	// calculate a sensible width for the items list
	// (default QListWidget size is 256 which looks too big)
	QFontMetrics fm(axesTitlesList->font());
	int width = 32,i;
	for(i=0 ; i<axesTitlesList->count() ; i++)
		if( fm.width(axesTitlesList->item(i)->text()) > width)
			width = fm.width(axesTitlesList->item(i)->text());
	axesTitlesList->setMaximumWidth( axesTitlesList->iconSize().width() + width + 50 );
	// resize the list to the maximum width
	axesTitlesList->resize(axesTitlesList->maximumWidth(),axesTitlesList->height());

	QHBoxLayout * topLayout = new QHBoxLayout();

	boxShowAxis = new QCheckBox(tr("Show"));
	boxShowAxis->setChecked(true);
	topLayout->addWidget( boxShowAxis );

	labelBox = new QGroupBox(tr( "Title"));
	topLayout->addWidget( labelBox );

	QVBoxLayout *labelBoxLayout = new QVBoxLayout( labelBox );
    labelBoxLayout->setSpacing(2);

	boxTitle = new QTextEdit();
	boxTitle->setTextFormat(Qt::PlainText);
	QFontMetrics metrics(this->font());
	boxTitle->setMaximumHeight(3*metrics.height());
    labelBoxLayout->addWidget(boxTitle);

	QHBoxLayout *hl = new QHBoxLayout();
	hl->setMargin(0);
	hl->setSpacing(2);
	buttonLabelFont = new QPushButton(tr("&Font"));
	hl->addWidget(buttonLabelFont);

	formatButtons = new TextFormatButtons(boxTitle);
	formatButtons->toggleCurveButton(false);
	hl->addWidget(formatButtons);
	hl->addStretch();

	boxTitle->setMaximumWidth(buttonLabelFont->width() + formatButtons->width());
	labelBoxLayout->addLayout(hl);

	QHBoxLayout * bottomLayout = new QHBoxLayout();

	QGroupBox *leftBox = new QGroupBox(QString());
	bottomLayout->addWidget( leftBox );
	QGridLayout * leftBoxLayout = new QGridLayout( leftBox );

	leftBoxLayout->addWidget( new QLabel(tr( "Type" )), 0, 0 );

	boxAxisType= new QComboBox();
	boxAxisType->addItem(tr("Numeric"));
	boxAxisType->addItem(tr("Text from table"));
	boxAxisType->addItem(tr("Day of the week"));
	boxAxisType->addItem(tr("Month"));
	boxAxisType->addItem(tr("Time"));
	boxAxisType->addItem(tr("Date"));
	boxAxisType->addItem(tr("Column Headings"));
	leftBoxLayout->addWidget( boxAxisType, 0, 1 );

	leftBoxLayout->addWidget( new QLabel(tr( "Font" )), 1, 0 );

	btnAxesFont = new QPushButton();
	btnAxesFont->setText( tr( "Axis &Font" ) );
	leftBoxLayout->addWidget( btnAxesFont, 1, 1 );

	leftBoxLayout->addWidget( new QLabel(tr( "Color" )), 2, 0 );
	boxAxisColor= new ColorButton();
	leftBoxLayout->addWidget( boxAxisColor, 2, 1 );

	leftBoxLayout->addWidget( new QLabel(tr( "Major Ticks" )), 3, 0 );

	boxMajorTicksType= new QComboBox();
	boxMajorTicksType->addItem(tr( "None" ) );
	boxMajorTicksType->addItem(tr( "Out" ) );
	boxMajorTicksType->addItem(tr( "In & Out" ) );
	boxMajorTicksType->addItem(tr( "In" ) );
	leftBoxLayout->addWidget( boxMajorTicksType, 3, 1 );

	leftBoxLayout->addWidget( new QLabel(tr( "Minor Ticks" )), 4, 0 );

	boxMinorTicksType= new QComboBox();
	boxMinorTicksType->addItem(tr( "None" ) );
	boxMinorTicksType->addItem(tr( "Out" ) );
	boxMinorTicksType->addItem(tr( "In & Out" ) );
	boxMinorTicksType->addItem(tr( "In" ) );
	leftBoxLayout->addWidget( boxMinorTicksType, 4, 1);

	leftBoxLayout->addWidget( new QLabel(tr("Stand-off")), 5, 0);
	boxBaseline = new QSpinBox();
	boxBaseline->setRange( 0, 1000 );
	leftBoxLayout->addWidget( boxBaseline );

	boxShowLabels = new QGroupBox(tr("Show Labels"));
	boxShowLabels->setCheckable(true);
	boxShowLabels->setChecked(true);

	bottomLayout->addWidget( boxShowLabels );
	QGridLayout *rightBoxLayout = new QGridLayout( boxShowLabels );

	label1 = new QLabel(tr("Column"));
	rightBoxLayout->addWidget( label1, 0, 0 );

	boxColName = new QComboBox();
	rightBoxLayout->addWidget( boxColName, 0, 1 );

	labelTable = new QLabel(tr("Table"));
	rightBoxLayout->addWidget( labelTable, 1, 0 );

	boxTableName = new QComboBox();
	rightBoxLayout->addWidget( boxTableName, 1, 1 );

	label2 = new QLabel(tr( "Format" ));
	rightBoxLayout->addWidget( label2, 2, 0 );

	boxFormat = new QComboBox();
	boxFormat->setDuplicatesEnabled(false);
	rightBoxLayout->addWidget( boxFormat, 2, 1 );

	label3 = new QLabel(tr( "Precision" ));
	rightBoxLayout->addWidget( label3, 3, 0 );
	boxPrecision = new QSpinBox();
	boxPrecision->setRange( 0, 10 );
	rightBoxLayout->addWidget( boxPrecision, 3, 1 );

	rightBoxLayout->addWidget( new QLabel(tr( "Angle" )), 4, 0 );

	boxAngle = new QSpinBox();
	boxAngle->setRange( -90, 90 );
	boxAngle->setSingleStep(5);
	rightBoxLayout->addWidget( boxAngle, 4, 1 );

    rightBoxLayout->addWidget(new QLabel(tr( "Color" )), 5, 0);
	boxAxisNumColor = new ColorButton();
	rightBoxLayout->addWidget( boxAxisNumColor, 5, 1 );

	boxShowFormula = new QCheckBox(tr( "For&mula" ));
	rightBoxLayout->addWidget( boxShowFormula, 6, 0 );

	boxFormula = new QTextEdit();
	boxFormula->setTextFormat(Qt::PlainText);
	boxFormula->setMaximumHeight(3*metrics.height());
	boxFormula->hide();
	rightBoxLayout->addWidget( boxFormula, 6, 1 );
	rightBoxLayout->setRowStretch(7, 1);

	QVBoxLayout * rightLayout = new QVBoxLayout();
	rightLayout->addLayout( topLayout );
	rightLayout->addLayout( bottomLayout );
	rightLayout->addStretch(1);

	QHBoxLayout * mainLayout3 = new QHBoxLayout( axesPage );
	mainLayout3->addWidget( axesTitlesList );
	mainLayout3->addLayout( rightLayout );

	generalDialog->addTab( axesPage, tr( "Axis" ) );

	connect(buttonLabelFont, SIGNAL(clicked()), this, SLOT(customAxisLabelFont()));

	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(updateShowBox(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(updateAxisColor(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(updateTitleBox(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(setTicksType(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(setAxisType(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(setBaselineDist(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(updateLabelsFormat(int)));

	connect(boxShowLabels,SIGNAL(clicked(bool)), this, SLOT(updateTickLabelsList(bool)));

	connect(boxAxisColor, SIGNAL(clicked()), this, SLOT(pickAxisColor()));
    connect(boxAxisNumColor, SIGNAL(clicked()), this, SLOT(pickAxisNumColor()));
	connect(boxShowFormula, SIGNAL(clicked()), this, SLOT(showFormulaBox()));

	connect(boxMajorTicksType, SIGNAL(activated(int)), this, SLOT(updateMajTicksType(int)));
	connect(boxMinorTicksType, SIGNAL(activated(int)), this, SLOT(updateMinTicksType(int)));

	connect(boxShowAxis,SIGNAL(clicked()), this, SLOT(showAxis()));
	connect(boxFormat, SIGNAL(activated(int) ), this, SLOT(setLabelsNumericFormat(int)));

	connect(btnAxesFont, SIGNAL(clicked()), this, SLOT(customAxisFont()));
	connect(boxBaseline, SIGNAL(valueChanged(int)), this, SLOT(changeBaselineDist(int)));
	connect(boxAxisType, SIGNAL(activated(int)), this, SLOT(showAxisFormatOptions(int)));
	connect(boxPrecision, SIGNAL(valueChanged(int)), this, SLOT(setLabelsNumericFormat(int)));
}

void AxesDialog::initFramePage()
{
	frame = new QWidget();

    boxFramed = new QGroupBox(tr("Canvas frame"));
    boxFramed->setCheckable (true);

    QGridLayout * boxFramedLayout = new QGridLayout( boxFramed );
    boxFramedLayout->addWidget( new QLabel(tr( "Color" )), 0, 0 );
    boxFrameColor= new ColorButton(boxFramed);
    boxFramedLayout->addWidget( boxFrameColor, 0, 1 );

    boxFramedLayout->addWidget( new QLabel(tr( "Width" )), 1, 0 );
    boxFrameWidth= new QSpinBox();
    boxFrameWidth->setMinimum(1);
    boxFramedLayout->addWidget( boxFrameWidth, 1, 1 );

    boxFramedLayout->setRowStretch( 2, 1 );

    QGroupBox * boxAxes = new QGroupBox(tr("Axes"));
    QGridLayout * boxAxesLayout = new QGridLayout( boxAxes );
    boxBackbones = new QCheckBox();
    boxBackbones->setText( tr( "Draw backbones" ) );
    boxAxesLayout->addWidget( boxBackbones, 0, 0 );

    boxAxesLayout->addWidget( new QLabel(tr( "Line Width" )), 1, 0 );
    boxAxesLinewidth = new QSpinBox();
    boxAxesLinewidth->setRange( 1, 100 );
    boxAxesLayout->addWidget( boxAxesLinewidth, 1, 1 );

    boxAxesLayout->addWidget( new QLabel(tr( "Major ticks length" )), 2, 0 );
    boxMajorTicksLength = new QSpinBox();
    boxMajorTicksLength->setRange( 0, 1000 );
    boxAxesLayout->addWidget( boxMajorTicksLength, 2, 1 );

    boxAxesLayout->addWidget( new QLabel(tr( "Minor ticks length" )), 3, 0 );
    boxMinorTicksLength = new QSpinBox();
    boxMinorTicksLength->setRange( 0, 1000 );
    boxAxesLayout->addWidget( boxMinorTicksLength, 3, 1 );

    boxAxesLayout->setRowStretch( 4, 1 );

    QHBoxLayout * mainLayout = new QHBoxLayout( frame );
    mainLayout->addWidget(boxFramed);
    mainLayout->addWidget(boxAxes);

    generalDialog->addTab(frame, tr( "General" ) );

	connect(boxFrameColor, SIGNAL(clicked()), this, SLOT(pickCanvasFrameColor()));
	connect(boxBackbones, SIGNAL(toggled(bool)), this, SLOT(drawAxesBackbones(bool)));
	connect(boxFramed, SIGNAL(toggled(bool)), this, SLOT(drawFrame(bool)));
	connect(boxFrameWidth, SIGNAL(valueChanged (int)), this, SLOT(updateFrame(int)));
	connect(boxAxesLinewidth, SIGNAL(valueChanged (int)), this, SLOT(changeAxesLinewidth(int)));
	connect(boxMajorTicksLength, SIGNAL(valueChanged (int)), this, SLOT(changeMajorTicksLength(int)));
	connect(boxMinorTicksLength, SIGNAL(valueChanged (int)), this, SLOT(changeMinorTicksLength(int)));
}

void AxesDialog::changeMinorTicksLength (int minLength)
{
	if (generalDialog->currentWidget() != frame)
		return;

    m_layer->changeTicksLength(minLength, boxMajorTicksLength->value());
	boxMajorTicksLength->setMinimum(minLength);
}

void AxesDialog::changeMajorTicksLength (int majLength)
{
	if (generalDialog->currentWidget() != frame)
		return;

    m_layer->changeTicksLength(boxMinorTicksLength->value(), majLength);
	boxMinorTicksLength->setMaxValue(majLength);
}

void AxesDialog::drawAxesBackbones(bool draw)
{
	if (generalDialog->currentWidget() != frame)
		return;

	m_layer->drawAxesBackbones(draw);
}

void AxesDialog::changeAxesLinewidth(int width)
{
	if (generalDialog->currentWidget() != frame)
		return;

    m_layer->setAxesLinewidth(width);
}

void AxesDialog::drawFrame(bool framed)
{
	if (generalDialog->currentWidget() != frame)
		return;

	m_layer->drawCanvasFrame(framed, boxFrameWidth->value(), boxFrameColor->color());
}

void AxesDialog::updateFrame(int width)
{
	if (generalDialog->currentWidget() != frame)
		return;

    m_layer->drawCanvasFrame(boxFramed->isChecked(), width, boxFrameColor->color());
}

void AxesDialog::pickCanvasFrameColor()
{
	QColor c = QColorDialog::getColor(boxFrameColor->color(), this);
		if ( !c.isValid() || c == boxFrameColor->color() )
			return;

	boxFrameColor->setColor ( c ) ;
	m_layer->drawCanvasFrame(boxFramed->isChecked(), boxFrameWidth->value(), c);
}

void AxesDialog::showAxisFormatOptions(int format)
{
	int axis=mapToQwtAxisId();

	boxFormat->clear();
	boxFormat->setEditable(false);
	boxFormat->hide();
	boxPrecision->hide();
	boxColName->hide();
	label1->hide();
	label2->hide();
	label3->hide();
	boxShowFormula->hide();
	boxFormula->hide();
	boxTableName->hide();
	labelTable->hide();

	switch (format)
	{
		case 0:
			label2->show();
			boxFormat->show();
			boxFormat->insertItem(tr( "Automatic" ) );
			boxFormat->insertItem(tr( "Decimal: 100.0" ) );
			boxFormat->insertItem(tr( "Scientific: 1e2" ) );
			boxFormat->insertItem(tr( "Scientific: 10^2" ) );
			boxFormat->setCurrentIndex(m_layer->plotWidget()->axisLabelFormat(axis));

			label3->show();
			boxPrecision->show();
			boxShowFormula->show();

			showAxisFormula(mapToQwtAxisId());
			break;

		case 1:
			label1->show();
			boxColName->show();
			break;

		case 2:
        {
            int day = (QDate::currentDate()).dayOfWeek();
            label2->show();
            boxFormat->show();
            boxFormat->insertItem(QDate::shortDayName(day));
            boxFormat->insertItem(QDate::longDayName(day));
            boxFormat->insertItem((QDate::shortDayName(day)).left(1));
            boxFormat->setCurrentIndex (formatInfo[axis].toInt());
        }
        break;

		case 3:
        {
            int month = (QDate::currentDate()).month();
            label2->show();
            boxFormat->show();
            boxFormat->insertItem(QDate::shortMonthName(month));
            boxFormat->insertItem(QDate::longMonthName(month));
            boxFormat->insertItem((QDate::shortMonthName(month)).left(1));
            boxFormat->setCurrentIndex (formatInfo[axis].toInt());
        }
        break;

		case 4:
        {
            label2->show();
            boxFormat->show();
            boxFormat->setEditable(true);

            QStringList lst = formatInfo[axis].split(";", QString::KeepEmptyParts);
            if (lst.count() == 2)
            {
                boxFormat->insertItem(lst[1]);
                boxFormat->setCurrentText(lst[1]);
            }

            boxFormat->insertItem("h");
            boxFormat->insertItem("h ap");
            boxFormat->insertItem("h AP");
            boxFormat->insertItem("h:mm");
            boxFormat->insertItem("h:mm ap");
            boxFormat->insertItem("hh:mm");
            boxFormat->insertItem("h:mm:ss");
            boxFormat->insertItem("h:mm:ss.zzz");
            boxFormat->insertItem("mm:ss");
            boxFormat->insertItem("mm:ss.zzz");
            boxFormat->insertItem("hmm");
            boxFormat->insertItem("hmmss");
            boxFormat->insertItem("hhmmss");
        }
        break;

		case 5:
        {
            label2->show();
            boxFormat->show();
            boxFormat->setEditable(true);

            QStringList lst = formatInfo[axis].split(";", QString::KeepEmptyParts);
            if (lst.count() == 2)
            {
                boxFormat->insertItem(lst[1]);
                boxFormat->setCurrentText(lst[1]);
            }
            boxFormat->insertItem("yyyy-MM-dd");
            boxFormat->insertItem("dd.MM.yyyy");
            boxFormat->insertItem("ddd MMMM d yy");
            boxFormat->insertItem("dd/MM/yyyy");
        }
        break;

		case 6:
        {
            labelTable->show();
            if (tablesList.contains(formatInfo[axis]))
                boxTableName->setCurrentText(formatInfo[axis]);
            boxTableName->show();
        }
        break;
	}
}

void AxesDialog::insertColList(const QStringList& cols)
{
	boxColName-> insertStringList(cols);
}

void AxesDialog::insertTablesList(const QStringList& l)
{
	tablesList = l;
		boxTableName->insertStringList(l);
}

void AxesDialog::updateAxisType(int)
{
int a=mapToQwtAxisId();
boxAxisType->setCurrentIndex(a);
}

void AxesDialog::setEnabledAxes(QVector<bool> ok)
{
	xAxisOn=ok[2];
		yAxisOn=ok[0];
		topAxisOn=ok[3];
	rightAxisOn=ok[1];
}

void AxesDialog::showAxis()
{
	bool ok=boxShowAxis->isChecked();
	boxTitle->setEnabled(ok);
	boxAxisColor->setEnabled(ok);
    boxAxisNumColor->setEnabled(ok);
	btnAxesFont->setEnabled(ok);
	boxShowLabels->setEnabled(ok);
	boxMajorTicksType->setEnabled(ok);
	boxMinorTicksType->setEnabled(ok);
	boxAxisType->setEnabled(ok);
	boxBaseline->setEnabled(ok);
	labelBox->setEnabled(ok);

		int axis=-1;
		int a=axesTitlesList->currentRow();
		switch(a)
		{
			case 0:
				   {
					   axis = QwtPlot::xBottom;
					   xAxisOn=ok;
					   break;
				   }
			case 1:
				   {
					   axis = QwtPlot::yLeft;
					   yAxisOn=ok;
					   break;
				   }
			case 2:
				   {
					   axis = QwtPlot::xTop;
					   topAxisOn=ok;
					   break;
				   }
			case 3:
				   {
					   axis = QwtPlot::yRight;
					   rightAxisOn=ok;
					   break;
				   }
		}

		bool labels=false;
		if (tickLabelsOn[axis] == "1")
			labels=true;

				boxFormat->setEnabled(labels && ok);
				boxColName->setEnabled(labels && ok);
				boxShowFormula->setEnabled(labels && ok);
	boxFormula->setEnabled(labels && ok);

	if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
		boxAngle->setEnabled(labels && ok);
	else
		boxAngle->setDisabled(true);

	bool userFormat=true;
    if (boxFormat->currentIndex() == 0)
	       userFormat=false;
	boxPrecision->setEnabled(labels && ok && userFormat);

	QString formula =  boxFormula->text();
	if (!boxShowFormula->isChecked())
	   formula = QString();

    emit showAxis(axis,boxAxisType->currentIndex(), boxColName->currentText(),ok, boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
                  boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(),
                  boxPrecision->value(), boxAngle->value(), boxBaseline->value(), formula, boxAxisNumColor->color());
}

void AxesDialog::updateShowBox(int axis)
{
	switch(axis)
	{
		case 0:
			{
				boxShowAxis->setChecked(xAxisOn);
				int labelsOn=tickLabelsOn[2].toInt();
				boxShowLabels->setChecked(labelsOn);
				boxAngle->setEnabled(labelsOn && xAxisOn);
				boxFormat->setEnabled(labelsOn && xAxisOn);
				boxAngle->setValue(xBottomLabelsRotation);
				break;
			}
		case 1:
			{
				boxShowAxis->setChecked(yAxisOn);
				int labelsOn=tickLabelsOn[0].toInt();
				boxShowLabels->setChecked(labelsOn);
				boxFormat->setEnabled(labelsOn && yAxisOn);
				boxAngle->setEnabled(false);
				boxAngle->setValue(0);
				break;
			}
		case 2:
			{
				boxShowAxis->setChecked(topAxisOn);

				int labelsOn=tickLabelsOn[3].toInt();
				boxShowLabels->setChecked(labelsOn);
				boxFormat->setEnabled(labelsOn && topAxisOn);
				boxAngle->setEnabled(labelsOn && topAxisOn);
				boxAngle->setValue(xTopLabelsRotation);
				break;
			}
		case 3:
			{
				boxShowAxis->setChecked(rightAxisOn);
				int labelsOn=tickLabelsOn[1].toInt();
				boxShowLabels->setChecked(labelsOn );
				boxFormat->setEnabled(labelsOn && rightAxisOn);
				boxAngle->setEnabled(false);
				boxAngle->setValue(0);
				break;
			}
	}

	bool ok=boxShowAxis->isChecked();
	boxTitle->setEnabled(ok);
	boxAxisColor->setEnabled(ok);
    boxAxisNumColor->setEnabled(ok);
	btnAxesFont->setEnabled(ok);
	boxShowLabels->setEnabled(ok);
	boxMajorTicksType->setEnabled(ok);
	boxMinorTicksType->setEnabled(ok);
	boxAxisType->setEnabled(ok);
	boxBaseline->setEnabled(ok);
	labelBox->setEnabled(ok);
}

void AxesDialog::initAxisFonts(const QFont& xB, const QFont& yL, const QFont& xT, const QFont& yR )
{
	xBottomFont=xB;
		yLeftFont=yL;
		xTopFont=xT;
	yRightFont=yR;
}

void AxesDialog::customAxisFont()
{
	bool okF;
		int axis=-1;
		QFont fnt;
		switch(axesTitlesList->currentRow())
		{
			case 0:
				   {
					   axis = QwtPlot::xBottom;
					   fnt = QFontDialog::getFont( &okF, xBottomFont,this);
					   if (okF)
						   xBottomFont=fnt;
					   break;
				   }
			case 1:
				   {
					   axis = QwtPlot::yLeft;
					   fnt = QFontDialog::getFont( &okF, yLeftFont,this);
					   if (okF)
						   yLeftFont=fnt;
					   break;
				   }
			case 2:
				   {
					   axis = QwtPlot::xTop;
					   fnt = QFontDialog::getFont( &okF, xTopFont, this);
					   if (okF)
						   xTopFont=fnt;
					   break;
				   }
			case 3:
				   {
					   axis = QwtPlot::yRight;
					   fnt = QFontDialog::getFont( &okF, yRightFont, this);
					   if (okF)
						   yRightFont=fnt;
					   break;
				   }
		}

	emit changeAxisFont(axis,fnt);
}

void AxesDialog::accept()
{
	if (updatePlot())
		close();
}

void AxesDialog::majorGridEnabled(bool on)
{
	boxMinorGrid->setEnabled(on);

	boxTypeMajor->setEnabled(on);
	boxColorMajor->setEnabled(on);
	boxWidthMajor->setEnabled(on);

	if (generalDialog->currentWidget()==gridPage)
		{
		m_layer->setGridOptions(getGridOptions());
		m_layer->replot();
		}
}

void AxesDialog::minorGridEnabled(bool on)
{
	boxTypeMinor->setEnabled(on);
	boxColorMinor->setEnabled(on);
	boxWidthMinor->setEnabled(on);

	if (generalDialog->currentWidget()==gridPage)
		{
		m_layer->setGridOptions(getGridOptions());
	    m_layer->replot();
		}
}

void AxesDialog::setGridOptions()
{
	putGridOptions(grid);
}

void AxesDialog::putGridOptions(GridOptions gr)
{
if (axesGridList->currentRow())
	{
	boxMajorGrid->setChecked(gr.majorOnX);
	boxMinorGrid->setChecked(gr.minorOnX);

	boxXLine->setEnabled(true);
	boxYLine->setDisabled(true);

	boxGridXAxis->setEnabled(true);
	boxGridYAxis->setDisabled(true);
	}
else
	{
	boxMajorGrid->setChecked(gr.majorOnY);
	boxMinorGrid->setChecked(gr.minorOnY);

	boxXLine->setDisabled(true);
	boxYLine->setEnabled(true);

	boxGridXAxis->setDisabled(true);
	boxGridYAxis->setEnabled(true);
	}

boxGridXAxis->setCurrentItem(gr.xAxis - 2);
boxGridYAxis->setCurrentItem(gr.yAxis);
boxTypeMajor->setCurrentItem(gr.majorStyle);
boxColorMajor->setCurrentItem(gr.majorCol);
boxWidthMajor->setValue(gr.majorWidth);
boxTypeMinor->setCurrentItem(gr.minorStyle);
boxColorMinor->setCurrentItem(gr.minorCol);
boxWidthMinor->setValue(gr.minorWidth);
boxXLine->setChecked(gr.xZeroOn);
boxYLine->setChecked(gr.yZeroOn);

grid = gr;
}

GridOptions AxesDialog::getGridOptions()
{
	if (boxMajorGrid->isChecked() && axesGridList->currentRow()==1)
		grid.majorOnX=1;
	else if (!boxMajorGrid->isChecked() && axesGridList->currentRow()==1)
		grid.majorOnX=0;
	else if (boxMajorGrid->isChecked() && axesGridList->currentRow()==0)
		grid.majorOnY=1;
	else
		grid.majorOnY=0;

    grid.majorStyle=boxTypeMajor->currentIndex();
	grid.majorCol=boxColorMajor->currentIndex();
	grid.majorWidth=boxWidthMajor->value();

	if (boxMinorGrid->isChecked() && axesGridList->currentRow()==1)
	   grid.minorOnX=1;
	else if (!boxMinorGrid->isChecked() && axesGridList->currentRow()==1)
       grid.minorOnX=0;
	else if (boxMinorGrid->isChecked() && axesGridList->currentRow()==0)
	     grid.minorOnY=1;
	else
	     grid.minorOnY=0;

	grid.minorStyle=boxTypeMinor->currentIndex();
	grid.minorCol=boxColorMinor->currentIndex();
	grid.minorWidth=boxWidthMinor->value();

	if (boxXLine->isChecked())
	   grid.xZeroOn=1;
	else grid.xZeroOn=0;

	if (boxYLine->isChecked())
		grid.yZeroOn=1;
	else grid.yZeroOn=0;

    grid.yAxis = boxGridYAxis->currentIndex();
  	grid.xAxis = boxGridXAxis->currentIndex() + 2;

	return grid;
}

void AxesDialog::updateAxisColor(int)
{
	int a = mapToQwtAxisId();
    boxAxisColor->setColor(m_layer->axisColor(a));
    boxAxisNumColor->setColor(m_layer->axisNumbersColor(a));
}

void AxesDialog::changeBaselineDist(int baseline)
{
	int axis=mapToQwtAxisId();
	axesBaseline[axis] = baseline;

	int at=axesTitlesList->currentRow();
	QString title=boxTitle->text();
	if (titles[at] != title)
	{
		emit updateAxisTitle(at,title);
		titles[at] = title;
	}

	QString formula =  boxFormula->text();
	if (!boxShowFormula->isChecked())
		formula = QString();
	emit showAxis(axis, boxAxisType->currentIndex(), formatInfo[axis], boxShowAxis->isChecked(), boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
			boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(), boxPrecision->value(),
			boxAngle->value(), baseline, formula, boxAxisNumColor->color());
}

bool AxesDialog::updatePlot()
{
	if (generalDialog->currentWidget()==(QWidget*)scalesPage)
	{
		QString from=boxStart->text().toLower();
		QString to=boxEnd->text().toLower();
		QString step=boxStep->text().toLower();
        int a = mapToQwtAxis(axesList->currentRow());
		double start, end, stp = 0;
		try
		{
			MyParser parser;
			parser.SetExpr(from.toAscii().constData());
			start=parser.Eval();
		}
		catch(mu::ParserError &e)
		{
			QMessageBox::critical(0, tr("Start limit error"),QString::fromStdString(e.GetMsg()));
			boxStart->setFocus();
			return false;
		}
		try
		{
			MyParser parser;
			parser.SetExpr(to.toAscii().constData());
			end=parser.Eval();
		}
		catch(mu::ParserError &e)
		{
			QMessageBox::critical(0, tr("End limit error"),QString::fromStdString(e.GetMsg()));
			boxEnd->setFocus();
			return false;
		}
        if (btnStep->isChecked())
  	        {
		    try
		       {
			    MyParser parser;
			    parser.SetExpr(step.toAscii().constData());
			    stp=parser.Eval();
		       }
           catch(mu::ParserError &e)
              {
			  QMessageBox::critical(0, tr("Step input error"),QString::fromStdString(e.GetMsg()));
			  boxStep->setFocus();
			  return false;
		      }

		      if (stp <=0)
		      {
			  QMessageBox::critical(0,tr("Step input error"), tr("Please enter a positive step value!"));
			  boxStep->setFocus();
			  return false;
		      }

           if (axesType[a] == Layer::Time)
		      {
		      switch (boxUnit->currentIndex())
                 {
			     case 0:
			     break;
			     case 1:
				      stp *= 1e3;
			     break;
			     case 2:
				   stp *= 6e4;
                 break;
			     case 3:
				     stp *= 36e5;
		         break;
			     }
		      }
	       else if (axesType[a] == Layer::Date)
		        {
		        switch (boxUnit->currentIndex())
                    {
                    case 0:
                    break;
                    case 1:
                         stp *= 7;
                    break;
                    }
	             }
          }

		m_layer->setScale(a, start, end, stp, boxMajorValue->value(), boxMinorValue->currentText().toInt(),
                             boxScaleType->currentIndex(), btnInvert->isChecked());
		m_layer->notifyChanges();
	}
	else if (generalDialog->currentWidget()==gridPage)
	{
		m_layer->setGridOptions(getGridOptions());
		m_layer->replot();
	}
	else if (generalDialog->currentWidget()==(QWidget*)axesPage)
	{
		int axis=mapToQwtAxisId();
		int format = boxAxisType->currentIndex();
		axesType[axis] = format;

		int baseline = boxBaseline->value();
		axesBaseline[axis] = baseline;

		if (format == Layer::Numeric)
		{
			if (boxShowFormula->isChecked())
			{
				QString formula = boxFormula->text().toLower();
				try
				{
					double value = 1.0;
					MyParser parser;
					if (formula.contains("x"))
						parser.DefineVar("x", &value);
					else if (formula.contains("y"))
						parser.DefineVar("y", &value);
					parser.SetExpr(formula.toAscii().constData());
					parser.Eval();
				}
				catch(mu::ParserError &e)
				{
					QMessageBox::critical(0, tr("Formula input error"), QString::fromStdString(e.GetMsg())+"\n"+
							tr("Valid variables are 'x' for Top/Bottom axes and 'y' for Left/Right axes!"));
					boxFormula->setFocus();
					return false;
				}
			}
		}
		else if (format == Layer::Time || format == Layer::Date)
		{
			QStringList lst = formatInfo[axis].split(";", QString::KeepEmptyParts);
			if ((int)lst.count() >= 2)
				lst[1] = boxFormat->currentText();
			formatInfo[axis]  = lst.join(";");
		}
		else if (format == Layer::Day || format == Layer::Month)
			formatInfo[axis] = QString::number(boxFormat->currentIndex());
		else if (format == Layer::ColHeader)
			formatInfo[axis] = boxTableName->currentText();
		else
			formatInfo[axis] = boxColName->currentText();

		int at=axesTitlesList->currentRow();
		QString title=boxTitle->text();
		if (titles[at] != title)
		{
			emit updateAxisTitle(at, title);
			titles[at] = title;
		}

		if (axis == QwtPlot::xBottom)
			xBottomLabelsRotation=boxAngle->value();
		else if (axis == QwtPlot::xTop)
			xTopLabelsRotation=boxAngle->value();

		QString formula =  boxFormula->text();
		if (!boxShowFormula->isChecked())
			formula = QString();
		emit showAxis(axis, format, formatInfo[axis], boxShowAxis->isChecked(), boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
				boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(),
				boxPrecision->value(), boxAngle->value(), baseline, formula, boxAxisNumColor->color());
	}
	else if (generalDialog->currentWidget()==(QWidget*)frame)
	{
		m_layer->setAxesLinewidth(boxAxesLinewidth->value());
        m_layer->changeTicksLength(boxMinorTicksLength->value(), boxMajorTicksLength->value());
        m_layer->drawCanvasFrame(boxFramed->isChecked(), boxFrameWidth->value(), boxFrameColor->color());
        m_layer->drawAxesBackbones(boxBackbones->isChecked());
	}

	return true;
}

void AxesDialog::setLayer(Layer *g)
{
	if (!g)
        return;

	m_layer = g;
	Plot *p = m_layer->plotWidget();

	boxAxesLinewidth->setValue(p->axesLinewidth());
    boxBackbones->setChecked (m_layer->axesBackbones());

	boxFramed->setChecked(m_layer->framed());
	boxFrameColor->setColor(m_layer->canvasFrameColor());
	boxFrameWidth->setValue(m_layer->canvasFrameWidth());

	boxMinorTicksLength->setValue(p->minorTickLength());
	boxMajorTicksLength->setValue(p->majorTickLength());
}

int AxesDialog::mapToQwtAxisId()
{
return mapToQwtAxis(axesTitlesList->currentRow());
}

int AxesDialog::mapToQwtAxis(int axis)
{
int a=-1;
switch(axis)
        {
        case 0:
           a = QwtPlot::xBottom;
        break;
        case 1:
            a = QwtPlot::yLeft;
        break;
        case 2:
             a = QwtPlot::xTop;
        break;
        case 3:
             a = QwtPlot::yRight;
        break;
		}
return a;
}

void AxesDialog::updateScale()
{
int axis = axesList->currentRow();

boxStart->clear();
boxEnd->clear();
boxStep->clear();
boxUnit->hide();
boxUnit->clear();

Plot *m_plot = m_layer->plotWidget();
int a = mapToQwtAxis(axis);
const QwtScaleDiv *scDiv=m_plot->axisScaleDiv(a);
boxStart->setText(QString::number(QMIN(scDiv->lBound(), scDiv->hBound())));
boxEnd->setText(QString::number(QMAX(scDiv->lBound(), scDiv->hBound())));

QwtValueList lst = scDiv->ticks (QwtScaleDiv::MajorTick);
boxStep->setText(QString::number(m_layer->axisStep(a)));
boxMajorValue->setValue(lst.count());

if (axesType[a] == Layer::Time)
	{
	boxUnit->show();
	boxUnit->insertItem(tr("millisec."));
	boxUnit->insertItem(tr("sec."));
	boxUnit->insertItem(tr("min."));
	boxUnit->insertItem(tr("hours"));
	}
else if (axesType[a] == Layer::Date)
	{
	boxUnit->show();
	boxUnit->insertItem(tr("days"));
	boxUnit->insertItem(tr("weeks"));
	}

if (m_layer->axisStep(a) != 0.0)
	{
	btnStep->setChecked(true);
	boxStep->setEnabled(true);
	boxUnit->setEnabled(true);

	btnMajor->setChecked(false);
	boxMajorValue->setEnabled(false);
	}
else
	{
	btnStep->setChecked(false);
	boxStep->setEnabled(false);
	boxUnit->setEnabled(false);
	btnMajor->setChecked(true);
	boxMajorValue->setEnabled(true);
	}

const QwtScaleEngine *sc_eng = m_plot->axisScaleEngine(a);
btnInvert->setChecked(sc_eng->testAttribute(QwtScaleEngine::Inverted));

QwtScaleTransformation *tr = sc_eng->transformation();
boxScaleType->setCurrentItem((int)tr->type());

boxMinorValue->clear();
if (tr->type())//log scale
	boxMinorValue->addItems(QStringList()<<"0"<<"2"<<"4"<<"8");
else
	boxMinorValue->addItems(QStringList()<<"0"<<"1"<<"4"<<"9"<<"14"<<"19");

boxMinorValue->setEditText(QString::number(m_plot->axisMaxMinor(a)));
}

void AxesDialog::updateTitleBox(int axis)
{
	boxTitle->setText(titles[axis]);
}

void AxesDialog::setAxisTitles(QStringList t)
{
	titles=t;
}

void AxesDialog::pickAxisColor()
{
	QColor c = QColorDialog::getColor( boxAxisColor->color(), this);
		if ( !c.isValid() || c ==  boxAxisColor->color() )
			return;

	boxAxisColor->setColor ( c ) ;

	int axis=mapToQwtAxisId();
	QString formula =  boxFormula->text();
	if (!boxShowFormula->isChecked())
	   formula = QString();

    emit showAxis(axis, boxAxisType->currentIndex(), formatInfo[axis], boxShowAxis->isChecked(), boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
                  boxShowLabels->isChecked(), c, boxFormat->currentIndex(), boxPrecision->value(),
                  boxAngle->value(), boxBaseline->value(), formula, boxAxisNumColor->color());
}

void AxesDialog::pickAxisNumColor()
{
  	QColor c = QColorDialog::getColor( boxAxisNumColor->color(), this);
  	if ( !c.isValid() || c == boxAxisNumColor->color() )
  	     return;

  	boxAxisNumColor->setColor ( c ) ;
  	int axis=mapToQwtAxisId();
  	QString formula = boxFormula->text();
  	if (!boxShowFormula->isChecked())
  	     formula = QString::null;

  	emit showAxis(axis, boxAxisType->currentIndex(), formatInfo[axis], boxShowAxis->isChecked(), boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
  	              boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(), boxPrecision->value(),
  	              boxAngle->value(), boxBaseline->value(), formula, c);
}

void AxesDialog::setAxisType(int)
{
	int a = mapToQwtAxisId();
	int style = axesType[a];

	boxAxisType->setCurrentIndex(style);
	showAxisFormatOptions(style);

	if (style == 1)
		boxColName->setCurrentText(formatInfo[a]);
}

void AxesDialog::setBaselineDist(int)
{
	int a=mapToQwtAxisId();
		boxBaseline->setValue(axesBaseline[a]);
}

void AxesDialog::setAxesLabelsFormatInfo(const QStringList& lst)
{
	formatInfo = lst;
}

void AxesDialog::setAxesType(const QList<int>& list)
{
	axesType=list;
}

void AxesDialog::setAxesBaseline(const QList<int>& lst)
{
	axesBaseline = lst;
}

void AxesDialog::setTicksType(const QList<int>& majLst, const QList<int>& minLst)
{
	majTicks=majLst;
		minTicks=minLst;
}

void AxesDialog::setTicksType(int)
{
	int a=mapToQwtAxisId();
		boxMajorTicksType->setCurrentIndex(majTicks[a]);
		boxMinorTicksType->setCurrentIndex(minTicks[a]);
}

void AxesDialog::updateMajTicksType(int)
{
	int axis=mapToQwtAxisId();
		int type=boxMajorTicksType->currentIndex();
		if ( majTicks[axis] == type)
			return;

	majTicks[axis]=type;
	QString formula =  boxFormula->text();
	if (!boxShowFormula->isChecked())
		formula = QString();

    emit showAxis(axis,boxAxisType->currentIndex(),formatInfo[axis], boxShowAxis->isChecked(), type, boxMinorTicksType->currentIndex(),
			     boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(),boxPrecision->value(),
			     boxAngle->value(), boxBaseline->value(), formula, boxAxisNumColor->color());
}

void AxesDialog::updateMinTicksType(int)
{
	int axis=mapToQwtAxisId();
		int type=boxMinorTicksType->currentIndex();
		if ( minTicks[axis] == type)
			return;

	minTicks[axis]=type;
	QString formula =  boxFormula->text();

    if (!boxShowFormula->isChecked())
		formula = QString();

    emit showAxis(axis,boxAxisType->currentIndex(),formatInfo[axis], boxShowAxis->isChecked(), boxMajorTicksType->currentIndex(), type,
				  boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(),boxPrecision->value(),
				  boxAngle->value(), boxBaseline->value(), formula, boxAxisNumColor->color());
}

void AxesDialog::setEnabledTickLabels(const QStringList& labelsOn)
{
	tickLabelsOn=labelsOn;
}

void AxesDialog::updateTickLabelsList(bool on)
{
	int axis = mapToQwtAxisId();
	if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
		boxAngle->setEnabled(on);

	bool userFormat = true;
	if (boxFormat->currentIndex() == 0)
		userFormat = false;
	boxPrecision->setEnabled(on && userFormat);

	if (tickLabelsOn[axis] == QString::number(on))
		return;
	tickLabelsOn[axis]=QString::number(on);

	int type = boxAxisType->currentIndex();
	if (type == Layer::Day || type == Layer::Month)
		formatInfo[axis] = QString::number(boxFormat->currentIndex());
	else if (type == Layer::Time || type == Layer::Date)
	{
		QStringList lst = formatInfo[axis].split(";", QString::SkipEmptyParts);
		lst[1] = boxFormat->currentText();
		formatInfo[axis]  = lst.join(";");
	}
	else
		formatInfo[axis] = boxColName->currentText();

	QString formula = boxFormula->text();
	if (!boxShowFormula->isChecked())
	   formula = QString();

    emit showAxis(axis, type, formatInfo[axis], boxShowAxis->isChecked(), boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
                  boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(), boxPrecision->value(),
	              boxAngle->value(), boxBaseline->value(), formula, boxAxisNumColor->color());
}

void AxesDialog::setCurrentScale(int axisPos)
{
int axis = -1;
switch (axisPos)
	{
	case QwtScaleDraw::LeftScale:
		axis = 1;
	break;
	case QwtScaleDraw::BottomScale:
		axis = 0;
	break;
	case QwtScaleDraw::RightScale:
		axis = 3;
	break;
	case QwtScaleDraw::TopScale:
		axis = 2;
	break;
	}
if (generalDialog->currentWidget()==(QWidget*)scalesPage)
	axesList->setCurrentRow(axis);
else if (generalDialog->currentWidget()==(QWidget*)axesPage)
	axesTitlesList->setCurrentRow(axis);
}

void AxesDialog::showAxesPage()
{
	if (generalDialog->currentWidget()!=(QWidget*)axesPage)
		generalDialog->setCurrentWidget(axesPage);
}

void AxesDialog::showGridPage()
{
	if (generalDialog->currentWidget()!=(QWidget*)gridPage)
		generalDialog->setCurrentWidget(gridPage);
}

void AxesDialog::updateGrid(int)
{
	if (generalDialog->currentWidget()==gridPage)
	{
		m_layer->setGridOptions(getGridOptions());
			m_layer->replot();
	}
}

void AxesDialog::setLabelsNumericFormat(int)
{
	int axis = mapToQwtAxisId();
	int type = boxAxisType->currentIndex();
	int prec = boxPrecision->value();
	int format = boxFormat->currentIndex();

	Plot *plot = m_layer->plotWidget();

	if (type == Layer::Numeric)
	{
		if (plot->axisLabelFormat(axis) == format &&
			plot->axisLabelPrecision(axis) == prec)
			return;

		if (format == 0)
			boxPrecision->setEnabled(false);
		else
			boxPrecision->setEnabled(true);
		}
	else if (type == Layer::Day || type == Layer::Month)
		formatInfo[axis] = QString::number(format);
	else if (type == Layer::Time || type == Layer::Date)
	{
		QStringList lst = formatInfo[axis].split(";", QString::SkipEmptyParts);
		lst[1] = boxFormat->currentText();
		formatInfo[axis]  = lst.join(";");
	}
	else
		formatInfo[axis] = boxColName->currentText();

	QString formula =  boxFormula->text();
	if (!boxShowFormula->isChecked())
		formula = QString();

    emit showAxis(axis, type, formatInfo[axis], boxShowAxis->isChecked(), boxMajorTicksType->currentIndex(),
			boxMinorTicksType->currentIndex(), boxShowLabels->isChecked(), boxAxisColor->color(),
			format, prec, boxAngle->value(), boxBaseline->value(), formula, boxAxisNumColor->color());

    axesType[axis] = type;
}

void AxesDialog::showAxisFormula(int axis)
{
	QStringList l = m_layer->getAxesFormulas();
		QString formula = l[axis];
		if (!formula.isEmpty())
		{
			boxShowFormula->setChecked(true);
			boxFormula->show();
			boxFormula->setText(formula);
		}
		else
		{
			boxShowFormula->setChecked(false);
			boxFormula->clear();
			boxFormula->hide();
		}
}

void AxesDialog::updateLabelsFormat(int)
{
        if (boxAxisType->currentIndex() != Layer::Numeric)
        	return;

		int a = mapToQwtAxisId();
		int format = m_layer->plotWidget()->axisLabelFormat(a);
        boxFormat->setCurrentIndex(format);
		boxPrecision->setValue(m_layer->plotWidget()->axisLabelPrecision(a));

        if (format == 0)
        	boxPrecision->setEnabled(false);
        else
        	boxPrecision->setEnabled(true);

        QStringList l = m_layer->getAxesFormulas();
        QString formula = l[a];
        if (!formula.isEmpty())
        {
        	boxShowFormula->setChecked(true);
        	boxFormula->show();
        	boxFormula->setText(formula);
        }
        else
        {
        	boxShowFormula->setChecked(false);
        	boxFormula->clear();
        	boxFormula->hide();
        }
}

void AxesDialog::initLabelsRotation(int xAngle, int yAngle)
{
	xBottomLabelsRotation=xAngle;
		xTopLabelsRotation=yAngle;
}

void AxesDialog::showGeneralPage()
{
	generalDialog->setCurrentIndex(generalDialog->indexOf(frame));
}

void AxesDialog::showFormulaBox()
{
	if (boxShowFormula->isChecked())
		boxFormula->show();
	else
		boxFormula->hide();
}

void AxesDialog::customAxisLabelFont()
{
	int axis = mapToQwtAxisId();
		bool okF;
		QFont oldFont = m_layer->axisTitleFont(axis);
	QFont fnt = QFontDialog::getFont( &okF, oldFont,this);
	if (okF && fnt != oldFont)
		m_layer->setAxisTitleFont(axis, fnt);
}

void AxesDialog::pageChanged ( QWidget *page )
{
  	if (lastPage == scalesPage && page == axesPage)
  	{
  		axesTitlesList->setCurrentRow(axesList->currentRow());
  	    lastPage = page;
  	}
  	else if (lastPage == axesPage && page == scalesPage)
  	{
  		axesList->setCurrentRow(axesTitlesList->currentRow());
  	    lastPage = page;
  	}
}

int AxesDialog::exec()
{
	axesList->setCurrentRow(0);
	axesGridList->setCurrentRow(0);
	axesTitlesList->setCurrentRow(0);

    setModal(true);
    show();
	return 0;
}

void AxesDialog::updateMinorTicksList(int scaleType)
{
	updatePlot();

	boxMinorValue->clear();
	if (scaleType)//log scale
		boxMinorValue->addItems(QStringList()<<"0"<<"2"<<"4"<<"8");
	else
		boxMinorValue->addItems(QStringList()<<"0"<<"1"<<"4"<<"9"<<"14"<<"19");

	int a = mapToQwtAxis(axesList->currentRow());
	boxMinorValue->setEditText(QString::number(m_layer->plotWidget()->axisMaxMinor(a)));
}
