/***************************************************************************
    File                 : PlotDialog.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Custom curves dialog

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
#ifndef PLOTDIALOG_H
#define PLOTDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "Graph.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QWidget;
class QStringList;
class QGroupBox;
class QDoubleSpinBox;
class QRadioButton;
class QTreeWidget;

class LayerItem;
class CurveTreeItem;
class ColorBox;
class PatternBox;
class ColorButton;
class Graph;
class SymbolBox;
class ColorMapEditor;
class QwtPlotItem;

//! Custom plot/curves dialog
class PlotDialog : public QDialog
{
	Q_OBJECT

	public:
		PlotDialog(bool showExtended, QWidget* parent = 0, Qt::WFlags fl = 0 );
		void initFonts(const QFont& titlefont, const QFont& axesfont, const QFont& numbersfont, const QFont& legendfont);
		void insertColumnsList(const QStringList& names){columnNames = names;};
		void setGraph(Graph *ml);

	public slots:
		void showAll(bool all);
		void selectCurve(int index);

	private slots:
		void showStatistics();
		void customVectorsPage(bool angleMag);
		void updateEndPointColumns(const QString& text);

		void fillBoxSymbols();
		void fillSymbols();
		bool acceptParams();
		void showWorksheet();
		void quit();

		int setPlotType(CurveTreeItem *item);
		void changePlotType(int plotType);
		void setActiveCurve(CurveTreeItem *item);

		void insertTabs(int plot_type);
		void updateTabWindow(QTreeWidgetItem *currentItem, QTreeWidgetItem *previousItem);
		void showAreaColor(bool show);

		void removeSelectedCurve();

		/******* error bars options **************/
		void pickErrorBarsColor();
		void changeErrorBarsType();
		void changeErrorBarsPlus();
		void changeErrorBarsMinus();
		void changeErrorBarsThrough();

		void setAutomaticBinning();
		bool validInput();
		void showPlotAssociations(QTreeWidgetItem *item, int);

		void setPenStyle(Qt::PenStyle style);
		void setPiePenStyle(const Qt::PenStyle& style);

		//box plots
		void setBoxType(int index);
		void setBoxRangeType(int index);
		void setWhiskersRange(int index);

		//spectrograms
		void pickContourLinesColor();
		void showDefaultContourLinesBox(bool show);
		void showColorMapEditor(bool show);

	protected slots:
		void setActiveLayer(LayerItem *item);
		void updateTreeWidgetItem(QTreeWidgetItem *item);
		void updateBackgroundTransparency(int alpha);
		void updateCanvasTransparency(int alpha);
		void updateAntialiasing(bool on);
		void updateBorder(int width);
		void pickBackgroundColor();
		void pickCanvasColor();
		void pickBorderColor();
		void changeMargin(int);
		void setTitlesFont();
		void setAxesLabelsFont();
		void setAxesNumbersFont();
		void setLegendsFont();
		void editCurve();

	private:
		void closeEvent(QCloseEvent* e);

		void clearTabWidget();
		void initAxesPage();
		void initLinePage();
		void initSymbolsPage();
		void initHistogramPage();
		void initErrorsPage();
		void initSpacingPage();
		void initVectPage();
		void initBoxPage();
		void initPercentilePage();
		void initSpectrogramPage();
		void initLayerPage();
		void initFontsPage();
		void initPiePage();
		void initPrintPage();
		void contextMenuEvent(QContextMenuEvent *e);

		QFont titleFont, legendFont, axesFont, numbersFont;

		Graph *m_graph;
		QStringList columnNames;

		QPushButton *btnTitle, *btnAxesLabels, *btnAxesNumbers, *btnLegend;
		ColorMapEditor *colorMapEditor;
		QWidget *curvePlotTypeBox, *layerPage, *piePage, *fontsPage, *printPage;
		QTreeWidget* listBox;
		QCheckBox *boxAntialiasing, *boxAll, *boxScaleLayers, *boxPrintCrops;
		ColorButton *boxBorderColor, *boxBackgroundColor, *boxCanvasColor;
		QSpinBox *boxBackgroundTransparency, *boxCanvasTransparency, *boxBorderWidth, *boxMargin;
		QSpinBox *boxRadius, *boxPieLineWidth;
		ColorBox *boxFirstColor, *boxPieLineColor;
		PatternBox *boxPiePattern;
		QComboBox* boxPieLineStyle;

		QPushButton* buttonApply, *btnWorksheet;
		QPushButton* buttonOk, *btnMore;
		QPushButton* buttonCancel;
		QComboBox* boxPlotType;
		QWidget* linePage;
		QComboBox* boxConnect;
		QComboBox* boxLineStyle;
		QSpinBox* boxLineWidth, *boxPenWidth;
		ColorBox* boxLineColor, *boxAreaColor;
		QWidget* symbolPage;
		QSpinBox* boxSymbolSize;
		ColorBox* boxSymbolColor,*boxFillColor;
		SymbolBox* boxSymbolStyle;
		PatternBox *boxPattern;
		QTabWidget* privateTabWidget;
		QWidget *errorsPage, *spectrogramPage;
		QGroupBox* fillGroupBox;
		QCheckBox* plusBox;
		QCheckBox* minusBox;
		QCheckBox* xBox;
		ColorButton *colorBox, *levelsColorBox;
		ColorBox* vectColorBox;
		QComboBox* widthBox;
		QComboBox* capBox;
		QCheckBox* throughBox;
		QLabel *labelPosition, *labelXEnd, *labelYEnd;
		QGroupBox* GroupBoxH;
		QWidget *histogramPage, *spacingPage;
		QLineEdit *binSizeBox, *histogramBeginBox, *histogramEndBox;
		QCheckBox *automaticBox;
		QPushButton* buttonStatistics, *btnEditCurve;
		QSpinBox* gapBox, *offsetBox, *boxWidth;
		QWidget *vectPage, *boxPage, *percentilePage, *axesPage;
		QComboBox *xEndBox, *yEndBox, *boxType, *boxWhiskersType, *boxWhiskersRange, *boxRange;
		QSpinBox* headAngleBox, *headLengthBox, *vectWidthBox, *boxPercSize, *boxEdgeWidth;
		QCheckBox *filledHeadBox;
		QSpinBox *boxCoef, *boxWhiskersCoef;
		QCheckBox *boxFillSymbols, *boxFillSymbol;
		ColorBox *boxPercFillColor, *boxEdgeColor;
		QLabel 	*whiskerCoeffLabel, *whiskerRangeLabel, *boxCoeffLabel;
		QLabel *boxRangeLabel, *whiskerCntLabel, *boxCntLabel;
		QGroupBox *GroupBoxVectEnd;
		QComboBox *vectPosBox, *boxXAxis, *boxYAxis, *colorScaleBox, *boxContourStyle;
		QSpinBox *levelsBox, *contourWidthBox, *colorScaleWidthBox;
		QGroupBox *levelsGroupBox, *axisScaleBox, *imageGroupBox;
		QGroupBox *defaultPenBox;
		QRadioButton *defaultScaleBox, *grayScaleBox, *customScaleBox, *defaultContourBox, *autoContourBox;

		SymbolBox *boxMaxStyle, *boxMinStyle, *boxMeanStyle, *box99Style, *box1Style;
		QDoubleSpinBox *whiskerCnt, *boxCnt;
};

/*****************************************************************************
 *
 * Class LayerItem
 *
 *****************************************************************************/
//! LayerItem tree widget item class
class LayerItem : public QTreeWidgetItem
{
	public:
		enum {LayerTreeItem = 1001};
		LayerItem(Layer *g, QTreeWidgetItem *parent, const QString& s);

		Layer *layer() { return m_layer; };
		void setActive(bool select);

	protected:
		void insertCurvesList();
		Layer *m_layer;
};

/*****************************************************************************
 *
 * Class CurveTreeItem
 *
 *****************************************************************************/
//! CurveTreeItem tree widget item class
class CurveTreeItem : public QTreeWidgetItem
{
	public:
		enum {PlotCurveTreeItem = 1002};
		CurveTreeItem(const QwtPlotItem *curve, LayerItem *parent, const QString& s);

		Layer* layer(){return ((LayerItem *)parent())->layer();};
		void setActive(bool on);

		const QwtPlotItem *plotItem() { return m_curve; };
		int plotItemType();
		int plotItemIndex();

	protected:
		const QwtPlotItem *m_curve;
};

#endif
