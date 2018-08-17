#include "TiffStackReader.hpp"
#include <exception>

#include <iostream>

#include <assert.h>


TiffStackReader::TiffStackReader( std::string fileName, bool keepAllFramesInRam )
{
	m_keepAllFramesInRam = keepAllFramesInRam;

	if ( !fileName.empty() ) {
		openTiff( fileName );
	} else {
		assert( !keepAllFramesInRam && "Can only be set if filename is not empty" );
	}
}

TiffStackReader::~TiffStackReader()
{
	closeTiff();
}

void TiffStackReader::openTiff( std::string fileName, bool keepBrightnessAndContrastSettings )
{
	closeTiff( keepBrightnessAndContrastSettings );
	m_fileName = fileName;
	m_file = TinyTIFFReader_open( fileName.c_str() );

	if ( !m_file ) {
		throw std::runtime_error( "Could not open \"" + fileName + "\"" );
	}

	m_numFrames = TinyTIFFReader_countFrames( m_file );

	if ( m_keepAllFramesInRam ) {
		m_allFrames.resize( m_numFrames );

		for ( cv::Mat& mat : m_allFrames ) {
			readCurrentFrame( &mat );

			if ( TinyTIFFReader_hasNext( m_file ) ) {
				if ( !TinyTIFFReader_readNext( m_file ) ) {
					m_isOk = false;
					throw std::runtime_error( "Could not open \"" + fileName + "\"" );
				}
			}
		}

	} else {
		readCurrentFrame();
	}
}


void TiffStackReader::closeTiff( bool keepBrightnessAndContrastSettings )
{
	if ( m_file ) {
		TinyTIFFReader_close( m_file );
		m_file = nullptr;
	}

	free( m_dataCurrrentFrame );
	m_dataCurrrentFrame = nullptr;

	for ( void* v :  m_memAllFrames ) {
		free( v );
	}

	m_memAllFrames.clear();
	m_rtnModified.release();

	m_bufferSizeCurrentFrame  = 0;
	m_isOk = true;

	if ( !keepBrightnessAndContrastSettings ) {
		m_min = 0.f;
		m_max = 1.f;
	}

	m_currentIdx = 0;
	m_numFrames = 0;
}


bool TiffStackReader::isOk()
{
	return !TinyTIFFReader_wasError( m_file ) && m_isOk;
}


bool TiffStackReader::hasErrors()
{
	bool hasErrors = TinyTIFFReader_wasError( m_file );

	if ( hasErrors ) {
		std::cerr << "Error: " << TinyTIFFReader_getLastError( m_file ) << std::endl;
	}

	return hasErrors;
}

void TiffStackReader::readCurrentFrame( cv::Mat* dstMat )
{
	if ( dstMat ) {
		assert( m_keepAllFramesInRam );
	} else {
		assert( !m_keepAllFramesInRam );
		dstMat = &m_mat;
	}

	uint32_t width = TinyTIFFReader_getWidth( m_file );
	uint32_t height = TinyTIFFReader_getHeight( m_file );

	if ( width > 0 && height > 0 ) {
		m_isOk = true;
	} else {
		m_isOk = false;
	}

	if ( m_isOk ) {

		int sampleFormat = TinyTIFFReader_getSampleFormat( m_file );
		int numChannels = TinyTIFFReader_getSamplesPerPixel( m_file );
		int bitsPerSample = TinyTIFFReader_getBitsPerSample( m_file );

		size_t newBufferSize = 0;
		int cvType;

		if ( sampleFormat == TINYTIFFREADER_SAMPLEFORMAT_FLOAT && bitsPerSample == 32 ) {
			newBufferSize = numChannels * width * height * sizeof( float );
			cvType = CV_MAKETYPE( CV_32F, numChannels );
		} else if ( sampleFormat ==  TINYTIFFREADER_SAMPLEFORMAT_UINT ) {
			if ( bitsPerSample == 16 ) {
				newBufferSize = numChannels * width * height * ( 16 / 8 );
				cvType = CV_MAKETYPE( CV_16U, numChannels );
			} else if ( bitsPerSample == 8 ) {
				newBufferSize = numChannels * width * height * ( 8 / 8 );
				cvType = CV_MAKETYPE( CV_8U, numChannels );
			}
		}

		if ( !newBufferSize ) {
			m_isOk = false;
			*dstMat = cv::Mat();
			return;
		}

		void* mem = nullptr;

		if ( m_keepAllFramesInRam ) {
			mem = malloc( newBufferSize );
			m_memAllFrames.push_back( mem );
		} else {

			if ( newBufferSize > m_bufferSizeCurrentFrame ) {
				free( m_dataCurrrentFrame );
				m_dataCurrrentFrame = malloc( newBufferSize );
				m_bufferSizeCurrentFrame = newBufferSize;
			}

			mem = m_dataCurrrentFrame;
		}

		m_isOk = TinyTIFFReader_getSampleData( m_file, mem, 0 );

		if ( hasErrors() ) {
			*dstMat = cv::Mat();
			return;
		}

		if ( mem ) {
			*dstMat = cv::Mat( height, width, cvType, mem );
		} else {

			m_isOk = false;
			throw std::bad_alloc();
		}
	} else {
		*dstMat = cv::Mat();
	}
}

