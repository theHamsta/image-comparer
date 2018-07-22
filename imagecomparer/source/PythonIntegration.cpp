#include "PythonIntegration.hpp"
#include <iostream>

using namespace boost::python;
using namespace boost::filesystem;

PythonIntegration* PythonIntegration::s_instance = nullptr;
std::mutex g_mutex;

PythonIntegration::PythonIntegration()
{
	Py_Initialize();


	object main_module = import( "__main__" );
	m_mainNamespace = main_module.attr( "__dict__" );
}

void PythonIntegration::import_path( const boost::filesystem::path& path )
{
	if ( !is_directory( path ) ) {
		std::runtime_error( "Cannot access \"" + path.string() + "\"" );
	}

	directory_iterator eod;

	for ( directory_iterator it( path ); it != eod; ++it ) {
		if ( is_regular_file( it->path() ) && it->path().extension().string() == ".py" ) {
			try {
				import_module( it->path() );
			}
			catch ( std::exception& e ) {
				std::cerr << e.what() << std::endl;
			}
		}
	}
}


void PythonIntegration::import_module( const boost::filesystem::path& pythonFile )
{
	if ( !is_regular_file( pythonFile ) ) {
		std::runtime_error( "Cannot access \"" + pythonFile.string() + "\"" );
	}

	std::string moduleName = pythonFile.stem().string();

	try {
		exec( str( "import sys\r\nsys.path.append('" + pythonFile.parent_path().string() + "')\r\n" ), m_mainNamespace );

		object module = import( str( moduleName ) );
// 		m_objects.push_back(extract<object>(m_mainNamespace.attr("__dict__.['obj']")));
		m_modules.push_back( module );
	}
	catch ( error_already_set ) {
		throw std::runtime_error( "Failed to import module \"" + moduleName + "\" (" + pythonFile.parent_path().string() + ")" );
	}
}

void PythonIntegration::exec_commands( const std::string& commands )
{
	exec( str( commands.c_str() ), m_mainNamespace );
}
std::mutex& PythonIntegration::mutex()
{
	return g_mutex;
}
