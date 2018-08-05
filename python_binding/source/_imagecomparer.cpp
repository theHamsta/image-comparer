#include "MainWindow.hpp"

#pragma push_macro("slots")
#undef slots

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#pragma pop_macro("slots")

#include <thread>

namespace py = pybind11;
using namespace pybind11::literals;

std::unique_ptr<QApplication> app;
int ONE_ARGUMENT_FOR_ARGC = 1;
char* qtApplicationName = ( char* )( "" );


PYBIND11_MODULE( _imagecomparer, m )
{
	app = std::make_unique<QApplication>( ONE_ARGUMENT_FOR_ARGC, &qtApplicationName );

	py::class_<ImageComparer::MainWindow> self( m, "ImageComparer" );
	self.def( py::init<> () )
	.def( "setLeftImage", [&]( ImageComparer::MainWindow & self, py::array_t<float>& array, std::string title ) {


		if ( array.ndim() == 1 ) {
			// return std::make_unique<NRRD::Image<float>> ( array.shape()[0], 1, 1, const_cast<float*>( array.data() ) );
		} else if ( array.ndim() == 2 ) {
			cv::Mat mat( array.shape()[0], array.shape()[1], CV_32FC1, array.mutable_unchecked<2>().mutable_data( 0, 0 ) );
			self.setLeftImage( mat, QString::fromStdString( title ) );
		}

		if ( array.ndim() == 3 ) {
			self.setLeftImage( array, QString::fromStdString( title ) );
		}

		self.show();

		app->processEvents();

	}, "array"_a, "title"_a = "" )
	.def( "setRightImage", [&]( ImageComparer::MainWindow & self, py::array_t<float>& array, std::string title ) {


		if ( array.ndim() == 1 ) {
			// return std::make_unique<NRRD::Image<float>> ( array.shape()[0], 1, 1, const_cast<float*>( array.data() ) );
		} else if ( array.ndim() == 2 ) {
			cv::Mat mat( array.shape()[0], array.shape()[1], CV_32FC1, array.mutable_unchecked<2>().mutable_data( 0, 0 ) );
			self.setRightImage( mat, QString::fromStdString( title ) );
		}

		if ( array.ndim() == 3 ) {
			self.setRightImage( array, QString::fromStdString( title ) );
		}

		self.show();
		app->processEvents();

	}, "array"_a, "title"_a = "" )
	.def( "show", [&]( ImageComparer::MainWindow & self, bool blockingCall ) {


		// } );
		// t.detach();
		// QMainWindow m;
		// m.show();

		// app->exec();
		// app->processEvents();
		self.show();
		app->processEvents();

		while ( self.isVisible() && blockingCall ) {
			app->processEvents();
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}

	},  py::arg( "blocking_call" ) = true )
	.def( "isVisible", &ImageComparer::MainWindow::isVisible )
	.def( "getLeftImage", [&]( ImageComparer::MainWindow & imagecomparer ) {
		return imagecomparer.leftImage();
	}  )
	.def( "getRightImage", [&]( ImageComparer::MainWindow & imagecomparer ) {
		return imagecomparer.rightImage();
	}  ) ;

	py::class_<cv::Mat>( m, "cvMat", py::buffer_protocol() )
	.def_buffer( []( cv::Mat & mat ) -> py::buffer_info {
		return py::buffer_info(
			mat.data,                               /* Pointer to buffer */
			mat.elemSize(),                        /* Size of one scalar */
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
			sizeof( float ) * mat.rows * mat.channels(),           /* Strides (in bytes) for each index */
			sizeof( float ) * mat.channels(),           /* Strides (in bytes) for each index */
			sizeof( float )
		}
		);
	} );




}


