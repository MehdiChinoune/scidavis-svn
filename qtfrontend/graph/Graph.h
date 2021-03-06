/***************************************************************************
    File                 : Graph.h
    Project              : SciDAVis
    Description          : Aspect providing a 2d plotting functionality
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2008 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2006-2008 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2006-2007 Ion Vasilief (ion_vasilief*yahoo.fr)
                           (replace * with @ in the email addresses) 

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
#ifndef GRAPH_H
#define GRAPH_H

#include "core/AbstractPart.h"
#include "lib/ActionManager.h"

class GraphView;

/**
 * Temporary 2d plot solution. to be replaced asap.
 */
class Graph : public AbstractPart
{
	Q_OBJECT

	public:
		Graph(const QString &name);
		~Graph();

		//! Return an icon to be used for decorating my views.
		virtual QIcon icon() const;
		//! Return a new context menu.
		/**
		 * The caller takes ownership of the menu.
		 */
		virtual QMenu *createContextMenu() const;
		//! Construct a primary view on me.
		/**
		 * This method may be called multiple times during the life time of an Aspect, or it might not get
		 * called at all. Aspects must not depend on the existence of a view for their operation.
		 */
		virtual QWidget *view();
		
		//! \name serialize/deserialize
		//@{
		//! Save as XML
		virtual void save(QXmlStreamWriter *) const;
		//! Load from XML
		virtual bool load(XmlStreamReader *);
		//@}
		
	public:
		static ActionManager * actionManager();
		static void initActionManager();
	private:
		static ActionManager * action_manager;
		//! Private ctor for initActionManager() only
		Graph();

	private:
		void createActions();
		void connectActions();
		void addActionsToView();

		GraphView *m_view;
};










// old code, to be ported
#if 0
#include "../core/MyWidget.h"

#include "Layer.h"
#include <QPushButton>
#include <QPointer>

class QWidget;
class QLabel;
class QHBoxLayout;

class LayerButton;
class SelectionMoveResizer;

/**
 * \brief An MDI window (MyWidget) managing one or more Layer objects.
 *
 * \section future_plans Future Plans
 * Manage any QWidget instead of only Layer.
 * This would allow 3D graphs to be added as well, so you could produce mixed 2D/3D arrangements.
 * It would also allow text labels to be added directly instead of having to complicate things by wrapping them
 * up in a Layer (see documentation of ImageMarker for details) (see documentation of ImageMarker for details).
 *
 * The main problem to be figured out for this is how Layer would interface with the rest of the project.
 * A possible solution is outlined in the documentation of ApplicationWindow:
 * If Graph exposes its parent Project to the widgets it manages, they could handle things like creating
 * tables by calling methods of Project instead of sending signals.
 */
class Graph: public MyWidget
{
	Q_OBJECT

public:
	Graph (const QString& label, QWidget* parent=0, const char* name=0, Qt::WFlags f=0);
	QList<Layer*> layers() {return m_layer_list;};
	Layer *layer(int num);
	LayerButton* addLayerButton();
	void copy(Graph* ml);

	enum HorAlignement{HCenter, Left, Right};
	enum VertAlignement{VCenter, Top, Bottom};

	//! \name Event Handlers
	//@{
	void mousePressEvent(QMouseEvent *);
	void contextMenuEvent(QContextMenuEvent *);
	void wheelEvent(QWheelEvent *);
	void keyPressEvent(QKeyEvent *);
	void changeEvent(QEvent *);
	bool eventFilter(QObject *object, QEvent *);
	void releaseLayer();

	bool focusNextPrevChild ( bool next );
	//@}

	void setOpenMaximized(){m_open_maximized = 1;};

	bool scaleLayersOnPrint(){return m_scale_on_print;};
	void setScaleLayersOnPrint(bool on){m_scale_on_print = on;};

	bool printCropmarksEnabled(){return m_print_cropmarks;};
	void printCropmarks(bool on){m_print_cropmarks = on;};

	void setHidden();

public slots:
	Layer* addLayer(int x = 0, int y = 0, int width = 0, int height = 0);
	void setLayersNumber(int n);

	bool isEmpty();
    void removeLayer();
	void confirmRemoveLayer();

	/*!\brief Start adding a text layer.
	 *
	 * This works by having #canvas grab the mouse, remembering that we are in the midst of adding
	 * text in #addTextOn and dispatching the next mouse click to addTextLayer(const QPoint&) in eventFilter().
	 *
	 * \sa #defaultTextMarkerFont, #defaultTextMarkerFrame, #defaultTextMarkerColor, #defaultTextMarkerBackground
	 */
	void addTextLayer(int f, const QFont& font, const QColor& textCol, const QColor& backgroundCol);
	/*!\brief Finish adding a text layer.
	 *
	 * An empty Layer is created and added to me.
	 * Legend, title and axes are removed and a new Legend is added with a placeholder text.
	 *
	 * \sa #defaultTextMarkerFont, #defaultTextMarkerFrame, #defaultTextMarkerColor, #defaultTextMarkerBackground, addTextLayer(int,const QFont&,const QColor&,const QColor&)
	 */
	void addTextLayer(const QPoint& pos);

