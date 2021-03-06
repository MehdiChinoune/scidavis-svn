/* vim: set filetype=cpp : */
/* due to a limitation of sip, this file has to use Unix style line endings */
/***************************************************************************
	File                 : scidavis.sip
	Project              : SciDAVis
--------------------------------------------------------------------
	Copyright            : (C) 2006 by Knut Franke, Ion Vasilief
	Email                : knut.franke@gmx.de, ion_vasilief@yahoo.fr
	Description          : Specifications for Python bindings

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

%Module scidavis 0
%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class ScriptEdit: QTextEdit
{
%TypeHeaderCode
#include "core/ScriptEdit.h"
%End
public slots:
  void execute();
  void executeAll();
  void evaluate();
  void print();
  void insertFunction(const QString &);
private:
  ScriptEdit(ScriptEdit&);
};

class MyWidget: QWidget /PyName=MDIWindow/
{
%TypeHeaderCode
#include "core/MyWidget.h"
%End
public:
  enum CaptionPolicy{Name = 0, Label = 1, Both = 2};

  SIP_PYOBJECT windowLabel();
%MethodCode
  sipRes = PyString_FromString(sipCpp->windowLabel());
%End
  void setWindowLabel(const QString&);

  CaptionPolicy captionPolicy();
  void setCaptionPolicy(CaptionPolicy);

  virtual SIP_PYOBJECT saveAsTemplate(const QString& );
%MethodCode
  sipRes = PyString_FromString(sipCpp->saveAsTemplate(*a0));
%End
  virtual void restore(const QStringList& );
  Folder* folder();

  void askOnCloseEvent(bool)/PyName=confirmClose/;

private:
  MyWidget(const MyWidget&);
};

typedef QList<MyWidget*> MDIWindowList;

class Table: MyWidget
{
%TypeHeaderCode
#include "table/Table.h"

#define CHECK_TABLE_COL(arg)\
    int col;\
    if (PyInt_Check(arg)) {\
      col = (int)PyInt_AsLong(arg) - 1;\
		if (col < 0 || col >= sipCpp->columnCount()) {\
			sipIsErr = 1;\
			PyErr_Format(PyExc_ValueError, "There's no column %d in table %s!", col+1, sipCpp->name().toAscii().constData());\
		}\
	 } else {\
      PyObject *tmp = PyObject_Str(arg);\
		if (!tmp) {\
			sipIsErr = 1;\
			PyErr_Format(PyExc_TypeError, "Column argument must be either int or string.");\
		} else {\
			col = sipCpp->colNames().indexOf(PyString_AsString(tmp));\
			if (col < 0) {\
				sipIsErr = 1;\
				PyErr_Format(PyExc_ValueError, "There's no column named %s in table %s!", PyString_AsString(tmp),\
						sipCpp->name().toAscii().constData());\
				Py_DECREF(tmp);\
			}\
		}\
	 }
#define CHECK_TABLE_ROW(arg)\
   int row = arg-1;\
	if (row < 0 || row >= sipCpp->rowCount()) {\
		sipIsErr = 1;\
		PyErr_Format(PyExc_ValueError, "There's no row %d in table %s!", row+1, sipCpp->name().toAscii().constData());\
	}

%End

public:
  int rowCount();
  int columnCount();
  void setRowCount(int);
  void setColumnCount(int);

  SIP_PYOBJECT text(SIP_PYOBJECT, int);
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  CHECK_TABLE_ROW(a1);
  if (sipIsErr == 0) {
		PyObject *encstr = PyString_FromString(sipCpp->text(row, col).utf8());
		if (encstr) {
			sipRes = PyUnicode_FromEncodedObject(encstr, "utf8", 0);
			Py_DECREF(encstr);
		} else {
			sipRes = NULL;
			sipIsErr = 1;
		}
	}
%End
  double cell(SIP_PYOBJECT, int);
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  CHECK_TABLE_ROW(a1);
	if (sipIsErr == 0)
		sipRes = sipCpp->cell(row, col);
%End
  void setText(SIP_PYOBJECT, int, const QString&);
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  CHECK_TABLE_ROW(a1);
	if (sipIsErr == 0)
		sipCpp->setText(row, col, *a2);
%End
	//TODO
	/*
  void setCell(SIP_PYOBJECT, int, double);
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  CHECK_TABLE_ROW(a1);
	if (sipIsErr == 0)
		sipCpp->setCell(row, col, a2);
%End
*/
  SIP_PYOBJECT colName(int);
