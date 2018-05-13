#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <opencv2/highgui/highgui.hpp>
#include <sys/time.h>

#include <vector>
#include <thread>
#include "Camera.h"
#include "VideoListener.h"

#define USER_QUIT 1
#define INTERNAL_QUIT -1



class VideoPlayer {
	public:
		VideoPlayer(std::vector<Camera*> &cameras, const char * title);
		virtual ~VideoPlayer();

		void start();
		void stop();
		
	private:
		void createOne(std::vector<cv::Mat> & images, cv::Mat &result, int cols, int min_gap_size);
	    cv::Mat makeCanvas(std::vector<cv::Mat>& vecMat, cv::Mat &canvasImage, int windowHeight, int nRows);
		void getScreenResolution(int &width, int &height);
		const char * title;
		bool stopped;
		cv::Mat colorImage;

		static const int ALL_CAMS = 1;
		static const int ONE_CAM  = 2;
		
		std::vector<Camera*> cameras; // many cameras
};

#endif
