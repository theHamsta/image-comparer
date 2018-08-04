#ifndef EMBEDDEDMODULE_HPP
#define EMBEDDEDMODULE_HPP

#include "MainWindow.hpp"

class EmbeddedModule
{
	public:
		EmbeddedModule( ImageComparer::MainWindow* imagecomparer );
		EmbeddedModule( EmbeddedModule&& ) = default;
		EmbeddedModule( const EmbeddedModule& ) = default;
		EmbeddedModule& operator=( EmbeddedModule&& ) = default;
		EmbeddedModule& operator=( const EmbeddedModule& ) = default;
		~EmbeddedModule();

	private:
		ImageComparer::MainWindow* m_imagecomparer;
};

#endif /* EMBEDDEDMODULE_HPP */