%MethodCode
   if (a0 < 1 || a0 > sipCpp->columnCount()) {\
		sipIsErr = 1;\
		PyErr_SetString(PyExc_ValueError, "Invalid column argument");\
	} else
		sipRes = PyString_FromString(sipCpp->colLabel(a0-1));
%End
  void setColName(SIP_PYOBJECT, const QString&);
%MethodCode
	sipIsErr = 0;
   CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setColName(col, *a1);
%End
  void notifyChanges();

// TODO: implement the following
/*
  void importASCII(const QString&, const QString&="\t", int=0, bool=false, bool=true, bool=false, bool=false);
  bool exportASCII(const QString&, const QString&="\t", bool=false, bool=false);
  void normalizeCol(SIP_PYOBJECT) /PyName=normalize/;
%MethodCode
	sipIsErr = 0;
	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->normalizeCol(col);
%End
  void normalize();

  void sortColumn(SIP_PYOBJECT, int order = 0);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->sortColumn(col, a1);
%End
  void sort(int type = 0, int order = 0, const QString& leadCol = QString());
  void sortColumns(SIP_PYTUPLE, int=0, int=0, const QString&=QString());
%MethodCode
  QStringList l;
  int n = PyTuple_Size(a0);
  for (int i=0; i<n; i++) {
    PyObject *str = PyObject_Str(PyTuple_GET_ITEM(a0,i));
    if (str) {
      l << PyString_AsString(str);
		Py_DECREF(str);
	 } else {
      sipIsErr = 1;
      break;
    }
  }
  sipCpp->sortColumns(l, a1, a2, *a3);
%End
*/

	void setCommand(SIP_PYOBJECT, const QString&);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setCommand(col, *a1);
%End

	void setComment(SIP_PYOBJECT, const QString&);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setColComment(col, *a1);
%End

private:
  Table(const Table&);
};

class Matrix: MyWidget
{
%TypeHeaderCode
#include "matrix/Matrix.h"
#define CHECK_MATRIX_COL(arg)\
	int col = arg-1;\
   if (col < 0 || col >= sipCpp->columnCount()) {\
		sipIsErr = 1;\
		PyErr_Format(PyExc_ValueError, "There's no column %d in matrix %s!", col+1, sipCpp->name().toAscii().constData());\
	}
#define CHECK_MATRIX_ROW(arg)\
   int row = arg-1;\
	if (row < 0 || row >= sipCpp->rowCount()) {\
		sipIsErr = 1;\
		PyErr_Format(PyExc_ValueError, "There's no row %d in matrix %s!", row+1, sipCpp->name().toAscii().constData());\
	}
%End
public:
  int rowCount();
  void setRowCount(int);
  int columnCount();
  void setColumnCount(int);
  void setDimensions(int rows, int cols);

  SIP_PYOBJECT text(int, int);
%MethodCode
	sipIsErr = 0;
	CHECK_MATRIX_ROW(a0);
	CHECK_MATRIX_COL(a1);
	if (sipIsErr == 0)
		sipRes = PyString_FromString(sipCpp->text(row, col));
%End
  double cell(int, int);
%MethodCode
	sipIsErr = 0;
	CHECK_MATRIX_ROW(a0);
	CHECK_MATRIX_COL(a1);
	if (sipIsErr == 0)
		sipRes = sipCpp->cell(row, col);
%End
  void setText(int, int, const QString&);
%MethodCode
	sipIsErr = 0;
	CHECK_MATRIX_ROW(a0);
	CHECK_MATRIX_COL(a1);
	if (sipIsErr == 0)
		sipCpp->setText(row, col, *a2);
%End
  void setCell(int, int, double);
%MethodCode
	sipIsErr = 0;
	CHECK_MATRIX_ROW(a0);
	CHECK_MATRIX_COL(a1);
	if (sipIsErr == 0)
		sipCpp->setCell(row, col, a2);
%End

    double xStart();
	double xEnd();
	double yStart();
	double yEnd();
    void setCoordinates(double xs, double xe, double ys, double ye);

    void setFormula(const QString &s);
    bool calculate(int startRow = 0, int endRow = -1, int startCol = 0, int endCol = -1);

    void setNumericPrecision(int prec);

    void transpose();
	void invert();
	double determinant();

private:
  Matrix(const Matrix&);
};

