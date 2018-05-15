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

		/**
		 * Main loop where the camera frames are shown on the screen.
		 * 
		 */
		void run();

		/**
		 * Stop all cameras.
		 * Stop main video player loop.
		 */
		void stop();
		
	private:
		void createOne(std::vector<cv::Mat> & images, cv::Mat &result, int cols, int min_gap_size);

		/**
		 * @brief makeCanvas Makes composite image from the given images
		 * @param vecMat Vector of Images.
		 * @param windowHeight The height of the new composite image to be formed.
		 * @param nRows Number of rows of images. (Number of columns will be calculated
		 *              depending on the value of total number of images).
		 * @return new composite image.
		 */
	    cv::Mat makeCanvas(std::vector<cv::Mat>& vecMat, cv::Mat &canvasImage, int windowHeight, int nRows);

		/**
		 * Multi-platform method to get the screen resolution of the system.
		 *
		 * @param width variable where the width will be set. 
		 * @param height variable where the height will be set. 
		 */ 
		void getScreenResolution(int &width, int &height);
		const char * title;
		bool stopped;
		cv::Mat colorImage;

		static const int ALL_CAMS = 1;
		static const int ONE_CAM  = 2;
		
		std::vector<Camera*> cameras; // many cameras
};

#endif
