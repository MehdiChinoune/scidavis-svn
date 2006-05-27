/***************************************************************************
    File                 : fileDialogs.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email                : ion_vasilief@yahoo.fr, thzs@gmx.net
    Description          : 2 File dialogs: Import multiple ASCII/Export image
                           
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
#ifndef MYFILESDIALOGS_H
#define MYFILESDIALOGS_H

#include <QFileDialog>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QImage>
#include <QImageWriter>
#include <QPicture>
#include <QHBoxLayout>
#include <QtAlgorithms>

//! Import multiple ASCII files dialog
class ImportFilesDialog: public QFileDialog
{
private:
	QComboBox * importType;

public:
	//! Constructor
	/**
	 * \param importTypeEnabled flag: enable/disable import type combo box
	 * \param parent parent widget
	 * \param fl window flags
	 */
	ImportFilesDialog(bool importTypeEnabled, QWidget * parent = 0, Qt::WFlags flags = 0 ) 
	  : QFileDialog( parent, flags )
	{
		setWindowTitle(tr("QtiPlot - Import Multiple ASCII Files"));

		QStringList filters;
		filters << "All files (*)" << "Text (*.TXT *.txt)" << "Data (*.DAT *.dat)";
		setFilters( filters );

		setFileMode( QFileDialog::ExistingFiles );

		if (importTypeEnabled)
		{
			QLabel* label = new QLabel( "Import each file as: " );

			importType = new QComboBox();
			importType->addItem(tr("New Table"));
			importType->addItem(tr("New Columns"));
			importType->addItem(tr("New Rows"));

			// FIXME: The following code may not work anymore
			// if the internal layout of QFileDialog changes
			layout()->addWidget( label );
			layout()->addWidget( importType );
		}
	};

	//! Return the selected import option
	/**
	 * Do not call this when the dialog was
	 * created with importTypeEnabled == false
	 */
	int importFileAs()
	{
		return importType->currentIndex();
	};

};


//! Export as image dialog
class ImageExportDialog: public QFileDialog
{
private:
	QCheckBox * boxOptions;

public:
	//! Constructor
	/**
	 * \param parent parent widget
	 * \param fl window flags
	 */
	ImageExportDialog( QWidget * parent = 0, Qt::WFlags flags = 0 )
	  : QFileDialog( parent, flags )
	{
		setWindowTitle( tr( "QtiPlot - Choose a filename to save under" ) );

		QList<QByteArray> list = QImageWriter::supportedImageFormats();

		list << "EPS";
		qSort(list);

		QStringList filters, selectedFilter;			
		for(int i=0 ; i<list.count() ; i++)
		{
			filters << "*."+list[i].toLower();
		}
		setFilters( filters );
		setFileMode( QFileDialog::AnyFile );

		boxOptions = new QCheckBox();
		boxOptions->setText( "Show export &options" );
#ifdef Q_OS_WIN // Windows systems
		boxOptions->setChecked( true );			
#else
		boxOptions->setChecked( false );
#endif
		// FIXME: The following code may not work anymore
		// if the internal layout of QFileDialog changes
		QSpacerItem * si1 = new QSpacerItem( 20, 20 );
		QSpacerItem * si2 = new QSpacerItem( 20, 20 );
		layout()->addItem( si1 );
		layout()->addItem( si2 );
		layout()->addWidget( boxOptions );
	};

	//! Return whether the export options check box is checked
	bool showExportOptions()
	{
		return boxOptions->isChecked();
	};
};

#endif