class LineEnrichment /PyName=ArrowMarker/ // : QwtPlotMarker
{
%TypeHeaderCode
#include "graph/enrichments/LineEnrichment.h"
%End
public:

  LineEnrichment();

  void setStartPoint(double, double) /PyName=setStart/;
  void setEndPoint(double, double) /PyName=setEnd/;

  void setStyle(Qt::PenStyle);
  void setColor(const QColor&);
  void setWidth(int);
  void drawStartArrow(bool = true);
  void drawEndArrow(bool = true);
  void setHeadLength(int);
  void setHeadAngle(int);
  void fillArrowHead(bool = true);

private:
  LineEnrichment(const LineEnrichment&);
};

class ImageEnrichment /PyName=ImageMarker/ // : QwtPlotMarker
{
%TypeHeaderCode
#include "graph/enrichments/ImageEnrichment.h"
%End
public:

  ImageEnrichment(const QString&);
  QString fileName();
  
  QSize size();
  void setSize(int, int);
  
  void setBoundingRect(double, double, double, double) /PyName=setCoordinates/;
  
private:
  ImageEnrichment(const ImageEnrichment&);
};

class TextEnrichment /PyName=Legend/ // : QwtPlotMarker
{
%TypeHeaderCode
#include "graph/TextEnrichment.h"
%End
public:

  void setText(const QString&);
  void setTextColor(const QColor&);
  void setFrameStyle(int);
  void setBackgroundColor(const QColor&);
  void setFont(const QFont&);
  void setOriginCoord(double, double);

private:
  TextEnrichment(const TextEnrichment&);
};

class Layer : QWidget
{
%TypeHeaderCode
#include "graph/Layer.h"
#include "graph/TextEnrichment.h"
%End
public:
  bool isPiePlot();
  SIP_PYOBJECT pieLegendText() /PyName=pieLegend/;
%MethodCode
  sipRes = PyString_FromString(sipCpp->pieLegendText());
%End

  bool insertCurve(Table*, const QString&, int=1);
  bool insertCurve(Table*, const QString&, const QString&, int=1);
  void removeCurve(int);
  void removeCurve(const QString&);
  void deleteFitCurves();
  int curveCount() /PyName=numCurves/;

  void addErrorBars(const QString&, Table *, const QString&,
		  int type = 1, int width = 1, int cap = 8, const QColor& color = QColor(Qt::black),
		  bool through = true, bool minus = true, bool plus = true);

  void addArrow(LineEnrichment*);

  ImageEnrichment* addImage(ImageEnrichment*);
  ImageEnrichment* addImage(const QString& fileName);
  
  void setTitle(const QString& t);
  TextEnrichment* newLegend();
  TextEnrichment* newLegend(const QString&);
  void setLegend(const QString&) /NoDerived/;
%MethodCode
  sipCpp->legend()->setText(*a0);
%End

  TextEnrichment* legend();

  void removeLegend();
  void addTimeStamp();

  void enableAxis(int axis, bool on = true);
  void setXAxisTitle(const QString& text) /PyName=setXTitle/;
  void setYAxisTitle(const QString& text) /PyName=setYTitle/;

  void setLabelsNumericFormat(int axis, int format, int = 6, const QString& = QString()) /PyName=setAxisNumericFormat/;
  void setScale(int axis, double start, double end, double step = 0.0,
                int majorTicks = 5, int minorTicks = 5, int type = 0, bool inverted = false);

  void setMargin(int);
  void setFrame(int width = 1, const QColor& color = QColor(Qt::black));
  void setBackgroundColor(const QColor& color);
  void setCanvasBackground(const QColor& color) /PyName=setCanvasColor/;

  void showGrid(int);
  void showGrid();
  void replot();
  void print();
  void exportImage(const QString& fileName, int quality = 100, bool transparent = false);
  void exportVector(const QString& fileName, int res = 0, bool color = true,
                    bool keepAspect = true, QPrinter::PageSize pageSize = QPrinter::Custom);
  void exportToFile(const QString& fileName) /PyName=export/;

  void enableAutoscaling(bool = true);
  void setIgnoreResizeEvents(bool = true)/PyName=setIgnoreResize/;
  void setAutoscaleFonts(bool = true);
  void setAntialiasing(bool on = true, bool update = true);

private:
  Layer(const Layer&);
};

