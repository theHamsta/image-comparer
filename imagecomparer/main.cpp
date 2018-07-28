/**
 * \file   ImageComparerMain.cpp
 * \brief  Main-function for Image Comparer
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   03.10.2016
 */

#include "MainWindow.hpp"

#include <QApplication>
#include <QtGlobal>
#include <QException>
#include <QDebug>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFileInfo>
#include <QMessageBox>


// using namespace ImageComparer;

int main( int argc, char* argv[] )
{
	//     Q_INIT_RESOURCE(resources);
#ifndef NDEBUG
	// see http://doc.qt.io/qt-5/qtglobal.html#qSetMessagePattern for format
	qSetMessagePattern( "[%{type}] (%{time}, thread: %{threadid}) %{message} (%{file}:%{line})" );
#endif

	QApplication app( argc, argv );

	QCoreApplication::setOrganizationName( "LMS" );
	QCoreApplication::setApplicationName( "Image Comparer" );
	QCoreApplication::setApplicationVersion( "0.1" );

	QCommandLineParser parser;
	parser.setApplicationDescription( QCoreApplication::applicationName() );
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument( "file1", QApplication::translate( "ImageComparerMain.cpp", "First image file to open" ) );
	parser.addPositionalArgument( "file2", QApplication::translate( "ImageComparerMain.cpp", "Second image file to open" ) );
	parser.process( app );

#ifdef DARK_THEME_IMAGECOMPARER
	QFile f( ":qdarkstyle/style.qss" );

	if ( !f.exists() ) {
		printf( "Unable to set stylesheet, file not found\n" );
	} else {
		f.open( QFile::ReadOnly | QFile::Text );
		QTextStream ts( &f );
		qApp->setStyleSheet( ts.readAll() );
	}

	QIcon::setThemeName( "breeze-dark" );
#endif

	ImageComparer::MainWindow mainWin;
	mainWin.show();

	if ( !parser.positionalArguments().isEmpty() ) {
		QString file( parser.positionalArguments().first() );

		if ( QFileInfo::exists( file ) ) {
			mainWin.openFile( file, ImageComparer::LeftImage );
		} else {
			QMessageBox msgBox;
			msgBox.setText( QApplication::translate( "ImageComparerMain.cpp", "The file \"%1\" could not be found." ).arg( file ) );
			msgBox.exec();
		}

		if ( parser.positionalArguments().size() == 2 ) {
			QString secondFile( parser.positionalArguments()[1] );

			if ( QFileInfo::exists( secondFile ) ) {
				mainWin.openFile( secondFile, ImageComparer::RightImage );
			} else {
				QMessageBox msgBox;
				msgBox.setText( QApplication::translate( "ImageComparerMain.cpp", "The file \"%1\" could not be found." ).arg( secondFile ) );
				msgBox.exec();
			}
		}

	}

	return app.exec();
}
