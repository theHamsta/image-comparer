
/**
 * \file   PlugIn.hpp
 * \brief  Defines interface for imagecomparer plugins
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   07/10/2017
 */

#ifndef PYTHONPLUGIN_HPP
#define PYTHONPLUGIN_HPP

#include "PlugIn.hpp"
#include <boost/python.hpp>

namespace ImageComparer
{
class MainWindow;
}

class PythonPlugIn : public PlugIn
{
public:
	PythonPlugIn( boost::python::object& plugInModule, ImageComparer::MainWindow* imagecomparer );
	PythonPlugIn( PythonPlugIn && ) = default;
	PythonPlugIn( const PythonPlugIn& ) = default;
	PythonPlugIn& operator=( PythonPlugIn && ) = default;

	virtual ~PythonPlugIn();

	virtual const std::string name() const override;
	virtual const std::string description() const override;
	
	virtual QList<QAction*> actionsLeft() override; 
	virtual QList<QAction*> actionsRight() override;
	virtual QList<QAction*> actionsBoth() override;
	virtual QList<QAction*> actionsNotInMenu() { return QList<QAction*>(); };
	virtual std::function<QList<QAction*>( const QString& )> dynamicActions()
		{ return []( const QString& ) { return QList<QAction*>(); }; }



private:
	boost::python::object m_pluginModule;
	std::string m_pluginName;
	std::string m_pluginDescription;
	std::string m_pluginShortcut;
	std::string m_pluginShortcutLeft;
	std::string m_pluginShortcutRight;

	bool m_singleImageProcessing = false;
	bool m_doubleImageProcessing = false;
};


#endif // PYTHONPLUGIN_HPP