class Graph : MyWidget
{
%TypeHeaderCode
#include "graph/Graph.h"
%End
public:
  Layer *activeLayer();
  void setActiveLayer(Layer*);
  int layerCount() /PyName=numLayers/;
  Layer* layer(int num);
  Layer* addLayer(int = 0, int = 0, int = 0, int = 0);

  void setCols(int);
  void setRows(int);
  void setSpacing (int, int);
  void setMargins (int, int, int, int);
  void setLayerCanvasSize (int, int);
  void setAlignement (int, int);
  void arrangeLayers(bool fit = true, bool userSize = false);

  void exportToFile(const QString& fileName) /PyName=export/;
  void exportImage(const QString& fileName, int quality = 100, bool transparent = false);
  void exportVector(const QString& fileName, int res = 0, bool color = true,
                    bool keepAspect = true, QPrinter::PageSize pageSize = QPrinter::Custom);

  void print();
private:
  Graph(const Graph&);
};

class Note: MyWidget
{
%TypeHeaderCode
#include "note/Note.h"
%End
public:
  void execute() const;
  bool autoexec() const;
private:
  Note(const Note&);
};

class ApplicationWindow: QMainWindow
{
%TypeHeaderCode
#include "core/ApplicationWindow.h"
%End
%ConvertToSubClassCode
// we have to do this to override casting in qt/qobject.sip (PyQt 3.16)
sipClass=NULL;
QString cN = sipCpp->className();
sipTypeDef *t;
  for (int i=0; i<sipModuleAPI_scidavis.em_nrtypes; i++)
  if (((t=sipModuleAPI_scidavis.em_types[i]->type)->td_cname && cN == t->td_cname) ||
      (!t->td_cname && cN == t->td_name+4))
    sipClass = sipModuleAPI_scidavis.em_types[i];
%End

public:
  Table* table(const QString&);
%MethodCode
  sipRes = sipCpp->current_folder->table(*a0, false);
  if(!sipRes)
    sipRes = sipCpp->projectFolder()->table(*a0, true);
%End
  Table* newTable();
  Table* newTable(const QString&, int=2, int=30);
%MethodCode
  sipRes = sipCpp->newTable(*a0, a2, a1);
%End
  Matrix* matrix(const QString&);
%MethodCode
  sipRes = sipCpp->current_folder->matrix(*a0, false);
  if(!sipRes)
    sipRes = sipCpp->projectFolder()->matrix(*a0, true);
%End
  Matrix* newMatrix();
  Matrix* newMatrix(const QString&, int=32, int=32);
  Graph *plot(const QString&) /PyName=graph/;
%MethodCode
  sipRes = sipCpp->current_folder->graph(*a0, false);
  if(!sipRes)
    sipRes = sipCpp->projectFolder()->graph(*a0, true);
%End
  Graph* newGraph();
  Note *note(const QString&);
%MethodCode
  sipRes = sipCpp->current_folder->note(*a0, false);
  if(!sipRes)
    sipRes = sipCpp->projectFolder()->note(*a0, true);
%End
  Note* newNote(const QString& = QString::null);
  Graph *multilayerPlot(Table*, SIP_PYTUPLE, int=1) /PyName=plot/;
%MethodCode
  QStringList l;
  int n = PyTuple_Size(a1);
  for (int i=0; i<n; i++) {
    PyObject *str = PyObject_Str(PyTuple_GET_ITEM(a1,i));
    if (str) {
      l << PyString_AsString(str);
		Py_DECREF(str);
	 } else {
      sipIsErr = 1;
      break;
    }
  }
  sipRes = sipCpp->multilayerPlot(a0, l, a2);
%End
  Graph *multiLayerPlot(Table*, const QString&, int=1) /PyName=plot/;
%MethodCode
  QStringList l;
  l << *a1;
  sipRes = sipCpp->multilayerPlot(a0, l, a2);
%End

	Matrix* importImage();
	Matrix* importImage(const QString&);
	Graph* plotContour(Matrix*);
	Graph* plotColorMap(Matrix*);
	Graph* plotGrayScale(Matrix*);

  QWidgetList *windowsList() /PyName=windows/;

  QTextEdit *results;
//  QTextEdit *console;

  // folders
  Folder *activeFolder() /NoDerived/;
%MethodCode
  sipRes = sipCpp->current_folder;
%End
  void saveFolder(Folder *folder, const QString& fn);
  Folder* projectFolder() /PyName=rootFolder/;

  void renameWindow(MyWidget *, const QString &);
  void setPreferences(Layer*);

