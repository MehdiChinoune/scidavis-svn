/***************************************************************************
    File                 : Graph3D.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : 3D graph widget

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
#ifndef GRAPH3D_H
#define GRAPH3D_H

#include <qwt3d_surfaceplot.h>
#include <qwt3d_function.h>

#include <QVector>
#include <QTimer>
#include <QWidget>
#include "core/AbstractPart.h"

class Table;
class Matrix;
class UserFunction;
class ActionManager;

class QEvent;
class QVBoxLayout;

using namespace Qwt3D;

/*!\brief 3D graph widget.
 *
 * This provides 3D plotting using Qwt3D.
 *
 * Current status: This class is beeing ported to the aspect framework while keeping most of the old
 * architecture. The aim is to get a working version of SciDAVis with the new Table and Matrix asap. 
 * At the moment, this class incorporates all 5 layers of the aspect framework. Many things are temporary
 * hacks. All major changes to this class are postponed past the next major release.
 *
 * \section future_plans Future Plans
 * If Graph is extended to accept any QWidget, Graph3D wouldn't have to inherit from MyWidget any more.
 * It could also make sense to unify the interface with other plot types; see documentation of Graph.
 * Big problem here: export to vector formats. Qwt3D's export filters write directly to a file, so they
 * can't be combined with output generated via QPrinter.
 */
class Graph3D: public AbstractPart
{
	Q_OBJECT

	public:
		Graph3D (const QString & name);
		~Graph3D();

		enum PlotType{Scatter=0, Trajectory = 1, Bars = 2};
		enum PointStyle{None=0, Dots=1, VerticalBars=2, HairCross=3, Cones=4};

		Qwt3D::SurfacePlot* sp;
		UserFunction *func;

		//! Construct a primary view on me.
		/**
		 * The caller recieves ownership of the view.
		 *
		 * This method may be called multiple times during the life time of a Part, or it might not get
		 * called at all. Parts must not depend on the existence of a view for their operation.
		 */
		virtual QWidget * view() const;
		//! Return a new context menu.
		/**
		 * The caller takes ownership of the menu.
		 */
		virtual QMenu *createContextMenu();
		//! Fill the part specific menu for the main window including setting the title
		/**
		 * \return true on success, otherwise false (e.g. part has no actions).
		 */
		virtual bool fillProjectMenu(QMenu * menu);
		//! Return an icon to be used for decorating my views.
		virtual QIcon icon() const;

	public slots:
		void copy(Graph3D* g);
		void initPlot();
		void initCoord();
		void addFunction(const QString& s,double xl,double xr,double yl,
				double yr,double zl,double zr);
		void insertFunction(const QString& s,double xl,double xr,double yl,
				double yr,double zl,double zr);
		void insertNewData(Table* table, const QString& colName);

		Matrix * matrix(){return m_matrix;};
		void addMatrixData(Matrix* m);//used to plot matrixes
		void addMatrixData(Matrix* m,double xl,double xr,double yl,double yr,double zl,double zr);
		void updateMatrixData(Matrix* m);

		void addData(Table* table, const QString& colName);
		/*!
		 * used when creating a ribbon plot from the plot wizard
		 */
		void addData(Table* table, int xcol, int ycol);
		void addData(Table* table,const QString& xColName,const QString& yColName);
		void addData(Table* table,const QString& xColName,const QString& yColName,
				double xl, double xr, double yl, double yr, double zl, double zr);
		void addData(Table* table, int xCol,int yCol,int zCol, int type);
		void addData(Table* table, int xCol,int yCol,int zCol,
				double xl, double xr, double yl, double yr, double zl, double zr);

		void clearData();
		bool hasData(){return sp->hasData();};

		void updateData(Table* table);
		void updateDataXY(Table* table, int xCol, int yCol);
		void updateDataXYZ(Table* table, int xCol, int yCol, int zCol);

		void changeMatrix(Matrix* m);
		void changeDataColumn(Table* table, const QString& colName);

		//! \name User Functions
		//@{
		UserFunction* userFunction();
		QString formula();
		//@}

		//! \name Event Handlers
		//@{
		bool eventFilter(QObject *object, QEvent *e);
		void resizeEvent ( QResizeEvent *);
		void contextMenuEvent(QContextMenuEvent *e);
		void scaleFonts(double factor);
		void setIgnoreFonts(bool ok){ignoreFonts = ok;};
		//@}

		//! \name Axes
		//@{
		void setFramed();
		void setBoxed();
		void setNoAxes();
		bool isOrthogonal(){return sp->ortho();};
		void setOrtho(bool on = true){sp->setOrtho(on);};

		QStringList axesLabels(){return labels;};
		void updateLabel(int axis,const QString& label, const QFont& f);
		void setAxesLabels(const QStringList& lst);
		void resetAxesLabels();

		QFont xAxisLabelFont();
		QFont yAxisLabelFont();
		QFont zAxisLabelFont();

		void setXAxisLabelFont(const QFont& fnt);
		void setYAxisLabelFont(const QFont& fnt);
		void setZAxisLabelFont(const QFont& fnt);

