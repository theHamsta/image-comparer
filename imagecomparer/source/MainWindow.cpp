#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "PythonIntegration.hpp"
#include "PythonPlugIn.hpp"

#include <QSettings>
#include <QDebug>
#include <QLayout>
#include <QMimeType>
#include <QFile>
#include <QDockWidget>
#include <QMessageBox>

#include "SplitView.hpp"
#include "FadeView.hpp"
#include "FlickerView.hpp"
#include "DiffView.hpp"

#include "HeadUpDisplay.hpp"

#include "SimpleCommandPaletteWidget.hpp"
#include "FuzzyCommandPaletteEngine.hpp"
#include "EmbeddedModule.hpp"
#include "NumpyStack.hpp"

#include <QCollator>

using namespace ImageComparer;


#include <exception>

bool isImageByExtension( QFileInfo file )
{
	//TODO: do this whith QMimeType.inherits( image)
	QString extension = file.suffix().toLower();
	return extension == "jpeg" || extension == "jp2" || extension == "jpg" || extension == "ppm" || extension == "bmp" || extension == "tif" || extension == "tiff" || extension == "png";
}

void writeImage( std::string fullPath, cv::Mat image )
{
	try {
		qDebug() << "Writing file \"" << QString::fromStdString( fullPath ) << "\"";

		// Only those depths are supported by cv::imwrite
		if ( image.depth() == CV_8U || image.depth() == CV_16U ) {
			cv::imwrite( fullPath, image );
		} else {
			cv::Mat output;
			image.convertTo( output, CV_8UC3, 255.f );
			cv::imwrite( fullPath, output );
		}
	} catch ( cv::Exception& err ) {
		qDebug() << "Error writing file \"" << QString::fromStdString( fullPath ) << "\": " << QString::fromStdString( err.what() ) << endl;
	}
}

MainWindow::MainWindow( QWidget* parent ) : QMainWindow( parent ),
	ui( new Ui::MainWindow ),
	m_viewModeActionGroup( this )
{
	ui->setupUi( this );
	delete ui->statusbar;
	ui->statusbar = nullptr;

#ifdef WITH_PYTHON
	QString root_pluginDir = QCoreApplication::applicationDirPath() + "/../share/imagecomparer/plugins";
	m_pluginDir = QDir::homePath() + "/.local/share/imagecomparer/";

	if ( !QDir( m_pluginDir ).exists() ) {
		QDir::root().mkpath( m_pluginDir );
		QDirIterator iterator( root_pluginDir );

		while ( iterator.hasNext() ) {
			QFileInfo pluginFile( iterator.next() );

			if ( pluginFile.isFile() && pluginFile.fileName().endsWith( ".py" ) ) {
				QFile::copy( pluginFile.absoluteFilePath(), m_pluginDir + pluginFile.fileName() );
			}
		}
	}

	reloadPlugins();

	// m_pythonDockWidget = new QDockWidget( tr( "Python Console" ), this );
	m_pythonConsole = std::make_unique<PythonConsole>( nullptr );
	// m_pythonDockWidget->setLayout( new QBoxLayout( QBoxLayout::Down ) );
	// m_pythonDockWidget->layout()->addWidget( m_pythonConsole.get() );
	// addDockWidget( Qt::BottomDockWidgetArea, m_pythonDockWidget );

	QAction* openPluginDir = new QAction( tr( "Open Plugin Directory" ), this );
	openPluginDir->setShortcut( QKeySequence( "Ctrl+Alt+P" ) );
	openPluginDir->setIcon( QIcon::fromTheme( "document-open", QIcon( ":/icons/document-open.svg" ) ) );
	connect<>( openPluginDir, &QAction::triggered, [&]() {
		QDesktopServices::openUrl( QUrl::fromLocalFile( m_pluginDir ) );
	} );

	ui->menuPlugIns->addSeparator();
	ui->menuPlugIns->addAction( openPluginDir );

	// 	try {
	// // 	python->exec_commands("foo.process(\'hu\')");
	// 		python->modules()[0].attr("__dict__")["process"]( "huh" );
	// // 		python->getObject("foo.process")( boost::python::str("huh") );
	// 	} catch ( std::exception& e) {
	// 		qDebug()<< e.what();
	//
	// 	}
#endif

	// Actions that are also available in Fullscreen Mode by their keyboard shortcuts
	QList<QAction*> actionsToAddToWidget;
	actionsToAddToWidget.append( ui->menubar->actions() );

	while ( !actionsToAddToWidget.empty() ) {
		QAction* action = actionsToAddToWidget.back();
		actionsToAddToWidget.pop_back();

		if ( action->isSeparator() ) {
		} else if ( action->menu() ) {
			actionsToAddToWidget.append( action->menu()->actions() );
		} else {
			this->addAction( action );
		}
	}

	m_viewModeActionGroup.addAction( ui->actionSplitView );
	m_viewModeActionGroup.addAction( ui->actionBlendView );
	m_viewModeActionGroup.addAction( ui->actionFlickerView );
	m_viewModeActionGroup.addAction( ui->actionDiffView );
	m_viewModeActionGroup.setExclusive( true );

	readSettings();
	setViewMode( m_viewMode );

	connect( &m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::on_fileChanged );
	connect( &m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &MainWindow::on_directoryChanged );


	ui->actionSaveLeftImage->setEnabled( false );
	ui->actionSaveRightImage->setEnabled( false );
	ui->actionCloseLeftImage->setEnabled( false );
	ui->actionCloseRightImage->setEnabled( false );

	QWidget* empty = new QWidget();
	empty->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
	ui->mainToolBar->addWidget( empty );
	m_commandPalette = new SimpleCommandPaletteWidget( this );
	ui->mainToolBar->addWidget( m_commandPalette );
	m_commandPalette->setCommandPaletteEngine( new FuzzyCommandPaletteEngine() );
	m_commandPalette->addActionsFromMenu( ui->menubar );

	// Give m_viewer the focus (or m_commandPalette will handle arrow keys)
	m_viewer->setFocus();
	connect( m_commandPalette, &AbstractCommandPaletteWidget::userInteractionFinished, this, [&]() { m_viewer->setFocus(); } );

	m_commandPalette->commandPaletteEngine()->addFunctionForDynamicActions( [&]( QString searchQuery ) {
		QMimeType mime;
		QList<QAction*> rtn;
		MatcherOptions options;
		options.case_sensitive = false;
		options.num_threads = 3;
		options.max_results = 10;
		options.max_gap = 0;
		options.record_match_indexes = false;

		std::vector<MatchResult> leftResults = m_leftFileMatcher.findMatches( searchQuery.toStdString(), options );

		for ( auto& result : leftResults ) {
			try {

				QFileInfo fileInfo( QString::fromStdString( *result.value ).replace( "Left: ", "" ) );

				QAction* action = new QAction( tr( "Left: Open %1" ).arg( fileInfo.fileName() ), this );
				QIcon icon = QIcon::fromTheme( m_mimeDb.mimeTypeForFile( fileInfo ).iconName(), QIcon( ":icons/image-png.svg" ) );
				action->setIcon( icon );

				QString path = fileInfo.absoluteFilePath();
				connect( action, &QAction::triggered, [ = ]() {
					this->openFile( path, LeftImage );
				} );
				rtn.append( action );
			} catch ( const std::exception& ex ) {
				qDebug() <<  ex.what();
			}
		}

		std::vector<MatchResult> rightResults = m_rightFileMatcher.findMatches( searchQuery.toStdString(), options );

		for ( auto& result : rightResults ) {

			try {
				QFileInfo fileInfo( QString::fromStdString( *result.value ).replace( "Right: ", "" ) ) ;

				QAction* action = new QAction( tr( "Right: Open %1" ).arg( fileInfo.fileName() ), this );
				QIcon icon = QIcon::fromTheme( m_mimeDb.mimeTypeForFile( fileInfo ).iconName(), QIcon( ":icons/image-png.svg" ) );
				action->setIcon( icon );

				QString path = fileInfo.absoluteFilePath();
				connect( action, &QAction::triggered, [ = ]() {
					this->openFile( path, RightImage );
				} );
				rtn.append( action );
			} catch ( const std::exception& ex ) {
				qDebug() <<  ex.what();
			}
		}

		return rtn;
	} );
}