  MyWidget* clone(MyWidget*);

private:
  ApplicationWindow(const ApplicationWindow&);
};

class Fit : QObject
{
%TypeHeaderCode
#include "core/Fit.h"
%End
public:
  enum Algorithm{ScaledLevenbergMarquardt, UnscaledLevenbergMarquardt, NelderMeadSimplex};
  enum WeightingMethod{NoWeighting, Instrumental, Statistical, Dataset};

  Fit(ApplicationWindow* /TransferThis/, Layer*=0, const char*=0);
  ~Fit();

  virtual void fit();

  bool setWeightingData(WeightingMethod, const QString&=QString::null);

  bool setDataFromCurve(const QString&, Layer*=0);
  bool setDataFromCurve(const QString&, double, double, Layer*=0);

  void setInterval(double from, double to);

  QString formula();
  int numParameters();

  void setInitialGuess(int, double) /PyName=setInitialValue/;
  void setInitialGuesses(...) /PyName=setInitialValues/;
%MethodCode
int n = PyTuple_GET_SIZE(a0);
double *values = new double[n];
for (int i=0; i<n; i++) {
  PyObject *item = PyTuple_GET_ITEM(a0, i);
  if (PyNumber_Check(item)) {
    item=PyNumber_Float(item);
    if (!item) {
      sipIsErr=1;
      break;
    }
    values[i] = PyFloat_AS_DOUBLE(item);
    Py_DECREF(item);
  } else
    values[i] = 0;
}
sipCpp->setInitialGuesses(values);
delete values;
%End

  virtual void guessInitialValues();

  void setAlgorithm(Algorithm);

  void setTolerance(double);

  void setColor(int);
  void setColor(const QString&);

  void setOutputPrecision(int);

  void generateFunction(bool, int=100);

  void setMaximumIterations(int);

  void showLegend();

  virtual QString legendInfo();

  void scaleErrors(bool yes = true);

  SIP_PYTUPLE results();
%MethodCode
double *results = sipCpp->results();
int size=sipCpp->numParameters();
sipRes = PyTuple_New(size);
if(sipRes)
{
  for(int i=0; i<size; i++)
    PyTuple_SET_ITEM(sipRes, i, PyFloat_FromDouble(results[i]));
} else
  sipIsErr = 1;
%End

  SIP_PYTUPLE errors();
%MethodCode
double *errors = sipCpp->errors();
int size=sipCpp->numParameters();
sipRes = PyTuple_New(size);
if(sipRes)
{
  for(int i=0; i<size; i++)
    PyTuple_SET_ITEM(sipRes, i, PyFloat_FromDouble(errors[i]));
} else
  sipIsErr = 1;
%End

  //! Returns the sum of squares of the residuals from the best-fit line
  double chiSquare();

  Table* parametersTable(const QString&);
  Matrix* covarianceMatrix(const QString&);
};

%ModuleCode
ApplicationWindow *sipscidavis_app()
{
  int iserr = 0;
  PyObject *me = PyImport_ImportModule("scidavis");
  PyObject *mydict = PyModule_GetDict(me);
  PyObject *pyapp = PyDict_GetItemString(mydict,"app");
  Py_DECREF(me);
  if (sipCanConvertToInstance(pyapp, sipClass_ApplicationWindow, SIP_NOT_NONE))
    return (ApplicationWindow*) sipConvertToInstance(pyapp, sipClass_ApplicationWindow, NULL, SIP_NOT_NONE, NULL, &iserr);
  else
    return NULL;
}
%End
%ModuleHeaderCode
class ApplicationWindow;
ApplicationWindow *sipscidavis_app();
#define SIPSCIDAVIS_APP(sipcppexpr)\
ApplicationWindow *app = sipscidavis_app();\
if (app) sipCpp = sipcppexpr;\
else { sipCpp = NULL; }
%End

class ExponentialFit : Fit
{
%TypeHeaderCode
#include "analysis/ExponentialFit.h"
%End
public:
  ExponentialFit(ApplicationWindow * /TransferThis/, Layer *, const QString&, bool=false);
  ExponentialFit(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double, bool=false);
  ExponentialFit(Layer *, const QString&, bool=false) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipExponentialFit(app, a0, *a1, a2))
%End
  ExponentialFit(Layer *, const QString&, double, double, bool=false) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipExponentialFit(app, a0, *a1, a2, a3, a4))
