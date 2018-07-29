/**
 * \file   imagecomparer/MainWindow.hpp
 * \brief  Defines class MainWindow
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   03.10.2016
 */

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "PlugIn.hpp"

#include <QMainWindow>
#include <QActionGroup>
#include <QFileInfo>
#include "MatcherBase.h"

#include <opencv2/opencv.hpp>

#include "AbstractDualViewer.hpp"
#include "TiffStackReader.hpp"

// TODO: recognize newly created files, ignore deleted files (observe directories)

class SimpleCommandPaletteWidget;

namespace Ui
{
	class MainWindow;
}

namespace ImageComparer
{
	enum ImageSide {
		RightImage,
		LeftImage
		// 		, BothImages
	};

	enum ViewMode {
		Unset,
		SplitViewMode,
		BlendViewMode,
		DiffViewMode,
		FlickerViewMode,
		numViewModes = FlickerViewMode
	};


	//TODO: include all icons in binary
	class MainWindow : public QMainWindow
	{
			Q_OBJECT

		public:
			explicit MainWindow( QWidget* parent = 0 );
			~MainWindow();

			void openFile( const QString& file, ImageComparer::ImageSide side, bool isReloadOpening = false );
			cv::Mat& leftImage() { return m_leftImg; }
			cv::Mat& rightImage() { return m_rightImg; }
			QFileInfo leftImageFileInfo() { return QFileInfo( m_leftImgPath ); }
			QFileInfo rightImageFileInfo() { return QFileInfo( m_rightImgPath ); }
			const QFileInfoList& leftImgFileList() const { return m_leftImgFileList; };
			const QFileInfoList& rightImgFileList() const { return m_rightImgFileList; }
			void updateDisplay();

		private slots:
			void on_actionSplitView_triggered();
			void on_actionDiffView_triggered();
			void on_actionFlickerView_triggered();
			void on_actionBlendView_triggered();

			void on_actionQuit_triggered();
			void on_actionReloadPlugins_triggered();

			void on_actionOpenImageLeft_triggered();
			void on_actionOpenImageRight_triggered();
			void on_actionCloseLeftImage_triggered();
			void on_actionCloseRightImage_triggered();
			//void on_actionSwapImagesLeftRight_triggered();
			void on_actionShowMenu_toggled( bool checked );

			void on_actionNextImageLeftAndRight_triggered();
			void on_actionPreviousImageLeftAndRight_triggered();
			void on_actionNextImageRightOnly_triggered();
			void on_actionNextImageLeftOnly_triggered();
			void on_actionPreviousImageRight_triggered();
			void on_actionPreviousImageLeft_triggered();


			void on_actionShowFullscreen_triggered();
			void on_actionSaveLeftImage_triggered();
			void on_actionSaveRightImage_triggered();
			void on_actionAbout_triggered();
			void on_actionKeepAllInRam_toggled( bool checked );
			void on_customContextMenuRequested( const QPoint& posOnCentralWidget );

			void on_directoryChanged( const QString& path );
			void on_fileChanged( const QString& path );

			void on_actionBilinearInterpolation_triggered();
			void on_actionAdjustBrightness_triggered();
			void on_actionGoToFrame_triggered();
			void on_actionShowInfoBox_toggled( bool checked );


		private:
			virtual void closeEvent( QCloseEvent* event ) override;
			virtual void keyReleaseEvent( QKeyEvent* event ) override;
			virtual void dropEvent( QDropEvent* event ) override;
			virtual void dragEnterEvent( QDragEnterEvent* event ) override;
			virtual void dragMoveEvent( QDragMoveEvent* event ) override;
			virtual void dragLeaveEvent( QDragLeaveEvent* event ) override;

			void readSettings();
			void reloadPlugins();
			void writeSetings();
			void setViewMode( ViewMode viewMode );
			void moveFilePointer( int delta, ImageSide side );
			void updateLabels();
			void updateFileInterators( ImageSide side );


			Ui::MainWindow* ui;
			AbstractDualViewer* m_viewer = nullptr;
			SimpleCommandPaletteWidget* m_commandPalette = nullptr;
			ViewMode m_viewMode = Unset;
			bool m_linearInterpolationDisplay = false;

			cv::Mat m_leftImg;
			cv::Mat m_rightImg;

			TiffStackReader m_leftStack;
			TiffStackReader m_rightStack;
			int m_lastLeftIdx = 0;
			int m_lastRightIdx = 0;

			QString m_leftImgPath;
			QString m_rightImgPath;
			QString m_infoText;

			QFileInfoList m_leftImgFileList;
			QFileInfoList m_rightImgFileList;
			QFileInfoList::iterator m_leftFileIterator;
			QFileInfoList::iterator m_rightFileIterator;
			MatcherBase m_rightFileMatcher;
			MatcherBase m_leftFileMatcher;
			QString m_pluginDir;
			QList<QAction*> m_pluginActions;

			QFileSystemWatcher m_fileSystemWatcher;
			QMimeDatabase m_mimeDb;
			QList<PlugIn*> m_plugIns;

			QActionGroup m_viewModeActionGroup;
	};
}
#endif // MAINWINDOW_HPP
