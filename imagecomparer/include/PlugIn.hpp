/**
 * \file   PlugIn.hpp
 * \brief  Defines interface for imagecomparer plugins
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   07/10/2017
 */

#ifndef PLUGIN_HPP
#define PLUGIN_HPP


#include <string>
#include <QList>
#include <QAction>
#include <QThread>
#include <functional>

namespace ImageComparer
{
	class MainWindow;
}

class PlugIn
{
	public:
		PlugIn( ImageComparer::MainWindow* imagecomparer );
		PlugIn( PlugIn&& ) = default;
		PlugIn( const PlugIn& ) = default;
		PlugIn& operator=( PlugIn&& ) = default;

		virtual ~PlugIn();

		virtual const std::string name() const = 0;
		virtual const std::string description() const = 0;

		virtual QList<QAction*> actionsLeft() { return QList<QAction*>(); };
		virtual QList<QAction*> actionsRight() { return QList<QAction*>(); };
		virtual QList<QAction*> actionsBoth() { return QList<QAction*>(); };
		virtual QList<QAction*> actionsNotInMenu() { return QList<QAction*>(); };
		virtual std::function<QList<QAction*>( const QString& )> dynamicActions()
		{ return []( const QString& ) { return QList<QAction*>(); }; }



	protected:
		ImageComparer::MainWindow* m_imagecomparer = nullptr;
		QThread* m_pluginThread = nullptr;

};


#endif // PLUGIN_HPP