MainWindow::~MainWindow()
{
	for ( auto p : m_plugIns ) {
		delete p;
	}

	delete ui;
}

void MainWindow::closeEvent( QCloseEvent* event )
{
	qDebug() << "Main window closed";
	m_pythonConsole->close();
	event->accept();

	writeSetings();
}

void MainWindow::keyReleaseEvent( QKeyEvent* event )
{
	// Exit fullscreen on Escape
	if ( event->key() == Qt::Key_Escape && windowState() == Qt::WindowFullScreen ) {
		ui->actionExitFullscreen->trigger();
	}
}

void MainWindow::dragEnterEvent( QDragEnterEvent* event )
{
	const QMimeData* mimeData = event->mimeData();

	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();

		if ( !urlList.empty() ) {

			QString file = urlList[0].toLocalFile();

			if ( QFileInfo::exists( file ) ) {
				event->accept();
			} else {
				qDebug() << "dragEnterEvent rejected (url was" << file << ")";
			}
		}
	} else {
		qDebug() << "dragEnterEvent rejected";
	}

	QWidget::dragEnterEvent( event );
}

void MainWindow::dropEvent( QDropEvent* event )
{
	const QMimeData* mimeData = event->mimeData();

	if ( mimeData->hasUrls() ) {
		qDebug() << "File dropped";

		QList<QUrl> urlList = mimeData->urls();

		if ( !urlList.empty() ) {
			QString file = urlList[0].toLocalFile();
			bool onLeftSide = event->pos().x() < width() / 2;

			if ( QFileInfo::exists( file ) ) {
				openFile( file, onLeftSide ? LeftImage : RightImage );
				this->activateWindow();
			} else {
				qDebug() << "Dropped file rejected (was" << file << ")";
			}
		}
	}

	m_viewer->primaryVideoOutput()->hideHighlightedRect();
}

void MainWindow::dragMoveEvent( QDragMoveEvent* event )
{
	int splitPos = width() / 2;
	// 	SplitView* splitView = dynamic_cast<SplitView*>( m_viewer );
	//
	// 	if ( splitView ) {
	// 		splitPos = width() * splitView->sliderVal();
	// 	}

	bool onLeftSide = event->pos().x() < splitPos;

	if ( onLeftSide ) {
		m_viewer->primaryVideoOutput()->showHighlightedRect( QRect( 0, 0, splitPos, m_viewer->height() ), HighlighWidgetCoordinates );
	} else {
		m_viewer->primaryVideoOutput()->showHighlightedRect( QRect( splitPos, 0, width() - splitPos, m_viewer->height() ), HighlighWidgetCoordinates );
	}

	QWidget::dragMoveEvent( event );
}

void MainWindow::dragLeaveEvent( QDragLeaveEvent* event )
{
	event->accept();
	m_viewer->primaryVideoOutput()->hideHighlightedRect();
}

