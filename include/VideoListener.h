#ifndef VIDEO_LISTENER_H
#define VIDEO_LISTENER_H

#include <vector>
#include <opencv2/highgui/highgui.hpp>

class VideoListener {

	public:
		VideoListener();
		virtual ~VideoListener();
		virtual void onMovement(cv::Mat &colorImage, cv::Mat &grayImage)=0;
		virtual void onFrame(cv::Mat &colorImage)=0;
		virtual void setWidthHeight(int width, int height)=0;
};


#endif
