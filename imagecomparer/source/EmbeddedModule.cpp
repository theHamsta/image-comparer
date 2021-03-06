#include "EmbeddedModule.hpp"
#pragma push_macro("slots")
#undef slots
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#pragma pop_macro("slots")
#include <QDebug>



namespace py = pybind11;
using namespace pybind11::literals;

ImageComparer::MainWindow* g_imagecomparer;


// modified structure necessary as pybind does not allow embedded module in python module
struct embedded_module_new {
#if PY_MAJOR_VERSION >= 3
	using init_t = PyObject * ( * )();
#else
	using init_t = void ( * )();
#endif
	embedded_module_new( const char* name, init_t init )
	{
		if ( !Py_IsInitialized() ) {

			PyImport_AppendInittab( name, init );

		}
	}
};

#undef PYBIND11_EMBEDDED_MODULE
#define PYBIND11_EMBEDDED_MODULE(name, variable)                              \
	static void PYBIND11_CONCAT(pybind11_init_, name)(pybind11::module &);    \
	static PyObject PYBIND11_CONCAT(*pybind11_init_wrapper_, name)() {        \
		auto m = pybind11::module(PYBIND11_TOSTRING(name));                   \
		try {                                                                 \
			PYBIND11_CONCAT(pybind11_init_, name)(m);                         \
			return m.ptr();                                                   \
		} catch (pybind11::error_already_set &e) {                            \
			PyErr_SetString(PyExc_ImportError, e.what());                     \
			return nullptr;                                                   \
		} catch (const std::exception &e) {                                   \
			PyErr_SetString(PyExc_ImportError, e.what());                     \
			return nullptr;                                                   \
		}                                                                     \
	}                                                                         \
	PYBIND11_EMBEDDED_MODULE_IMPL(name)                                       \
	embedded_module_new name(PYBIND11_TOSTRING(name),           \
							 PYBIND11_CONCAT(pybind11_init_impl_, name));   \
	void PYBIND11_CONCAT(pybind11_init_, name)(pybind11::module &variable)




PYBIND11_EMBEDDED_MODULE( imagecomparer, m )
{
	py::class_<cv::Mat>( m, "cvMat", py::buffer_protocol() )
	.def_buffer( []( cv::Mat & mat ) -> py::buffer_info {
		return py::buffer_info(
			mat.data,                               /* Pointer to buffer */
			mat.elemSize1(),                        /* Size of one scalar */
			[&]()
		{
			if ( mat.type() == CV_32FC1 ) {
				return py::format_descriptor<float>::format();
			} else if ( mat.type() == CV_8UC1 || mat.type() == CV_8UC3 ) {
				return py::format_descriptor<uchar>::format();
			} else {
				return py::format_descriptor<uchar>::format();

			}
		}(), /* Python struct-style format descriptor */
		3,                                      /* Number of dimensions */
		{ mat.rows, mat.cols, mat.channels() },                /* Buffer dimensions */
		{
			mat.step[0],           /* Strides (in bytes) for each index */
			mat.elemSize1() * mat.channels(),           /* Strides (in bytes) for each index */
			mat.elemSize1()
		}
		);
	} );

	m.def( "setLeftImage", [&]( py::array_t<float>& array, std::string title ) {


		if ( array.ndim() == 1 ) {
			// return std::make_unique<NRRD::Image<float>> ( array.shape()[0], 1, 1, const_cast<float*>( array.data() ) );
		} else if ( array.ndim() == 2 ) {
			cv::Mat mat( array.shape()[0], array.shape()[1], CV_32FC1, array.mutable_unchecked<2>().mutable_data( 0, 0 ) );
			g_imagecomparer->setLeftImage( mat.clone(), QString::fromStdString( title ) );
		}

		if ( array.ndim() == 3 ) {
			g_imagecomparer->setLeftImage( array, QString::fromStdString( title ) );
		}



	}, "array"_a, "title"_a = "" )
	.def( "setRightImage", [&](  py::array_t<float>& array, std::string title ) {


		if ( array.ndim() == 1 ) {
			// return std::make_unique<NRRD::Image<float>> ( array.shape()[0], 1, 1, const_cast<float*>( array.data() ) );
		} else if ( array.ndim() == 2 ) {
			cv::Mat mat( array.shape()[0], array.shape()[1], CV_32FC1, array.mutable_unchecked<2>().mutable_data( 0, 0 ) );
			g_imagecomparer->setRightImage( mat.clone(), QString::fromStdString( title ) );
		}

		if ( array.ndim() == 3 ) {
			g_imagecomparer->setRightImage( array, QString::fromStdString( title ) );
		}


	}, "array"_a, "title"_a = "" )
	.def( "getLeftImage", [&]() {
		return g_imagecomparer->leftImage();
	}  )
	.def( "getRightImage", [&]() {
		return g_imagecomparer->rightImage();
	}  ) ;
}
EmbeddedModule::EmbeddedModule( ImageComparer::MainWindow* imagecomparer ): m_imagecomparer( imagecomparer )
{
	g_imagecomparer = m_imagecomparer;

	try {

		py::module::import( "imagecomparer" );
		py::exec( R"(
import numpy as np
import imagecomparer
import imagecomparer as ic
import matplotlib.pyplot as plt
)");
	}
	catch (const std::exception& e)
	{
		qDebug()<< QString::fromStdString(e.what());
	}
}

EmbeddedModule::~EmbeddedModule()
{
}