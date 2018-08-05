#ifndef NUMPYSTACK_HPP
#define NUMPYSTACK_HPP

#include <string>

#include <opencv2/opencv.hpp>


#pragma push_macro("slots")
#undef slots

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#pragma pop_macro("slots")
#include "NumpyStack.hpp"
#include "FrameStack.hpp"
#include <QString>


namespace py = pybind11;


class NumpyStack : public FrameStack
{
	public:
		NumpyStack( const py::array_t<float>& array ) ;
		virtual ~NumpyStack() {}

		virtual void release( bool keepBrightnessAndContrastSettings = false ) override;
		virtual void previousFrame() override;
		virtual void nextFrame() override;
		virtual void goToFrame( uint idx ) override;
		virtual bool hasMoreFrames() override;
		virtual bool isOk() override;
		virtual void adjustBrightness() override;
		virtual void adjustBrightness( float min, float max ) override;
		virtual inline bool hasFileOpen() const override;

		virtual cv::Mat currentFrame() override;
		virtual uint numFrames() const override;
		virtual uint currentIdx() const override;
		virtual bool keepAllFramesInRam() const override;

		virtual void setKeepAllFramesInRam( bool inRam ) override;

	private:
		py::array_t<float> m_array;
		int m_currentIdx = 0;

};

#endif /* NUMPYSTACK_HPP */