void MainWindow::openFile( const QString& file, ImageSide side, bool isReloadOpening )
{
	qDebug() << "Try to read" << file;
	FrameStack* reader = ( ( ImageSide::LeftImage == side ) ? m_leftStack.get() : m_rightStack.get() );
	FrameStack* otherReader = ( ( ImageSide::RightImage == side ) ? m_leftStack.get() : m_rightStack.get() );
	reader->release();
	cv::Mat mat = cv::imread( file.toStdString(), cv::IMREAD_COLOR );

	if ( mat.empty() ) {
		qDebug() << "Failed to load image with OpenCV" << file;

		if ( file.endsWith( ".tif", Qt::CaseInsensitive ) || file.endsWith( ".tiff", Qt::CaseInsensitive ) ) {

			try {
				auto tiffReader = dynamic_cast<TiffStackReader*>( reader );

				if ( tiffReader ) {
					tiffReader->openTiff( file.toStdString().c_str(), isReloadOpening );
				} else {
					if ( side == LeftImage ) {
						m_leftStack = std::make_shared<TiffStackReader>( file.toStdString().c_str(), ui->actionKeepAllInRam->isChecked() );
						reader = m_leftStack.get();
					} else {
						m_rightStack = std::make_shared<TiffStackReader>( file.toStdString().c_str(), ui->actionKeepAllInRam->isChecked() );
						reader = m_rightStack.get();
					}
				}

				if ( otherReader->hasFileOpen() && otherReader->currentIdx() < reader->numFrames() ) {
					reader->goToFrame( otherReader->currentIdx() );
				}

				mat = reader->currentFrame();
			} catch ( ... ) {
				qDebug() << "Failed to load image" << file;
			}
		}
	} else {
		qDebug() << "Successfully opened file" << file;
	}

	QFileInfo leftInfo( m_leftImgPath );
	QFileInfo rightInfo( m_rightImgPath );

	switch ( side ) {
		case ImageComparer::LeftImage: {
			if ( !m_leftImgPath.isEmpty() && // remove old dir if available
					( m_rightImgPath.isEmpty() || rightInfo.absoluteDir().absolutePath() != leftInfo.absoluteDir().absolutePath() ) ) {
				// do not remove right image's directory
				m_fileSystemWatcher.removePath( leftInfo.absoluteDir().absolutePath() );
				qDebug() << "Directory" << leftInfo.absoluteDir().absolutePath() << "will no longer be observed to detect file changes";
			}

			if ( m_leftImgPath != m_rightImgPath ) {
				m_fileSystemWatcher.removePath( leftInfo.absoluteFilePath() );
			}

			m_leftImg = mat;
			m_leftImgPath = file;
			m_viewer->setFirstImage( mat );

			if ( m_rightImg.empty() ) {
				updateFileInterators( ImageComparer::RightImage );
			}

			updateFileInterators( ImageComparer::LeftImage );
			break;
		}

		case ImageComparer::RightImage: {
			if ( !m_rightImgPath.isEmpty() && // remove old dir if available
					( m_leftImgPath.isEmpty() || rightInfo.absoluteDir().absolutePath() != leftInfo.absoluteDir().absolutePath() ) ) {
				// do not remove left image's directory
				m_fileSystemWatcher.removePath( rightInfo.absoluteDir().absolutePath() );
				qDebug() << "Directory" << rightInfo.absoluteDir().absolutePath() << "will no longer be observed to detect file changes";
			}

			if ( m_leftImgPath != m_rightImgPath ) {
				m_fileSystemWatcher.removePath( rightInfo.absoluteFilePath() );
			}

			m_rightImg = mat;
			m_rightImgPath = file;
			m_viewer->setSecondImage( mat );

			if ( m_leftImg.empty() ) {
				updateFileInterators( ImageComparer::LeftImage );
			}

			updateFileInterators( ImageComparer::RightImage );

			break;
		}
	}

	QFileInfo info( LeftImage == side ? m_leftImgPath : m_rightImgPath );

	if ( m_fileSystemWatcher.addPath( info.absoluteFilePath() ) ) {
		qDebug() << "File" << info.absoluteFilePath() << "will be observed to detect file changes";
	}

	if ( m_fileSystemWatcher.addPath( info.absoluteDir().absolutePath() ) ) {
		qDebug() << "Directory" << info.absoluteDir().absolutePath() << "will be observed to detect file changes";
	}

	updateLabels();
}

void MainWindow::readSettings()
{
	qDebug() << "Reading settings file";
	QSettings settings( QCoreApplication::organizationName(), QCoreApplication::applicationName() );

	m_viewMode = static_cast<ViewMode>( std::min( settings.value( "viewMode", ( uint )SplitViewMode ).toUInt(), static_cast<uint>( numViewModes ) ) );

	const QByteArray state = settings.value( "state", QByteArray() ).toByteArray();
	const QByteArray geometry = settings.value( "geometry", QByteArray() ).toByteArray();

	if ( !state.isEmpty() ) {
		restoreState( state );
	}

	if ( !geometry.isEmpty() ) {
		restoreGeometry( geometry );
	}

	m_linearInterpolationDisplay = settings.value( "linearInterpolation", false ).toBool();
	ui->actionBilinearInterpolation->setChecked( m_linearInterpolationDisplay );
	ui->actionShowInfoBox->setChecked( settings.value( "showInfoBox", false ).toBool() );
	ui->actionShowMenu->setChecked( settings.value( "showMenu", true ).toBool() );
	ui->actionKeepAllInRam->setChecked( settings.value( "keepAllTiffFramesInRam", false ).toBool() );
	ui->menubar->setVisible( ui->actionShowMenu->isChecked() );
}

void MainWindow::writeSetings()
{
	qDebug() << "Writing settings file";

	if ( windowState() == Qt::WindowFullScreen ) {
		ui->actionExitFullscreen->trigger();
	}

	QSettings settings( QCoreApplication::organizationName(), QCoreApplication::applicationName() );
	settings.setValue( "geometry", saveGeometry() );
	settings.setValue( "state", saveState() );
	settings.setValue( "viewMode", m_viewMode );
	settings.setValue( "linearInterpolation", m_linearInterpolationDisplay );
	settings.setValue( "showInfoBox", ui->actionShowInfoBox->isChecked() );
	settings.setValue( "showMenu", ui->actionShowMenu->isChecked() );
	settings.setValue( "keepAllTiffFramesInRam", ui->actionKeepAllInRam->isChecked() );
}

