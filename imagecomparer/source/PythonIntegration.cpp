#include "PythonIntegration.hpp"
#include <iostream>

#include <boost/filesystem.hpp>
#pragma push_macro("slots")
#undef slots
#include "pybind11/eval.h"
#pragma pop_macro("slots")
#include <QDebug>
#include <thread>

namespace py = pybind11;
using namespace pybind11::literals;

using namespace py::literals;
using namespace boost::filesystem;

PythonIntegration* PythonIntegration::s_instance = nullptr;
std::mutex g_mutex;



PythonIntegration::PythonIntegration()
{
	try {
		m_interpreter = std::make_unique<py::scoped_interpreter>();
	} catch ( const std::exception& ) {
		m_failedToStartEmbeddedInterpreter = true;
	}
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

std::mutex& PythonIntegration::mutex()
{
	return g_mutex;
}