		void setXAxisLabelFont(const QStringList& lst);
		void setYAxisLabelFont(const QStringList& lst);
		void setZAxisLabelFont(const QStringList& lst);

		QFont numbersFont();
		void setNumbersFont(const QFont& font);
		void setNumbersFont(const QStringList& lst);

		double xStart();
		double xStop();
		double yStart();
		double yStop();
		double zStart();
		double zStop();
		QStringList scaleLimits();
		void updateScale(int axis,const QStringList& options);
		void updateScales(double xl, double xr, double yl, double yr, double zl, double zr);
		void updateScales(double xl, double xr, double yl, double yr,
				double zl, double zr, int xcol, int ycol);
		void updateScales(double xl, double xr, double yl, double yr,
				double zl, double zr, int xCol, int yCol, int zCol);
		void updateScalesFromMatrix(double xl,double xr,double yl,double yr,double zl,double zr);

		QStringList scaleTicks();
		void setTicks(const QStringList& options);

		void updateTickLength(int, double majorLength, double minorLength);
		void adjustLabels(int val);
		int labelsDistance(){return labelsDist;};

		QStringList axisTickLengths();
		void setTickLengths(const QStringList& lst);
		//@}

		//! \name Mesh
		//@{
		void setNoGrid();
		void setHiddenLineGrid();
		void setLineGrid();
		void setFilledMesh();
		void setPointsMesh();
		void setBarsPlot();
		void setFloorData();
		void setFloorIsolines();
		void setEmptyFloor();

		void setMeshLineWidth(int lw);
		double meshLineWidth(){return sp->meshLineWidth();};
		//@}

		//! \name Grid
		//@{
		int grids();
		void setGrid(Qwt3D::SIDE s, bool b);
		void setGrid(int grids);

		void setLeftGrid(bool b);
		void setRightGrid(bool b);
		void setCeilGrid(bool b);
		void setFloorGrid(bool b);
		void setFrontGrid(bool b);
		void setBackGrid(bool b);
		//@}

		void setStyle(Qwt3D::COORDSTYLE coord,Qwt3D::FLOORSTYLE floor,
				Qwt3D::PLOTSTYLE plot, Graph3D::PointStyle point);
		void setStyle(const QStringList& st);
		void customPlotStyle(int style);
		void resetNonEmptyStyle();

		void setRotation(double  xVal,double  yVal,double  zVal);
		void setScale(double  xVal,double  yVal,double  zVal);
		void setShift(double  xVal,double  yVal,double  zVal);
		void updateScaling(double  xVal,double  yVal,double  zVal);

		double xRotation(){return sp->xRotation();};
		double yRotation(){return sp->yRotation();};
		double zRotation(){return sp->zRotation();};

		double xScale(){return sp->xScale();};
		double yScale(){return sp->yScale();};
		double zScale(){return sp->zScale();};

		double xShift(){return sp->xShift();};
		double yShift(){return sp->yShift();};
		double zShift(){return sp->zShift();};

		double zoom(){return sp->zoom();};
		void setZoom(double  val);
		void updateZoom(double  val);

		Qwt3D::PLOTSTYLE plotStyle();
		Qwt3D::FLOORSTYLE floorStyle();
		Qwt3D::COORDSTYLE coordStyle();

		void print();
		void copyImage();
		void exportImage(const QString& fileName, int quality = 100, bool transparent = false);

		void exportPDF(const QString& fileName);
		void exportVector(const QString& fileName, const QString& fileType = "pdf");

		QString saveToString(const QString& geometry);
		QString saveAsTemplate(const QString& geometryInfo);

		void zoomChanged(double);
		void rotationChanged(double, double, double);
		void scaleChanged(double, double, double);
		void shiftChanged(double, double, double);

		//! \name Colors
		//@{
		void setDataColors(const QColor& cMax, const QColor& cMin);

		void updateColors(const QColor& meshColor,const QColor& axesColor,const QColor& numColor,
				const QColor& labelColor,const QColor& bgColor,const QColor& gridColor);
		void changeTransparency(double t);
		void setTransparency(double t);
		double transparency(){return alpha;};

		QColor minDataColor();
		QColor maxDataColor();
		QColor meshColor(){return meshCol;};
		QColor axesColor(){return axesCol;};
		QColor labelColor(){return labelsCol;};
		QColor numColor(){return numCol;};
		QColor bgColor(){return bgCol;};
		QColor gridColor(){return gridCol;};

		QString colorMap(){return color_map;};
		void setDataColorMap(const QString& fileName);
		bool openColorMap(ColorVector& cv, QString fname);

		void setColors(const QStringList& colors);
		void setColors(const QColor& meshColor,const QColor& axesColor,const QColor& numColor,
				const QColor& labelColor,const QColor& bgColor,const QColor& gridColor);
		//@}

