#ifndef VIDEO_LISTENER_H
#define VIDEO_LISTENER_H

#include <vector>
#include <opencv2/highgui/highgui.hpp>

class VideoListener {

	public:
		VideoListener();
		virtual ~VideoListener();

		/**
		 * Method will be dispached when an movement occurs at the video.
		 */
		virtual void onMovement(cv::Mat &colorImage, cv::Mat &grayImage)=0;

		/**
		 * Method will be dispached at all the frames.
		 */
		virtual void onFrame(cv::Mat &colorImage)=0;

		/**
		 * Set width and height of the video frame
		 */
		virtual void setWidthHeight(int width, int height)=0;
};


#endif