void MainWindow::setViewMode( ViewMode viewMode )
{
	QCvMatViewerViewPort viewPort;
	bool setViewPort = false;

	m_viewMode = viewMode;

	if ( m_viewer ) {
		if ( m_viewer->primaryVideoOutput() ) {
			viewPort = m_viewer->primaryVideoOutput()->currentViewPort();
			setViewPort = true;
		}

		delete m_viewer;
		m_viewer = nullptr;
	}

	switch ( viewMode ) {
		case ImageComparer::SplitViewMode: {
			qDebug() << "ViewMode changed to \"SplitView\"";
			ui->actionSplitView->setChecked( true );
			m_viewer = new SplitView( this );
			break;
		}

		case ImageComparer::BlendViewMode: {
			qDebug() << "ViewMode changed to \"BlendView\"";
			ui->actionBlendView->setChecked( true );
			m_viewer = new FadeView( this );
			break;
		}

		case ImageComparer::FlickerViewMode: {
			qDebug() << "ViewMode changed to \"FlickerView\"";
			ui->actionFlickerView->setChecked( true );
			m_viewer = new FlickerView( this );
			break;
		}

		case ImageComparer::DiffViewMode: {
			qDebug() << "ViewMode changed to \"DiffView\"";
			ui->actionDiffView->setChecked( true );
			m_viewer = new DiffView( this );
			break;
		}

		case ImageComparer::Unset:
			assert( false && "Should not be set to unset!" );
			return;
	}

	m_viewer->setScalingMethod( m_linearInterpolationDisplay ? ScalingMethod::Bilinear : NearestNeighbor );
	m_viewer->primaryVideoOutput()->setBackgroundColor( 0.2f, 0.2f, 0.2f );

	setCentralWidget( m_viewer );

	m_viewer->setFirstImage( m_leftImg );
	m_viewer->setSecondImage( m_rightImg );

	if ( m_viewer->primaryVideoOutput() && setViewPort ) {
		m_viewer->primaryVideoOutput()->setViewPort( viewPort );
	}

	m_viewer->setContextMenuPolicy( Qt::ContextMenuPolicy::CustomContextMenu );
	connect( m_viewer, &QWidget::customContextMenuRequested, this, &MainWindow::on_customContextMenuRequested );

	on_actionShowInfoBox_toggled( ui->actionShowInfoBox->isChecked() );
	updateLabels();
}

void MainWindow::updateLabels()
{
	m_viewer->setFirstImageDescription( m_leftImg.empty() ? "" : m_leftImgPath );

	m_viewer->setSecondImageDescription( m_rightImg.empty() ? "" : m_rightImgPath );

	QFileInfo leftInfo( m_leftImg.empty() ? "" : m_leftImgPath );
	QFileInfo rightInfo( m_rightImg.empty() ? "" : m_rightImgPath );

	QString dirNameLeft = m_leftImgPath.isEmpty() ? "." : leftInfo.absoluteDir().dirName();
	QString dirNameRight = m_rightImgPath.isEmpty() ? "." : rightInfo.absoluteDir().dirName();

	QString leftFileName = m_leftImgPath.isEmpty() ? "." : leftInfo.fileName();
	QString rightFileName = m_rightImgPath.isEmpty() ? "." : rightInfo.fileName();

	QString leftDimZ;

	if ( m_leftStack->hasFileOpen() ) {
		leftDimZ = "x" + QString::number( m_leftStack->numFrames() );
	}

	QString rightDimZ;

	if ( m_rightStack->hasFileOpen() ) {
		rightDimZ = "x" + QString::number( m_rightStack->numFrames() );
	}

	if ( m_leftStack->hasFileOpen() ) {
		leftFileName += tr( " (%1)" ).arg( m_leftStack->currentIdx() + 1 );
		dirNameLeft += tr( " (%1)" ).arg( m_leftStack->currentIdx() + 1 );
		m_viewer->setFirstImageDescription( m_leftImgPath + tr( " (%1/%2)" ).arg( m_leftStack->currentIdx() + 1 ).arg( m_leftStack->numFrames() ) );
	}

	if ( m_rightStack->hasFileOpen() ) {
		rightFileName += tr( " (%1)" ).arg( m_rightStack->currentIdx() + 1 );
		dirNameRight += tr( " (%1)" ).arg( m_rightStack->currentIdx() + 1 );
		m_viewer->setSecondImageDescription( m_rightImgPath + tr( " (%1/%2)" ).arg( m_rightStack->currentIdx() + 1 ).arg( m_rightStack->numFrames() ) );
	}

	if ( leftFileName != rightFileName ) {
		this->setWindowTitle( "\"" + leftFileName + "\" vs \"" + rightFileName + "\" - Image Comparer" );
	} else {
		this->setWindowTitle( "\"" + dirNameLeft + "\" vs \"" + dirNameRight + "\" - Image Comparer" );
	}

	if ( m_leftImg.size() == m_rightImg.size() ) {
		ui->mainToolBar->setToolTip( tr( "Resolution of both images: %1x%2" ).arg( m_leftImg.cols ).arg( m_leftImg.rows ) );
	} else {
		ui->mainToolBar->setToolTip( tr( "Resolution of left image: %1x%2"
										 "\nResolution of right image: %3x%4" )
									 .arg( m_leftImg.cols )
									 .arg( m_leftImg.rows )
									 .arg( m_rightImg.cols )
									 .arg( m_rightImg.rows ) );
	}

	// 	m_infoText = tr( "<p><i>%1</i><br>Resolution: %2x%3</p><p><i>%4</i><br>Resolution: %5x%6</p>" )
	if ( !m_leftImg.empty() && !m_rightImg.empty() ) {
		m_infoText = tr( "<p><i>%1</i><br><font color=#f0f0f0>%2x%3%4</font></p><p><i>%5</i><br><font color=#f0f0f0>%6x%7%8</font></p>" )
					 .arg( leftFileName )
					 .arg( m_leftImg.cols )
					 .arg( m_leftImg.rows )
					 .arg( leftDimZ )
					 .arg( rightFileName )
					 .arg( m_rightImg.cols )
					 .arg( m_rightImg.rows )
					 .arg( rightDimZ );
	} else if ( !m_leftImg.empty() ) {
		m_infoText = tr( "<p><i>%1</i><br><font color=#f0f0f0>%2x%3%4</font></p>" )
					 .arg( leftFileName )
					 .arg( m_leftImg.cols )
					 .arg( m_leftImg.rows )
					 .arg( leftDimZ );
	} else if ( !m_rightImg.empty() ) {
		m_infoText = tr( "<p><i>%1</i><br><font color=#f0f0f0>%2x%3%4</font></p>" )
					 .arg( rightFileName )
					 .arg( m_rightImg.cols )
					 .arg( m_rightImg.rows )
					 .arg( rightDimZ );
	} else {
		m_infoText = "";
	}

	on_actionShowInfoBox_toggled( ui->actionShowInfoBox->isChecked() );

	ui->actionCloseLeftImage->setEnabled( !m_leftImg.empty() );
	ui->actionCloseRightImage->setEnabled( !m_rightImg.empty() );
	ui->actionSaveLeftImage->setEnabled( !m_leftImg.empty() );
	ui->actionSaveRightImage->setEnabled( !m_rightImg.empty() );
	ui->actionGoToFrame->setEnabled( m_leftStack->hasFileOpen() || m_rightStack->hasFileOpen() );
	ui->actionKeepAllInRam->setEnabled( m_leftStack->hasFileOpen() || m_rightStack->hasFileOpen() );

	if ( m_leftStack->hasFileOpen() ) {
		m_lastLeftIdx = m_leftStack->currentIdx();
	}

	if ( m_rightStack->hasFileOpen() ) {
		m_lastLeftIdx = m_rightStack->currentIdx();
	}

	if ( m_leftImg.empty() && m_rightImg.empty() ) {

		m_viewer->headUpDisplay()->showPermanentMessage( tr( "Drag two images onto this window to compare them! One to the left and one to the right." ) );
	} else {
		m_viewer->headUpDisplay()->showPermanentMessage( "" );
	}

}

