#ifndef PYTHONFILEOPEN_HPP
#define PYTHONFILEOPEN_HPP


#include "PythonIntegration.hpp"
#pragma push_macro("slots")
#undef slots
#include "pybind11/numpy.h"
#pragma pop_macro("slots")

py::array_t<float> openFilePython( std::string filename );

#endif /* PYTHONFILEOPENER_HPP */