	Layer* activeLayer(){return m_active_layer;};
	void setActiveLayer(Layer* g);
	void activateLayer(LayerButton* button);

	void setLayerGeometry(int x, int y, int w, int h);

	void findBestLayout(int &rows, int &cols);

	QSize arrangeLayers(bool userSize);
	void arrangeLayers(bool fit, bool userSize);
    void adjustSize();

	int getRows(){return rows;};
	void setRows(int r);

	int getCols(){return cols;};
	void setCols(int c);

	int colsSpacing(){return colsSpace;};
	int rowsSpacing(){return rowsSpace;};
	void setSpacing (int rgap, int cgap);

	int leftMargin(){return left_margin;};
	int rightMargin(){return right_margin;};
	int topMargin(){return top_margin;};
	int bottomMargin(){return bottom_margin;};
	void setMargins (int lm, int rm, int tm, int bm);

	QSize layerCanvasSize(){return QSize(l_canvas_width, l_canvas_height);};
	void setLayerCanvasSize (int w, int h);

	int horizontalAlignement(){return hor_align;};
	int verticalAlignement(){return vert_align;};
	void setAlignement (int ha, int va);

	int layerCount() { return m_layer_count; };

	//! \name Print and Export
	//@{
	QPixmap canvasPixmap();
	void exportToFile(const QString& fileName);
	void exportImage(const QString& fileName, int quality = 100, bool transparent = false);
	void exportSVG(const QString& fname);
    void exportPDF(const QString& fname);
	void exportVector(const QString& fileName, int res = 0, bool color = true,
                    bool keepAspect = true, QPrinter::PageSize pageSize = QPrinter::Custom);

	void copyAllLayers();
	void print();
	void printAllLayers(QPainter *painter);
	void printActiveLayer();
	//@}

	void setFonts(const QFont& titleFnt, const QFont& scaleFnt,
							const QFont& numbersFnt, const QFont& legendFnt);

	void connectLayer(Layer *g);

	QString saveToString(const QString& geometry);
	QString saveAsTemplate(const QString& geometryInfo);

signals:
	void showTextDialog();
	void showPlotDialog(int);
	void showAxisDialog(int);
	void showScaleDialog(int);
	void showLayerContextMenu();
	void showLayerButtonContextMenu();
	void showCurveContextMenu(int);
	void showWindowContextMenu();
	void showCurvesDialog();
	void drawTextOff();
	void drawLineEnded(bool);
	void showXAxisTitleDialog();
	void showYAxisTitleDialog();
	void showTopAxisTitleDialog();
	void showRightAxisTitleDialog();
	void showMarkerPopupMenu();
	void modifiedPlot();
	void cursorInfo(const QString&);
	void showImageDialog();
	void showLineDialog();
	void viewTitleDialog();
	void createTable(const QString&,int,int,const QString&);
	void showGeometryDialog();
	void pasteMarker();
	void createIntensityTable(const QString&);
	void setPointerCursor();

private:
	void resizeLayers (const QResizeEvent *re);
	void resizeLayers (const QSize& size, const QSize& oldSize, bool scaleFonts);

	Layer* m_active_layer;
	//! Used for resizing of layers.
	int m_layer_count, cols, rows, m_layer_default_width, m_layer_default_height, colsSpace, rowsSpace;
	int left_margin, right_margin, top_margin, bottom_margin;
	int l_canvas_width, l_canvas_height, hor_align, vert_align;
	bool addTextOn;
	bool m_scale_on_print, m_print_cropmarks;

	//! Used when adding text markers on new layers
	int defaultTextMarkerFrame;
	QFont defaultTextMarkerFont;
	QColor defaultTextMarkerColor, defaultTextMarkerBackground;

	QWidgetList m_button_list;
	QList<Layer*> m_layer_list;
	QHBoxLayout *layerButtonsBox;
	QWidget *canvas;

	QPointer<SelectionMoveResizer> m_layers_selector;
	int m_open_maximized;
	//! Stores the size of the widget in the Qt::WindowMaximized state.
	QSize m_max_size;
	//! Stores the size of the widget in Qt::WindowNoState (normal state).
	QSize m_normal_size;
};


//! Button with layer number
class LayerButton: public QPushButton
{
	Q_OBJECT

public:
    LayerButton (const QString& text = QString::null, QWidget* parent = 0);
	~LayerButton(){};

	static int btnSize(){return 20;};

protected:
	void mousePressEvent( QMouseEvent * );
	void mouseDoubleClickEvent ( QMouseEvent * );

signals:
	void showCurvesDialog();
	void clicked(LayerButton*);
	void showContextMenu();
};
#endif

#endif // ifndef GRAPH_H