		//! \name Title
		//@{
		void updateTitle(const QString& s,const QColor& color,const QFont& font);
		QFont titleFont(){return titleFnt;};
		void setTitleFont(const QFont& font);
		QString plotTitle(){return title;};
		QColor titleColor(){return titleCol;};
		void setTitle(const QStringList& lst);
		void setTitle(const QString& s,const QColor& color,const QFont& font);
		//@}

		//! \name Resolution
		//@{
		void setResolution(int r);
		int resolution(){return sp->resolution();};
		//@}

		//! \name Legend
		//@{
		void showColorLegend(bool show);
		bool isLegendOn(){return legendOn;};
		//@}

		void setOptions(bool legend, int r, int dist);
		void setOptions(const QStringList& lst);
		void update();

		//! \name Bars
		//@{
		double barsRadius();
		void setBarsRadius(double rad);
		void updateBars(double rad);
		//@}

		//! \name Scatter Plots
		//@{
		double pointsSize(){return pointSize;};
		bool smoothPoints(){return smooth;};
		void updatePoints(double size, bool sm);

		bool smoothCrossHair(){return crossHairSmooth;};
		bool boxedCrossHair(){return crossHairBoxed;};
		double crossHairRadius(){return crossHairRad;};
		double crossHairLinewidth(){return crossHairLineWidth;};
		void updateCross(double rad, double linewidth, bool smooth, bool boxed);
		void setCrossOptions(double rad, double linewidth, bool smooth, bool boxed);
		void setCrossMesh();

		double coneRadius(){return conesRad;};
		int coneQuality(){return conesQuality;};
		void updateCones(double rad, int quality);
		void setConesOptions(double rad, int quality);
		void setConesMesh();

		PointStyle pointType(){return pointStyle;};
		void setPointOptions(double size, bool s);
		//@}

		Table* getTable(){return m_table;};
		void showWorksheet();
		void setSmoothMesh(bool smooth);

		//! Used for the animation: rotates the scene with 1/360 degrees
		void rotate();
		void animate(bool on = true);
		bool isAnimated(){return m_timer->isActive();};

		void findBestLayout();
		bool autoscale(){return m_autoscale;};
		//! Enables/Disables autoscaling using findBestLayout().
		void setAutoscale(bool on = true){m_autoscale = on;};

		void plot3DWireframe();
		void plot3DHiddenLine();
		void plot3DPolygons();
		void plot3DWireSurface();
		void plot3DMatrix(int style);

		Matrix * selectMatrix();

	public:
		static ActionManager * actionManager();
		static void initActionManager();
	private:
		static ActionManager * action_manager;
		//! Private ctor for initActionManager() only
		Graph3D();

	signals:
		void showContextMenu();
		void showOptionsDialog();
		void modified();
		void custom3DActions(QWidget*);

	private:
		void createActions();
		void connectActions();
		static Qwt3D::Triple** allocateData(int columns, int rows);
		static void deleteData(Qwt3D::Triple **data, int columns);
		//! Allocate memory for a matrix buffer
		static double** allocateMatrixData(int rows, int columns);
		//! Free memory used for a matrix buffer
		static void freeMatrixData(double **data, int rows);

		//! Wait this many msecs before redraw 3D plot (used for animations)
		int animation_redraw_wait;
		//! File name of the color map used for the data (if any)
		QString color_map;

		QTimer *m_timer;
		QString title;
		
		// FIXME: is it really a good idea to use indices instead of AbstractColumn*?
		struct {
			Table * table;
			int x_index;
			int y_index;
			int z_index;
			Matrix * matrix;
		} plotAssociation;

		void clearPlotAssociation() 
		{
			plotAssociation.x_index = -1;
			plotAssociation.y_index = -1;
			plotAssociation.z_index = -1;
			plotAssociation.matrix = 0;
			plotAssociation.table = 0;
		}

		QStringList labels;
		QFont titleFnt;
		bool legendOn, smoothMesh, m_autoscale;
		QVector<int> scaleType;
		QColor axesCol,labelsCol,titleCol,meshCol,bgCol,numCol,gridCol;
		//! Custom data colors.
		QColor fromColor, toColor;
		int labelsDist, legendMajorTicks;
		bool ignoreFonts;
		Qwt3D::StandardColor* col_;
		double barsRad, alpha, pointSize, crossHairRad, crossHairLineWidth, conesRad;
		//! Draw 3D points with smoothed angles.
		bool smooth;
		bool crossHairSmooth, crossHairBoxed;
		int conesQuality;
		PointStyle pointStyle;
		Table *m_table;
		Matrix *m_matrix;
		Qwt3D::PLOTSTYLE style_;
		mutable QWidget *m_view;
		QWidget *m_view_widget;
		QVBoxLayout * m_main_layout;

		QAction * action_plot_wire_frame;
		QAction * action_plot_hidden_line;
		QAction * action_plot_polygons;
		QAction * action_plot_wire_surface;
};

//! Class for user defined functions
class UserFunction : public Function
{
	public:

		UserFunction(const QString& s, SurfacePlot& pw);
		~UserFunction();
		double operator()(double x, double y);
		QString function(){return formula;};

	private:
		QString formula;
};

#endif // Plot3D_H
