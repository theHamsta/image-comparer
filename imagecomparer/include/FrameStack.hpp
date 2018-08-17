#ifndef FRAMESTACK_HPP
#define FRAMESTACK_HPP


#include <string>

#include <opencv2/opencv.hpp>




class FrameStack
{
	public:
		FrameStack() {}
		virtual ~FrameStack() {}

		virtual void release( bool keepBrightnessAndContrastSettings = false ) = 0;
		virtual void previousFrame() = 0;
		virtual void nextFrame() = 0;
		virtual void goToFrame( int idx ) = 0;
		virtual bool hasMoreFrames() = 0;
		virtual bool isOk() = 0;
		virtual void adjustBrightness() = 0;
		virtual void adjustBrightness( float min, float max ) = 0;
		virtual bool hasFileOpen() const = 0;

		virtual cv::Mat currentFrame() = 0;
		virtual uint numFrames() const = 0;
		virtual uint currentIdx() const = 0;
		virtual bool keepAllFramesInRam() const = 0;

		virtual void setKeepAllFramesInRam( bool inRam ) = 0;

};

#endif /* FRAMESTACK_HPP */