%End
};

class TwoExpFit : Fit
{
%TypeHeaderCode
#include "analysis/ExponentialFit.h"
%End
public:
  TwoExpFit(ApplicationWindow * /TransferThis/, Layer *, const QString&);
  TwoExpFit(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double);
  TwoExpFit(Layer *, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipTwoExpFit(app, a0, *a1))
%End
  TwoExpFit(Layer *, const QString&, double, double) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipTwoExpFit(app, a0, *a1, a2, a3))
%End
};

class ThreeExpFit : Fit
{
%TypeHeaderCode
#include "analysis/ExponentialFit.h"
%End
public:
  ThreeExpFit(ApplicationWindow * /TransferThis/, Layer *, const QString&);
  ThreeExpFit(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double);
  ThreeExpFit(Layer *, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipThreeExpFit(app, a0, *a1))
%End
  ThreeExpFit(Layer *, const QString&, double, double) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipThreeExpFit(app, a0, *a1, a2, a3))
%End
};

class SigmoidalFit : Fit
{
%TypeHeaderCode
#include "analysis/SigmoidalFit.h"
%End
public:
  SigmoidalFit(ApplicationWindow * /TransferThis/, Layer *, const QString&);
  SigmoidalFit(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double);
  SigmoidalFit(Layer *, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipSigmoidalFit(app, a0, *a1))
%End
  SigmoidalFit(Layer *, const QString&, double, double) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipSigmoidalFit(app, a0, *a1, a2, a3))
%End
};

class GaussAmpFit : Fit
{
%TypeHeaderCode
#include "analysis/MultiPeakFit.h"
%End
public:
  GaussAmpFit(ApplicationWindow * /TransferThis/, Layer *, const QString&);
  GaussAmpFit(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double);
  GaussAmpFit(Layer *, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipGaussAmpFit(app, a0, *a1))
%End
  GaussAmpFit(Layer *, const QString&, double, double) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipGaussAmpFit(app, a0, *a1, a2, a3))
%End
};

class UserFunctionFit : Fit /PyName=NonLinearFit/
{
%TypeHeaderCode
#include "analysis/UserFunctionFit.h"
%End
public:
  UserFunctionFit(ApplicationWindow * /TransferThis/, Layer *, const QString&);
  UserFunctionFit(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double);
  UserFunctionFit(Layer *, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipUserFunctionFit(app, a0, *a1))
%End
  UserFunctionFit(Layer *, const QString&, double, double) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipUserFunctionFit(app, a0, *a1, a2, a3))
%End
  void setParametersList(...) /PyName=setParameters/;
%MethodCode
  QStringList l;
  char *item;
  for (int i=0; i<PyTuple_GET_SIZE(a0); i++)
    if (item = PyString_AsString(PyTuple_GET_ITEM(a0, i)))
      l << item;
    else
      sipIsErr = 1;
  sipCpp->setParametersList(l);
%End
  // TODO: make it accept Python callables
  void setFormula(const QString&);
};

class PluginFit : Fit
{
%TypeHeaderCode
#include "analysis/PluginFit.h"
%End
public:
  PluginFit(ApplicationWindow * /TransferThis/, Layer *, const QString&);
  PluginFit(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double);
  PluginFit(Layer *, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipPluginFit(app, a0, *a1))
%End
  PluginFit(Layer *, const QString&, double, double) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipPluginFit(app, a0, *a1, a2, a3))
%End
  bool load(const QString&);
};

class MultiPeakFit : Fit
{
%TypeHeaderCode
#include "analysis/MultiPeakFit.h"
%End
public:
  enum PeakProfile{Gauss, Lorentz};
  MultiPeakFit(ApplicationWindow * /TransferThis/, Layer *, PeakProfile=Gauss, int=1);

  int peaks() /PyName=numPeaks/;
  void setNumPeaks(int);

  void enablePeakCurves(bool);
  void setPeakCurvesColor(int);

  static QString generateFormula(int, PeakProfile);
  static QStringList generateParameterList(int);
};

class LorentzFit : MultiPeakFit
{
%TypeHeaderCode
#include "analysis/MultiPeakFit.h"
%End
public:
  LorentzFit(ApplicationWindow * /TransferThis/, Layer *, const QString&);
  LorentzFit(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double);
  LorentzFit(Layer *, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipLorentzFit(app, a0, *a1))
%End
  LorentzFit(Layer *, const QString&, int, int) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipLorentzFit(app, a0, *a1, a2, a3))
