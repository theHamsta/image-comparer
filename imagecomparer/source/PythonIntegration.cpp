#include "PythonIntegration.hpp"
#include <iostream>

#include <boost/filesystem.hpp>
#include "pybind11/eval.h"
#include <QDebug>

using namespace py::literals;
using namespace boost::filesystem;

PythonIntegration* PythonIntegration::s_instance = nullptr;
std::mutex g_mutex;

PythonIntegration::PythonIntegration()
{
	//Py_Initialize();


	//object main_module = import( "__main__" );
	//m_mainNamespace = main_module.attr( "__dict__" );
}

void PythonIntegration::import_path( const boost::filesystem::path& path )
{
	if ( !is_directory( path ) ) {
		std::runtime_error( "Cannot access \"" + path.string() + "\"" );
	}

	auto locals = py::dict( "path"_a = path.string() );

	py::exec( R"(
import sys
path =  "{path}".format( **locals() )
sys.path.append( path )
print("Added %s to Python path!"%path)
				 )", py::globals(), locals );

	directory_iterator eod;
	std::string errorMessage;

	for ( directory_iterator it( path ); it != eod; ++it ) {
		if ( is_regular_file( it->path() ) && it->path().extension().string() == ".py" ) {
			try {
				import_module( it->path() );
			} catch ( std::exception& e ) {
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


	py::module module = py::module::import( moduleName.c_str() );
	m_modules.push_back( module );

}

void PythonIntegration::exec_commands( const std::string& commands )
{
	//exec( str( commands.c_str() ), m_mainNamespace );
}
std::mutex& PythonIntegration::mutex()
{
	return g_mutex;
}
