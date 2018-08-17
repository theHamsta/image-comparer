#include "NumpyStack.hpp"
#include <algorithm>

NumpyStack::NumpyStack ( const py::array_t<float>& array ): m_array( array ) {}

void NumpyStack::release( bool /*keepBrightnessAndContrastSettings*/  ) {m_array.release();}
void  NumpyStack::previousFrame() {if ( m_currentIdx > 0 ) m_currentIdx--;}
void  NumpyStack::nextFrame() {if ( hasMoreFrames() ) m_currentIdx++;}
void  NumpyStack::goToFrame( int idx ) {  m_currentIdx = std::min( std::max( idx, 0 ), static_cast<int>( numFrames() ) - 1 );}
bool  NumpyStack::hasMoreFrames() {return m_currentIdx < numFrames() - 1;}
bool  NumpyStack::isOk() {return true;}
void  NumpyStack::adjustBrightness() {}
void  NumpyStack::adjustBrightness( float min, float max ) {}
inline bool  NumpyStack::hasFileOpen() const {return numFrames();}

cv::Mat  NumpyStack::currentFrame()
{

	if ( m_array.ndim() == 3 ) {
		cv::Mat mat( m_array.shape()[1], m_array.shape()[2], CV_32FC1, m_array.mutable_unchecked<3>().mutable_data( m_currentIdx, 0, 0 ) );
		return mat;
	} else {
		cv::Mat mat( m_array.shape()[0], m_array.shape()[1], CV_32FC1, m_array.mutable_unchecked<2>().mutable_data( 0, 0 ) );
		return mat;

	}
}
uint  NumpyStack::numFrames() const {return m_array.ndim() == 3 ? m_array.shape()[0] : 1;}
uint  NumpyStack::currentIdx()  const {return m_currentIdx;}
bool  NumpyStack::keepAllFramesInRam() const {return true;}

void NumpyStack::setKeepAllFramesInRam( bool inRam ) {}