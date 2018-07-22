/**
 * \file   TiffStackReader.hpp
 * \brief  Defines class TiffStackReader
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   September 9th, 2017
 */

#ifndef TIFFSTACKREADER_HPP
#define TIFFSTACKREADER_HPP

#ifdef WITH_TIFF_FLOAT32

#include <string>

#include "tinytiffreader.h"
#include <opencv2/opencv.hpp>



/**
* @brief Class for reading Tiff image stacks
* and converting the frames into cv::Mats
*
*/
class TiffStackReader
{
public:
	TiffStackReader(std::string fileName = "", bool keepAllFramesInRam = false );
	TiffStackReader( const TiffStackReader& copy ) = delete;
	virtual ~TiffStackReader();

	void openTiff( std::string fileName, bool keepBrightnessAndContrastSettings = false );
	void closeTiff( bool keepBrightnessAndContrastSettings = false );
	void previousFrame();
	void nextFrame();
	void goToFrame(uint idx);
	bool hasMoreFrames();
	bool isOk();
	void adjustBrightness();
	void adjustBrightness(float min, float max);
	inline bool hasFileOpen() const { return m_file !=  nullptr; }

	cv::Mat currentFrame();
	uint numFrames();
	uint currentIdx();
	bool keepAllFramesInRam() const { return m_keepAllFramesInRam; }

	void setKeepAllFramesInRam( bool inRam );

private:
	void readCurrentFrame(cv::Mat* dstMat = nullptr);
	bool hasErrors();

	TinyTIFFReaderFile* m_file = nullptr;                      ///< Currently opened tiff file
	std::string m_fileName;                                    ///< Currently opened tiff file
	void* m_dataCurrrentFrame = nullptr;                       ///< Buffer for currently opened frame in stack
	uint m_bufferSizeCurrentFrame = 0;                         ///< Buffer size of currently opened frame in stack
	cv::Mat m_mat;                                             ///< cv::Mat wrapping currently opened frame of stack
	cv::cuda::GpuMat m_matGpu;                                 ///< cv::Mat wrapping currently opened frame of stack
	cv::Mat m_rtnModified;                                     ///< cv::Mat to save temporary results
	uint m_currentIdx = 0;                                     ///< Currently index in image stack
	uint m_numFrames = 0;                                      ///< Number of images in Tiff file
	bool m_isOk = true;                                        ///< Number of images in Tiff file
	float m_min = 0.f;                                         ///< Minimum of dynamic range (in case of float32 images)
	float m_max = 1.f;                                         ///< Maximum of dynamic range (in case of float32 images)
	bool m_keepAllFramesInRam = false;                         ///< Decides whether all frames of the stack will be kept in RAM (or only the current one)
	std::vector<cv::Mat> m_allFrames;                          ///< Mats of all frames if they are all kept in RAM @see m_keepAllFramesInRam
	std::vector<void*> m_memAllFrames;                         ///< Memory for each frames if they are all kept in RAM @see m_keepAllFramesInRam
	bool m_doBrightnessAdjustmentGpu = false;


};
#else                                                       // WITH_TIFF_FLOAT32

#include <string>
#include <exception>
#include <opencv2/opencv.hpp>

#define UNUSED(X)  ((void)(&(X)))

class TiffStackReader
{
public:
	TiffStackReader(std::string fileName = "") { UNUSED(fileName); if (!fileName.empty()) std::runtime_error("Compiled without WITH_TIFF_FLOAT32!"); }
	virtual ~TiffStackReader() {}

	void openTiff( std::string fileName ) { UNUSED(fileName); throw std::runtime_error("Compiled without WITH_TIFF_FLOAT32!"); }
	void closeTiff() {}
	void previousFrame() {}
	void nextFrame() {}
	void goToFrame(uint idx) { UNUSED(idx);}
	bool hasMoreFrames() { return false; }
	bool isOk() { return false; }
	void adjustBrightness() {}
	void adjustBrightness(float min, float max){UNUSED(min); UNUSED(max);}
	inline bool hasFileOpen() const { return false; }

	cv::Mat currentFrame() {return cv::Mat(); }
	uint numFrames() { return 0; }
	uint currentIdx() { return 0; }


	void setKeepAllFramesInRam( bool inRam ){ UNUSED(inRam); }

};

#endif                                                      // WITH_TIFF_FLOAT32
#endif                                                      // TIFFSTACKREADER_HPP