%End
};

class GaussFit : MultiPeakFit
{
%TypeHeaderCode
#include "analysis/MultiPeakFit.h"
%End
public:
  GaussFit(ApplicationWindow * /TransferThis/, Layer *, const QString&);
  GaussFit(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double);
  GaussFit(Layer *, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipGaussFit(app, a0, *a1))
%End
  GaussFit(Layer *, const QString&, double, double) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipGaussFit(app, a0, *a1, a2, a3))
%End
};

class PolynomialFit : Fit
{
%TypeHeaderCode
#include "analysis/PolynomialFit.h"
%End
public:
  PolynomialFit(ApplicationWindow * /TransferThis/, Layer *, QString&, int=2, bool=false);
  PolynomialFit(ApplicationWindow * /TransferThis/, Layer *, QString&, int, int, int=2, bool=false);
  PolynomialFit(Layer *, QString&, int=2, bool=false) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipPolynomialFit(app, a0, *a1, a2, a3))
%End
  PolynomialFit(Layer *, QString&, int, int, int=2, bool=false) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipPolynomialFit(app, a0, *a1, a2, a3, a4, a5))
%End

  virtual QString legendInfo();
  void fit();

  static QString generateFormula(int);
  static QStringList generateParameterList(int);
};

class LinearFit : Fit
{
%TypeHeaderCode
#include "analysis/PolynomialFit.h"
%End
public:
  LinearFit(ApplicationWindow * /TransferThis/, Layer *, const QString&);
  LinearFit(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double);
  LinearFit(Layer *, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipLinearFit(app, a0, *a1))
%End
  LinearFit(Layer *, const QString&, double, double) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipLinearFit(app, a0, *a1, a2, a3))
%End
  void fit();
};

class Filter : QObject
{
%TypeHeaderCode
#include "core/Filter.h"
%End
public:
  Filter(ApplicationWindow* /TransferThis/, Layer*=0, const char*=0);
  ~Filter();

  void setOutputPoints(int);

  void setColor(int);
  void setColor(const QString&);

  virtual bool run();
};

class Differentiation : Filter
{
%TypeHeaderCode
#include "analysis/Differentiation.h"
%End
public:
  Differentiation(ApplicationWindow * /TransferThis/, Layer *, const QString&);
  Differentiation(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double);
  Differentiation(Layer *, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipDifferentiation(app, a0, *a1))
%End
  Differentiation(Layer *, const QString&, double, double) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipDifferentiation(app, a0, *a1, a2, a3))
%End
  bool run();
};

class Integration : Filter
{
%TypeHeaderCode
#include "analysis/Integration.h"
%End
public:
  Integration(ApplicationWindow * /TransferThis/, Layer *, const QString&);
  Integration(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double);
  Integration(Layer *, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipIntegration(app, a0, *a1))
%End
  Integration(Layer *, const QString&, double, double) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipIntegration(app, a0, *a1, a2, a3))
%End

  void setMethodOrder(int n);
  bool run();
};

class Interpolation : Filter
{
%TypeHeaderCode
#include "analysis/Interpolation.h"
%End
public:
  enum InterpolationMethod{Linear, Cubic, Akima};

  Interpolation(ApplicationWindow * /TransferThis/, Layer *, const QString&, int=0);
  Interpolation(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double, int=0);
  Interpolation(Layer *, const QString&, int=0) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipInterpolation(app, a0, *a1, a2))
%End
  Interpolation(Layer *, const QString&, double, double, int=0) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipInterpolation(app, a0, *a1, a2, a3, a4))
%End

  void setMethod(int n);
  void setMethod(InterpolationMethod m);
  bool run();
};

class SmoothFilter : Filter
{
%TypeHeaderCode
#include "analysis/SmoothFilter.h"
%End
public:
  enum SmoothMethod{SavitzkyGolay = 1, FFT = 2, Average = 3};

  SmoothFilter(ApplicationWindow * /TransferThis/, Layer *, const QString&, int=3);
  SmoothFilter(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double, int=3);
  SmoothFilter(Layer *, const QString&, int=3) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipSmoothFilter(app, a0, *a1, a2))
%End
  SmoothFilter(Layer *, const QString&, double, double, int=3) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipSmoothFilter(app, a0, *a1, a2, a3, a4))