void ImageComparer::MainWindow::on_actionSplitView_triggered()
{
	setViewMode( SplitViewMode );
}

void ImageComparer::MainWindow::on_actionBlendView_triggered()
{
	setViewMode( BlendViewMode );
}

void ImageComparer::MainWindow::on_actionFlickerView_triggered()
{
	setViewMode( FlickerViewMode );
}

void ImageComparer::MainWindow::on_actionDiffView_triggered()
{
	setViewMode( DiffViewMode );
}

void ImageComparer::MainWindow::on_actionQuit_triggered()
{
	close();
}

void ImageComparer::MainWindow::on_actionNextImageLeftAndRight_triggered()
{
	moveFilePointer( +1, LeftImage );
	moveFilePointer( +1, RightImage );
	updateLabels();
}

void ImageComparer::MainWindow::on_actionPreviousImageLeftAndRight_triggered()
{
	moveFilePointer( -1, LeftImage );
	moveFilePointer( -1, RightImage );
	updateLabels();
}

void ImageComparer::MainWindow::on_actionNextImageRightOnly_triggered()
{
	moveFilePointer( +1, RightImage );
	updateLabels();
}

void ImageComparer::MainWindow::on_actionNextImageLeftOnly_triggered()
{
	moveFilePointer( +1, LeftImage );
	updateLabels();
}

void ImageComparer::MainWindow::on_actionPreviousImageRight_triggered()
{
	moveFilePointer( -1, RightImage );
	updateLabels();
}

void ImageComparer::MainWindow::on_actionPreviousImageLeft_triggered()
{
	moveFilePointer( -1, LeftImage );
	updateLabels();
}

void MainWindow::moveFilePointer( int delta, ImageSide side )
{
	if ( ( side == LeftImage && m_leftImg.empty() ) || ( side == RightImage && m_rightImg.empty() ) ) {
		return;
	}

	std::shared_ptr<FrameStack> reader = ( ( ImageSide::LeftImage == side ) ? m_leftStack : m_rightStack );
	cv::Mat* img = ( ( ImageSide::LeftImage == side ) ? &m_leftImg : &m_rightImg );
	QString file;

	switch ( side ) {
		case ImageComparer::LeftImage: {
			bool doActionOnStack = m_leftStack->hasFileOpen() &&
								   !( delta == -1 && m_leftStack->currentIdx() == 0 && !m_leftStack->keepAllFramesInRam() ) &&
								   !( delta == +1 && !m_leftStack->hasMoreFrames() && !m_leftStack->keepAllFramesInRam() );

			if ( !doActionOnStack && !m_leftImgFileList.empty() ) {
				reader->release();

				if ( delta == 1 && m_leftFileIterator != m_leftImgFileList.end() - 1 ) {
					m_leftFileIterator++;
				} else if ( delta == -1 && m_leftFileIterator != m_leftImgFileList.begin() ) {
					m_leftFileIterator--;
				} else {
					return;
				}

				m_fileSystemWatcher.removePath( m_leftImgPath );

				file = m_leftFileIterator->absoluteFilePath();
				m_leftImgPath = file;
				m_fileSystemWatcher.addPath( m_leftImgPath );
			}

			break;
		}

		case ImageComparer::RightImage: {
			bool doActionOnStack = m_rightStack->hasFileOpen() &&
								   !( delta == -1 && m_rightStack->currentIdx() == 0 && !m_rightStack->keepAllFramesInRam() ) &&
								   !( delta == +1 && !m_rightStack->hasMoreFrames() && !m_rightStack->keepAllFramesInRam() );

			if ( !doActionOnStack && !m_rightImgFileList.empty() ) {
				reader->release();

				if ( delta == 1 && m_rightFileIterator != m_rightImgFileList.end() - 1 ) {
					m_rightFileIterator++;
				} else if ( delta == -1 && m_rightFileIterator != m_rightImgFileList.begin() ) {
					m_rightFileIterator--;
				} else {
					return;
				}

				m_fileSystemWatcher.removePath( m_rightImgPath );
				file = m_rightFileIterator->absoluteFilePath();
				m_rightImgPath = file;
				m_fileSystemWatcher.addPath( file );
			}

			break;
		}
	}

	if ( !file.isEmpty() ) {

		cv::Mat mat = cv::imread( file.toStdString(), cv::IMREAD_COLOR );

		if ( mat.empty() ) {
			qDebug() << "Failed to load image" << file;

			if ( file.endsWith( "tif", Qt::CaseInsensitive ) || file.endsWith( "tiff", Qt::CaseInsensitive ) ) {
				openFile( file, side );

				if ( delta == -1 && reader->hasFileOpen() ) {
					reader->goToFrame( reader->numFrames() - 1 );
				}

				return;
			}
		} else {
			qDebug() << "Successfully opened file" << file;

			if ( side == ImageComparer::LeftImage ) {
				m_leftImg = mat;
				m_viewer->setFirstImage( m_leftImg );
			} else {
				m_rightImg = mat;
				m_viewer->setSecondImage( m_rightImg );
			}
		}
	} else if ( reader->hasFileOpen() ) {
		if ( delta == +1 ) {
			if ( reader->hasMoreFrames() ) {
				reader->nextFrame();
			}
		} else if ( delta == -1 && reader->currentIdx() != 0 ) {
			reader->previousFrame();
		}

		*img = reader->currentFrame();

		if ( side == LeftImage ) {
			m_viewer->setFirstImage( *img );
		} else {
			m_viewer->setSecondImage( *img );
		}
	}
}

