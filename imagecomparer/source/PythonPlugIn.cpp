#include "PythonPlugIn.hpp"
#include "PythonIntegration.hpp"
#include "MainWindow.hpp"

#include <thread>


PythonPlugIn::PythonPlugIn( py::module& module,  ImageComparer::MainWindow* imagecomparer ) : PlugIn( imagecomparer ), m_pluginModule( module )
{
	try {
		m_pluginName = m_pluginModule.attr( "name" ).cast<std::string>();
		// m_pluginName = m_pluginModule.attr( "name" ).cast<std::string>();
		m_pluginShortcut = m_pluginModule.attr( "shortcut" ).cast<std::string>();
		m_pluginShortcutLeft = m_pluginModule.attr( "shortcut_left" ).cast<std::string>();
		m_pluginShortcutRight = m_pluginModule.attr( "shortcut_right" ).cast<std::string>();
		m_singleImageProcessing = m_pluginModule.attr( "enable_processing_of_one_image" ).cast<bool>();
		m_doubleImageProcessing = m_pluginModule.attr( "enable_processing_of_two_images" ).cast<bool>();
	} catch ( std::exception& e ) {
		std::cerr << e.what() << std::endl;

	}
}

PythonPlugIn::~PythonPlugIn()
{
}

const std::string PythonPlugIn::name() const
{
	return m_pluginName;
}

const std::string PythonPlugIn::description() const
{
	return m_pluginDescription;
}

QList<QAction*> PythonPlugIn::actionsBoth()
{
	if ( m_doubleImageProcessing ) {
		QAction* action = new QAction( QString::fromStdString( m_pluginName ), m_imagecomparer );
		action->setShortcut( QKeySequence( QString::fromStdString( m_pluginShortcut ) ) );
		action->setIcon( QIcon::fromTheme( "plugins", QIcon( ":icons/plugins.svg" ) ) );
		m_imagecomparer->connect( action, &QAction::triggered, [&]() {
			std::thread thread( [&]() {
				if ( PythonIntegration::mutex().try_lock() ) {
					try {
						m_pluginModule.attr( "process" )(
							m_imagecomparer->leftImageFileInfo().absoluteFilePath().toStdString().c_str(),
							m_imagecomparer->rightImageFileInfo().absoluteFilePath().toStdString().c_str() );
					} catch ( std::exception& e ) {
						std::cout << m_imagecomparer->leftImageFileInfo().absoluteFilePath().toStdString().c_str() << std::endl;
						qDebug() << e.what();
					}

					PythonIntegration::mutex().unlock();
				}

			} );
			thread.detach();
		} );
		return {action};
	} else {

		return {};
	}
}

QList<QAction*> PythonPlugIn::actionsLeft()
{

	if ( m_singleImageProcessing ) {
		QAction* action = new QAction( QString::fromStdString( "Left: " + m_pluginName ), m_imagecomparer );
		action->setShortcut( QKeySequence( QString::fromStdString( m_pluginShortcutLeft ) ) );
		action->setIcon( QIcon::fromTheme( "plugins", QIcon( ":icons/plugins.svg" ) ) );
		m_imagecomparer->connect( action, &QAction::triggered, [&]() {
			std::thread thread( [&]() {
				if ( PythonIntegration::mutex().try_lock() ) {
					try {
						m_pluginModule.attr( "process" )(
							m_imagecomparer->leftImageFileInfo().absoluteFilePath().toStdString().c_str() );
					} catch ( std::exception& e ) {
						qDebug() << e.what();
					}

					PythonIntegration::mutex().unlock();
				}

			} );
			thread.detach();
		} );

		return {action};
	} else {
		return {};
	}
}

QList<QAction*> PythonPlugIn::actionsRight()
{
	if ( m_singleImageProcessing ) {
		QAction* action = new QAction( QString::fromStdString( "Right: " + m_pluginName ), m_imagecomparer );
		action->setShortcut( QKeySequence( QString::fromStdString( m_pluginShortcutRight ) ) );
		action->setIcon( QIcon::fromTheme( "plugins", QIcon( ":icons/plugins.svg" ) ) );
		m_imagecomparer->connect( action, &QAction::triggered, [&]() {
			std::thread thread( [&]() {
				if ( PythonIntegration::mutex().try_lock() ) {
					try {
						m_pluginModule.attr( "process" )(
							m_imagecomparer->rightImageFileInfo().absoluteFilePath().toStdString().c_str() );
					} catch ( std::exception& e ) {
						qDebug() << e.what();
					}

					PythonIntegration::mutex().unlock();
				}

			} );
			thread.detach();
		} );

		return {action};
	} else {
		return {};
	}
}

// PythonPlugIn( PythonPlugIn && ) = default;
// PythonPlugIn( const PythonPlugIn& ) = default;
// PythonPlugIn& operator=( PythonPlugIn && ) = default;
//
// virtual ~PythonPlugIn();
//
// virtual const std::string name();
// virtual const std::string description();
//
// virtual QList<QAction*> actionsLeft();
// virtual QList<QAction*> actionsRight();
// virtual QList<QAction*> actionsBoth();
