#include "PythonPlugIn.hpp"
#include "PythonIntegration.hpp"
#include "MainWindow.hpp"

#include <thread>

namespace py = boost::python;

PythonPlugIn::PythonPlugIn(boost::python::object &module, ImageComparer::MainWindow *imagecomparer) : PlugIn(imagecomparer),
																									  m_pluginModule(module)
{
	try
	{
		m_pluginName = py::extract<char const *>(m_pluginModule.attr("name"));
		m_pluginShortcut = py::extract<char const *>(m_pluginModule.attr("shortcut"));
		m_pluginShortcutLeft = py::extract<char const *>(m_pluginModule.attr("shortcut_left"));
		m_pluginShortcutRight = py::extract<char const *>(m_pluginModule.attr("shortcut_right"));
		
		m_singleImageProcessing = py::extract<bool>(m_pluginModule.attr("enable_processing_of_one_image"));
		m_doubleImageProcessing = py::extract<bool>(m_pluginModule.attr("enable_processing_of_two_images"));
	}
	catch (std::exception &e)
	{
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

QList<QAction *> PythonPlugIn::actionsBoth()
{
	if (m_doubleImageProcessing)
	{
		QAction *action = new QAction(QString::fromStdString(m_pluginName), m_imagecomparer);
		action->setShortcut(QKeySequence(QString::fromStdString(m_pluginShortcut)));
		action->setIcon(QIcon::fromTheme("plugins", QIcon(":icons/plugins.svg")));
		m_imagecomparer->connect(action, &QAction::triggered, [&]() {
			std::thread thread([&]() {
				if (PythonIntegration::mutex().try_lock())
				{
					try
					{
						m_pluginModule.attr("__dict__")["process"](
							m_imagecomparer->leftImageFileInfo().absoluteFilePath().toStdString().c_str(),
							m_imagecomparer->rightImageFileInfo().absoluteFilePath().toStdString().c_str());
					}
					catch (std::exception &e)
					{
						std::cout << m_imagecomparer->leftImageFileInfo().absoluteFilePath().toStdString().c_str() << std::endl;
						qDebug() << e.what();
					}

					PythonIntegration::mutex().unlock();
				}

			});
			thread.detach();
		});
		return {action};
	}
	else
	{

		return {};
	}
}

QList<QAction *> PythonPlugIn::actionsLeft()
{

	if (m_singleImageProcessing)
	{
		QAction *action = new QAction(QString::fromStdString("Left: " + m_pluginName), m_imagecomparer);
		action->setShortcut(QKeySequence(QString::fromStdString(m_pluginShortcutLeft)));
		action->setIcon(QIcon::fromTheme("plugins", QIcon(":icons/plugins.svg")));
		m_imagecomparer->connect(action, &QAction::triggered, [&]() {
			std::thread thread([&]() {
				if (PythonIntegration::mutex().try_lock())
				{
					try
					{
						m_pluginModule.attr("__dict__")["process"](
							m_imagecomparer->leftImageFileInfo().absoluteFilePath().toStdString().c_str());
					}
					catch (std::exception &e)
					{
						qDebug() << e.what();
					}

					PythonIntegration::mutex().unlock();
				}

			});
			thread.detach();
		});

		return {action};
	}
	else
	{
		return {};
	}
}

QList<QAction *> PythonPlugIn::actionsRight()
{
	if (m_singleImageProcessing)
	{
		QAction *action = new QAction(QString::fromStdString("Right: " + m_pluginName), m_imagecomparer);
		action->setShortcut(QKeySequence(QString::fromStdString(m_pluginShortcutRight)));
		action->setIcon(QIcon::fromTheme("plugins", QIcon(":icons/plugins.svg")));
		m_imagecomparer->connect(action, &QAction::triggered, [&]() {
			std::thread thread([&]() {
				if (PythonIntegration::mutex().try_lock())
				{
					try
					{
						m_pluginModule.attr("__dict__")["process"](
							m_imagecomparer->rightImageFileInfo().absoluteFilePath().toStdString().c_str());
					}
					catch (std::exception &e)
					{
						qDebug() << e.what();
					}

					PythonIntegration::mutex().unlock();
				}

			});
			thread.detach();
		});

		return {action};
	}
	else
	{
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