void ImageComparer::MainWindow::on_actionShowFullscreen_triggered()
{
	auto formerState = windowState();
	// 	bool mainToolBarWasVisible = ui->mainToolBar->isVisible();
	bool menuBarWasVisible = ui->menubar->isVisible();

	showFullScreen();
	ui->actionExitFullscreen->setVisible( true );
	ui->actionShowFullscreen->setVisible( false );
	ui->menubar->setVisible( false );
	ui->actionShowMenu->setVisible( false );
	// 	ui->mainToolBar->setVisible( false );

	ui->actionExitFullscreen->disconnect();
	connect( ui->actionExitFullscreen, &QAction::triggered,
	[ = ]() {
		if ( isFullScreen() ) {

			if ( formerState != Qt::WindowFullScreen ) {
				setWindowState( formerState );
			} else {
				showNormal();
			}

			ui->actionExitFullscreen->setVisible( false );
			ui->actionShowFullscreen->setVisible( true );

			ui->menubar->setVisible( menuBarWasVisible );
			// 			ui->mainToolBar->setVisible( mainToolBarWasVisible );
			ui->actionShowMenu->setVisible( true );
		}
	} );
}

void MainWindow::on_customContextMenuRequested( const QPoint& posOnCentralWidget )
{
	QMenu menu( this );

	//     menu.addSection("View");
	menu.addAction( ui->actionSplitView );
	menu.addAction( ui->actionBlendView );
	menu.addAction( ui->actionFlickerView );
	menu.addAction( ui->actionDiffView );

	menu.addSeparator();
	menu.addAction( ui->actionBilinearInterpolation );

	menu.addSeparator();
	menu.addAction( ui->actionExitFullscreen );

	menu.exec( m_viewer->mapToGlobal( posOnCentralWidget ) );
}

void MainWindow::on_directoryChanged( const QString& path )
{
	qDebug() << "Directory changed:" << path;

	if ( QFileInfo( m_leftImgPath ).absoluteDir() == path ) {
		updateFileInterators( ImageComparer::LeftImage );

		if ( !m_leftImgPath.isEmpty() && m_leftImg.empty() ) {
			openFile( m_leftImgPath, ImageComparer::LeftImage );
		}
	}

	if ( QFileInfo( m_rightImgPath ).absoluteDir() == path ) {
		updateFileInterators( ImageComparer::RightImage );

		if ( !m_rightImgPath.isEmpty() && m_rightImg.empty() ) {
			openFile( m_rightImgPath, ImageComparer::RightImage );
		}
	}
}

void MainWindow::on_fileChanged( const QString& path )
{
	qDebug() << "File changed:" << path;
	const bool isReloadOpening = true;

	if ( QFileInfo( m_leftImgPath ).absoluteFilePath() == path ) {
		openFile( m_leftImgPath, ImageComparer::LeftImage, isReloadOpening );
	}

	if ( QFileInfo( m_rightImgPath ).absoluteFilePath() == path ) {
		openFile( m_rightImgPath, ImageComparer::RightImage, isReloadOpening );
	}
}

void ImageComparer::MainWindow::on_actionBilinearInterpolation_triggered()
{
	qDebug() << "Interpolation mode changed";

	m_linearInterpolationDisplay = ui->actionBilinearInterpolation->isChecked();
	setViewMode( m_viewMode );
}

void MainWindow::on_actionSaveLeftImage_triggered()
{
	if ( !m_leftImg.empty() ) {

		QString oldFileName = QFileInfo( m_leftImgPath ).fileName();
		QString saveFileName = QFileDialog::getSaveFileName( this,
							   tr( "Save a copy of %1" ).arg( oldFileName ),
							   m_leftImgPath,
							   tr( "Portable Network Graphics (*.png);;JPEG (*.jpg);;Bitmap (*.bmp)" ) );

		if ( !saveFileName.isEmpty() ) {
			writeImage( saveFileName.toStdString(), m_leftImg );
		}
	}
}

void MainWindow::on_actionSaveRightImage_triggered()
{
	if ( !m_rightImg.empty() ) {
		QString saveFileName = QFileDialog::getSaveFileName( this,
							   tr( "Save a copy of %1" ).arg( QFileInfo( m_rightImgPath ).fileName() ),
							   m_rightImgPath, // folder
							   tr( "Portable Network Graphics (*.png);;JPEG (*.jpg);;Bitmap (*.bmp)" ) );

		if ( !saveFileName.isEmpty() ) {
			writeImage( saveFileName.toStdString(), m_rightImg );
		}
	}
}

