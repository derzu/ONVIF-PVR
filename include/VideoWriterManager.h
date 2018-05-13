#ifndef VIDEOWRITERMANAGER_H
#define VIDEOWRITERMANAGER_H

#include "VideoListener.h"

#include <queue>
#include <thread>

class VideoWriterManager : public VideoListener {
	public:
		VideoWriterManager(const char * title);
		virtual ~VideoWriterManager();
		void setWidthHeight(int width, int height);
		void onMovement(cv::Mat &colorImage, cv::Mat &grayImage);
		void onFrame(cv::Mat &colorImage);
		void stop();
		int writeFrameVideo();

	private:
		uchar * copy(unsigned char * data, int size);
	
		std::queue<uchar*> videoQueue;
		std::thread * threadWrite;
		int width, height, qPixels;
		const char * title;
		bool stopped;
		cv::Mat writeMat;
		int camNumber;
		static int quantInstances;
};

#endif
