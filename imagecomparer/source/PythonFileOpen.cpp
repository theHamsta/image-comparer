#include "PythonFileOpen.hpp"

py::array_t<float> openFilePython( std::string filename )
{
	auto openFileModule = py::module::import( "open_files" );
	py::array_t<float> rtn = py::cast<py::array_t<float>>( openFileModule.attr( "open_files" )( filename ) );



	return rtn;
}