void MainWindow::on_actionShowInfoBox_toggled( bool checked )
{
	if ( m_viewer ) {
		if ( checked ) {
			m_viewer->headUpDisplay()->showInfoBox( m_infoText );
		} else {
			m_viewer->headUpDisplay()->hideInfoBox();
		}
	}
}

void MainWindow::on_actionOpenImageLeft_triggered()
{
	qDebug() << "Action \"Open File\" triggered";
	qDebug() << "Show file open dialog";

	QString path;

	if ( QFileInfo::exists( m_leftImgPath ) ) {
		path = m_leftImgPath;
	} else {
		path = QStandardPaths::standardLocations( QStandardPaths::PicturesLocation )[0];
	}

	QString selectedFile = QFileDialog::getOpenFileName( this,
						   tr( "Open Image" ), path, tr( "Image Files (*.jpg *.jpeg *.png *.bmp *.ppm *.tiff *.tif)" ) );

	if ( selectedFile.length() == 0 ) {
		qDebug() << "No file was selected in QFileDialog";
	} else {
		qDebug() << "File" << selectedFile << "was selected in QFileDialog";
		openFile( selectedFile, ImageSide::LeftImage );
	}
}

void MainWindow::on_actionOpenImageRight_triggered()
{
	qDebug() << "Action \"Open File\" triggered";
	qDebug() << "Show file open dialog";

	QString path;

	if ( QFileInfo::exists( m_rightImgPath ) ) {
		path = m_rightImgPath;
	} else {
		path = QStandardPaths::standardLocations( QStandardPaths::PicturesLocation )[0];
	}

	QString selectedFile = QFileDialog::getOpenFileName( this,
						   tr( "Open Image" ), path, tr( "Image Files (*.jpg *.jpeg *.png *.bmp *.ppm *.tiff *.tif)" ) );

	if ( selectedFile.length() == 0 ) {
		qDebug() << "No file was selected in QFileDialog";
	} else {
		qDebug() << "File" << selectedFile << "was selected in QFileDialog";
		openFile( selectedFile, RightImage );
	}
}

void ImageComparer::MainWindow::on_actionCloseLeftImage_triggered()
{
	m_leftImg.release();
	m_leftStack->release();
	m_viewer->setFirstImage( m_leftImg );

	ui->actionCloseLeftImage->setEnabled( false );
	ui->actionSaveLeftImage->setEnabled( false );
	updateLabels();

	m_leftImgPath = ""; //QFileInfo(m_rightImgPath).absoluteDir();
	updateFileInterators( LeftImage );
}

void ImageComparer::MainWindow::on_actionCloseRightImage_triggered()
{
	m_rightImg.release();
	m_rightStack->release();
	m_viewer->setSecondImage( m_rightImg );

	ui->actionCloseRightImage->setEnabled( false );
	ui->actionSaveRightImage->setEnabled( false );
	updateLabels();

	m_rightImgPath = "";
	updateFileInterators( RightImage );
}

void ImageComparer::MainWindow::on_actionShowMenu_toggled( bool checked )
{
	ui->menubar->setVisible( checked );
}

void ImageComparer::MainWindow::on_actionAbout_triggered()
{
	qDebug() << "Action \"actionAbout\" triggered";
	QMessageBox::about( this,
						tr( "About this program" ),
						tr( "This program was created as a byproduct of the following master's thesis: <br>"
							"<center><font size= \"32\"><b><i>"
							"Design of a Demonstrator for Super-Resolution of Compressed Video Data in CUDA"
							"</i></b><br></font>"
							" by Stephan Seitz (stephan.seitz@fau.de)</center>" ) );
}

void ImageComparer::MainWindow::on_actionAdjustBrightness_triggered()
{
	if ( m_leftStack->hasFileOpen() ) {
		m_leftStack->adjustBrightness();
		m_leftImg = m_leftStack->currentFrame();
		m_viewer->setFirstImage( m_leftImg );
	}

	if ( m_rightStack->hasFileOpen() ) {
		m_rightStack->adjustBrightness();
		m_rightImg = m_rightStack->currentFrame();
		m_viewer->setSecondImage( m_rightImg );
	}
}

void ImageComparer::MainWindow::on_actionGoToFrame_triggered()
{
	qDebug() << "Action \"actionGoToFrame\" triggered";

	int maxFrames = std::max( m_leftStack->numFrames(), m_rightStack->numFrames() );
	int defaultVal = std::max( m_leftStack->currentIdx(), m_rightStack->currentIdx() );

	bool ok;
	int frameIdx = QInputDialog::getInt( this,
										 tr( "Go to Frame" ),
										 tr( "Enter the index of the frame you want to go to (between 1 and " ) + QString::number( maxFrames ) + "):",
										 defaultVal + 1, // default value
										 1,				// minVal
										 maxFrames,
										 1, // step
										 &ok );

	if ( ok ) {
		if ( m_leftStack->hasFileOpen() ) {
			m_leftStack->goToFrame( frameIdx - 1 );
			m_leftImg = m_leftStack->currentFrame();
			m_viewer->setFirstImage( m_leftImg );
		}

		if ( m_rightStack->hasFileOpen() ) {
			m_rightStack->goToFrame( frameIdx - 1 );
			m_rightImg = m_rightStack->currentFrame();
			m_viewer->setSecondImage( m_rightImg );
		}
	}

	updateLabels();
}

void ImageComparer::MainWindow::on_actionKeepAllInRam_toggled( bool checked )
{
	m_leftStack->setKeepAllFramesInRam( checked );
	m_rightStack->setKeepAllFramesInRam( checked );
}