%End

  void setMethod(int);
  void setMethod(SmoothMethod);

  void setSmoothPoints(int, int = 0);
  void setPolynomOrder(int);

  bool run();
};

class FFTFilter : Filter
{
%TypeHeaderCode
#include "analysis/FFTFilter.h"
%End
public:
  enum FilterType{LowPass = 1, HighPass = 2, BandPass = 3, BandBlock = 4};

  FFTFilter(ApplicationWindow * /TransferThis/, Layer *, const QString&, int=1);
  FFTFilter(ApplicationWindow * /TransferThis/, Layer *, const QString&, double, double, int=1);
  FFTFilter(Layer *, const QString&, int=1) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipFFTFilter(app, a0, *a1, a2))
%End
  FFTFilter(Layer *, const QString&, double, double, int=1) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipFFTFilter(app, a0, *a1, a2, a3, a4))
%End

  void setFilterType(int);
  void setFilterType(FilterType);

  void setCutoff(double);
  void setBand(double, double);
  void enableOffset(bool=true);

  bool run();
};

class FFT : Filter
{
%TypeHeaderCode
#include "analysis/FFT.h"
%End
public:

  FFT(ApplicationWindow * /TransferThis/, Table *, const QString&, const QString& = QString());
  FFT(ApplicationWindow * /TransferThis/, Layer *, const QString&);
  FFT(Table *, const QString&, const QString& = QString()) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipFFT(app, a0, *a1, *a2))
%End
  FFT(Layer *, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipFFT(app, a0, *a1))
%End

  void setInverseFFT(bool=true);
  void setSampling(double);
  void normalizeAmplitudes(bool=true);
  void shiftFrequencies(bool=true);

  bool run();
};

class Correlation : Filter
{
%TypeHeaderCode
#include "analysis/Correlation.h"
%End
public:

  Correlation(ApplicationWindow * /TransferThis/, Table *, const QString&, const QString&);
  Correlation(Table *, const QString&, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipCorrelation(app, a0, *a1, *a2))
%End

  void setDataFromTable(Table *, const QString&, const QString&);
  bool run();
};

class Convolution : Filter
{
%TypeHeaderCode
#include "analysis/Convolution.h"
%End
public:

  Convolution(ApplicationWindow * /TransferThis/, Table *, const QString&, const QString&);
  Convolution(Table *, const QString&, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipConvolution(app, a0, *a1, *a2))
%End

  void setDataFromTable(Table *, const QString&, const QString&);
  bool run();
};

class Deconvolution : Filter
{
%TypeHeaderCode
#include "analysis/Convolution.h"
%End
public:

  Deconvolution(ApplicationWindow * /TransferThis/, Table *, const QString&, const QString&);
  Deconvolution(Table *, const QString&, const QString&) /NoDerived/;
%MethodCode
  SIPSCIDAVIS_APP(new sipDeconvolution(app, a0, *a1, *a2))
%End

  void setDataFromTable(Table *, const QString&, const QString&);
  bool run();
};

// used for output redirection
class PythonScriptingEngine
{
%TypeHeaderCode
#include "python/PythonScriptingEngine.h"
%End
public:
  void write(const QString&);
private:
  PythonScriptingEngine(const PythonScriptingEngine&);
};
class PythonScript
{
%TypeHeaderCode
#include "python/PythonScript.h"
%End
public:
  void write(const QString&);
private:
  PythonScript(const PythonScript&);
};

class Folder : QObject
{
%TypeHeaderCode
#include "core/Folder.h"
%End
public:
  MDIWindowList windowsList() /PyName=windows/;
// TODO: implement signal Folder::nameChanged and make it update the project explorer; adjust renaming from GUI accordingly
//  void setFolderName(const QString&) /PyName=setName/;
  QString name();
  QString path();

  QList<Folder*> folders();
  Folder *findSubfolder(const QString&, bool=true, bool=false)  /PyName=folder/;
  MyWidget* findWindow(const QString&, bool=true, bool=true, bool=false, bool=true);

  MyWidget *window(const QString &name, const char *cls="MyWidget", bool recursive=false);
  Table *table(const QString &name, bool recursive=false);
  Matrix *matrix(const QString &name, bool recursive=false);
  Graph *graph(const QString &name, bool recursive=false);
  Note *note(const QString &name, bool recursive=false);

  Folder* rootFolder();

private:
  Folder(const Folder&);
};
