/**
 * \file   TiffStackReader.hpp
 * \brief  Defines class TiffStackReader
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   September 9th, 2017
 */

#ifndef TIFFSTACKREADER_HPP
#define TIFFSTACKREADER_HPP


#include <string>

#include "tinytiffreader.h"
#include "FrameStack.hpp"
#include <opencv2/opencv.hpp>



/**
* @brief Class for reading Tiff image stacks
* and converting the frames into cv::Mats
*
*/
class TiffStackReader : public FrameStack
{
	public:
		TiffStackReader( std::string fileName = "", bool keepAllFramesInRam = false );
		TiffStackReader( const TiffStackReader& copy ) = delete;
		virtual ~TiffStackReader();

		void openTiff( std::string fileName, bool keepBrightnessAndContrastSettings = false );
		void closeTiff( bool keepBrightnessAndContrastSettings = false );
		void release( bool keepBrightnessAndContrastSettings = false ) override {closeTiff( keepBrightnessAndContrastSettings );} ;
		void previousFrame() override;
		void nextFrame() override;
		void goToFrame( int idx ) override;
		bool hasMoreFrames() override;
		bool isOk() override;
		void adjustBrightness() override;
		void adjustBrightness( float min, float max ) override;
		inline bool hasFileOpen() const override { return m_file !=  nullptr; }

		cv::Mat currentFrame() override;
		uint numFrames() const override;
		uint currentIdx() const override;
		bool keepAllFramesInRam() const override { return m_keepAllFramesInRam; }

		void setKeepAllFramesInRam( bool inRam ) override;

	private:
		void readCurrentFrame( cv::Mat* dstMat = nullptr );
		bool hasErrors();

		TinyTIFFReaderFile* m_file = nullptr;                      ///< Currently opened tiff file
		std::string m_fileName;                                    ///< Currently opened tiff file
		void* m_dataCurrrentFrame = nullptr;                       ///< Buffer for currently opened frame in stack
		uint m_bufferSizeCurrentFrame = 0;                         ///< Buffer size of currently opened frame in stack
		cv::Mat m_mat;                                             ///< cv::Mat wrapping currently opened frame of stack
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
#endif                                                      // TIFFSTACKREADER_HPP