//void ImageComparer::MainWindow::on_actionSwapImagesLeftRight_triggered()
//{
//assert( false && "not implemented" );
//std::swap(m_leftFileIterator,  m_rightFileIterator);
//std::swap(m_leftImgFileList,  m_rightImgFileList);
//std::swap(m_rightImg,  m_leftImg);
//std::swap(m_rightImgPath,  m_leftImgPath);
//std::swap(m_rightImg,  m_leftImg);
//// 	std::swap(m_leftStack,  m_rightStack);
//}

void ImageComparer::MainWindow::updateFileInterators( ImageSide side )
{
	QFileInfo info( LeftImage == side ? m_leftImgPath : m_rightImgPath );
	QFileInfoList dirUnfiltered = info.absoluteDir().entryInfoList( QDir::Files, QDir::NoSort ); //Name | QDir::LocaleAware );

	QCollator collator;
	collator.setNumericMode( true );
	std::sort( dirUnfiltered.begin(), dirUnfiltered.end(), [&]( const QFileInfo & f1, const QFileInfo & f2 ) {
		return collator.compare( f1.absoluteFilePath(), f2.absoluteFilePath() ) < 0;
	} );


	QFileInfoList dir;

	for ( QFileInfo& f : dirUnfiltered ) {
		if ( isImageByExtension( f ) ) {
			dir.append( f );
		}
	}

	qDebug() << "Found" << dir.size() << "files in image folder" << info.absoluteDir().absolutePath();

	auto findResult = std::find( dir.begin(), dir.end(), info );
	bool found = findResult != dir.end();

	if ( side == LeftImage ) {

		if ( found ) {
			m_leftImgFileList = dir;
			m_leftFileIterator = findResult;
		} else {
			m_leftImgFileList.clear();
		}


		// try {
		m_leftFileMatcher.clear();

		for ( auto& fileInfo : dir ) {
			QString path ( fileInfo.absoluteFilePath() );
			m_leftFileMatcher.addCandidate( "Left: " + path.toStdString() );

			if ( m_rightImg.empty() ) {
				m_rightFileMatcher.addCandidate( "Right: " + path.toStdString() );
			}
		}

		// } catch ( const std::exception& ) {


		// }
	} else {
		if ( found ) {
			m_rightImgFileList = dir;
			m_rightFileIterator = findResult;
		} else {
			m_rightImgFileList.clear();
		}

		m_rightFileMatcher.clear();

		for ( auto fileInfo : dir ) {
			m_rightFileMatcher.addCandidate( "Right: " + fileInfo.absoluteFilePath().toStdString() );

			if ( m_leftImg.empty() ) {
				m_leftFileMatcher.addCandidate( "Left: " + fileInfo.absoluteFilePath().toStdString() );
			}
		}


	}

}

void MainWindow::updateDisplay()
{
	m_viewer->setFirstImage( m_leftImg );
	m_viewer->setSecondImage( m_rightImg );
}

void MainWindow::on_actionReloadPlugins_triggered()
{
	reloadPlugins();
}

void MainWindow::reloadPlugins()
{

	try {
		PythonIntegration* python = PythonIntegration::instance();
		python->import_path( m_pluginDir.toStdString() );

		EmbeddedModule m( this );

		for ( auto a : m_pluginActions ) {
			delete a;
		}

		m_pluginActions.clear();

		for ( auto pluginModule : python->modules() ) {
			try {
				PlugIn* plugin = new PythonPlugIn( pluginModule, this );
				m_plugIns.append( plugin );
				auto actionsBoth =  plugin->actionsBoth();
				auto actionsLeft =  plugin->actionsLeft();
				auto actionsRight =  plugin->actionsRight();
				ui->menuPlugIns->addActions( actionsBoth );
				ui->menu_Left_Image->addActions( actionsLeft );
				ui->menu_Right_Image->addActions( actionsRight );

				for ( auto a : actionsBoth ) {
					m_pluginActions.push_back( a );
				}

				for ( auto a : actionsLeft ) {
					m_pluginActions.push_back( a );
				}

				for ( auto a : actionsRight ) {
					m_pluginActions.push_back( a );
				}

				qDebug() << "Added module" << QString::fromStdString( plugin->name() );
			} catch ( std::exception& e ) {
				QMessageBox msgBox;
				msgBox.setText( QString::fromStdString( e.what() ) );
				msgBox.exec();
				qDebug() << QString::fromStdString( e.what() );
			}
		}
	} catch ( ... ) {
		qDebug() << "Could not start internal python interpreter.";
	}

}

void MainWindow::setLeftImage( cv::Mat img, QString title )
{
	m_viewer->setFirstImage( img );
	m_leftImgPath = title;
	m_leftImg = img;
	updateLabels();
}

void MainWindow::setRightImage( py::array_t<float> img, QString title )
{
	m_rightStack = std::static_pointer_cast<FrameStack>( std::make_shared<NumpyStack>( img ) );

	if ( m_rightStack->hasFileOpen() ) {
		m_rightStack->goToFrame( m_leftStack->currentIdx() );
	}

	m_rightImg = m_rightStack->currentFrame();
	m_viewer->setSecondImage( m_rightImg );
	m_rightImgPath = title;
	updateLabels();
}
void MainWindow::setRightImage( cv::Mat img, QString title )
{
	m_viewer->setSecondImage( img );
	m_rightImgPath = title;
	m_rightImg = img;
	updateLabels();
}

void MainWindow::setLeftImage( py::array_t<float> img, QString title )
{
	m_leftStack = std::static_pointer_cast<FrameStack>( std::make_shared<NumpyStack>( img ) );

	if ( m_rightStack->hasFileOpen() ) {
		m_leftStack->goToFrame( m_rightStack->currentIdx() );
	}

	m_leftImg = m_leftStack->currentFrame();
	m_viewer->setFirstImage( m_leftImg );
	m_leftImgPath = title;
	updateLabels();
}
void MainWindow::on_actionShowPythonConsole_triggered()
{
	m_pythonConsole->setVisible( !m_pythonConsole->isVisible() );
	// m_pythonDockWidget->setVisible( !m_pythonDockWidget->isVisible() );
	// m_pythonConsole->show();

}