cv::Mat TiffStackReader::currentFrame()
{
	cv::Mat rtn;

	if ( m_keepAllFramesInRam ) {
		rtn = m_allFrames[m_currentIdx];
	} else {
		rtn = m_mat;
	}

	if ( m_min == 0.f && m_max == 1.f ) {
		return rtn;
	} else {
		float diff = m_max - m_min;

		m_rtnModified = ( rtn - m_min ) / diff;

		return m_rtnModified;
	}
}

uint TiffStackReader::currentIdx() const
{
	return m_currentIdx;
}

void TiffStackReader::goToFrame( int idx )
{
	if ( idx < 0 ) {
		idx = 0;
	}

	if ( m_keepAllFramesInRam ) {
		assert( idx < m_numFrames );
		m_currentIdx = idx;
		return;
	}

	if ( idx == m_currentIdx ) {
		return;
	} else {
		if ( idx < m_currentIdx )  {
			TinyTIFFReader_close( m_file );

			if ( hasErrors() ) {
				return;
			}

			m_file = TinyTIFFReader_open( m_fileName.c_str() );

			if ( hasErrors() ) {
				return;
			}

			m_currentIdx = 0;
		}

		for ( ; m_currentIdx < idx; ++m_currentIdx ) {
			if ( !TinyTIFFReader_readNext( m_file ) ) {
				m_isOk = false; return;
			}
		}

		readCurrentFrame();
	}
}

void TiffStackReader::nextFrame()
{
	assert( hasMoreFrames() );
	m_currentIdx++;

	if ( !m_keepAllFramesInRam ) {

		if ( !TinyTIFFReader_readNext( m_file ) ) {
			m_isOk = false;
			return;
		}

		readCurrentFrame();
	}
}

void TiffStackReader::previousFrame()
{
	assert( m_currentIdx != 0 );
	goToFrame( m_currentIdx - 1 );
}


uint TiffStackReader::numFrames() const
{
	return m_numFrames;
}



bool TiffStackReader::hasMoreFrames()
{
	return m_currentIdx + 1 < m_numFrames;
}

void TiffStackReader::adjustBrightness()
{
	double min,  max;

	if ( m_keepAllFramesInRam ) {
		cv::minMaxLoc( m_allFrames[m_currentIdx],  &min,  &max );
	} else {
		cv::minMaxLoc( m_mat,  &min,  &max );
	}

	if ( max == min ) {
		max = min + 1.f;
	}

	m_min = min;
	m_max = max;
}

void TiffStackReader::adjustBrightness( float min, float max )
{
	m_min = min;
	m_max = max;
}

void TiffStackReader::setKeepAllFramesInRam( bool inRam )
{
	m_keepAllFramesInRam = inRam;

	if ( m_file ) {
		uint curIdx = m_currentIdx;
		float min = m_min;
		float max = m_max;
		openTiff( m_fileName );
		goToFrame( curIdx );
		m_min = min;
		m_max = max;
	}
}



