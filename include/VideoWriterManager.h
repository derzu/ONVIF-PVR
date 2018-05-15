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

		/**
		 * @Overrided from VideoListener
		 * 
		 * If movement inserts the color frame at the writing queue.
		 */
		void onMovement(cv::Mat &colorImage, cv::Mat &grayImage);
		void onFrame(cv::Mat &colorImage);
		void stop();
		/**
		 * Thread thta record the video.
		 *
		 */
		int writeFrameVideo();

		/**
		 * Make a copy of data.
		 * 
		 * @param data the source data.
		 * @param size the byte size of the data to be copied. 
		 * 
		 * @return a copy of data.
		 */
		static uchar * copy(unsigned char * data, int size);
	private:
		// Frames queue	
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
