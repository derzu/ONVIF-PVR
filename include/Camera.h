#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/highgui/highgui.hpp>
#include <sys/time.h>

#include <vector>
#include <thread>
#include <mutex>
#include "VideoListener.h"

#define USER_QUIT 1
#define INTERNAL_QUIT -1

class Camera {
	public:
		Camera(char * title, char *, bool, bool);
		virtual ~Camera();
		bool verifyMovement(uchar* frame1, uchar* frame2, int size);
		int playVideo(cv::VideoCapture * stream);
		void start();
		void stop();
		bool isStopped();
		void join();
		void run();
		void setUDP(bool udp);
		bool isToSave();
		bool isUDP();
		bool isReady();
		bool isRunning();
		const char * getTitle();
		bool empty();
		bool empty(cv::Mat f);

		cv::Mat getFrame();

		// listeners
		void notifyMovementListeners(cv::Mat &colorImage, cv::Mat &grayImage);
		void notifyAllFramesListeners(cv::Mat &colorImage);
		void registerMovementListener(VideoListener * listener);
		void registerAllFramesListener(VideoListener * listener);

		cv::VideoCapture * stream = NULL;

	private:
		const char * title;
		char * url;
		int width, height;
		int qPixels;
		bool stopped;
		bool hasMovement;
		bool udp;
		bool save;
		bool ready;
		bool running;
		struct timeval lastMovementTime;
		uchar * lastFrame;
		cv::Mat grayImage;
		cv::Mat colorImage;
		cv::Mat frame;
		cv::Mat loading;

		std::thread * threadPlayer;
		std::mutex *mtx;
		std::mutex *mtx2;
		
		// listeners
		std::vector<VideoListener*> moveListeners; // many movement listeners
		VideoListener *frameListener; // just one all frame listener
};

#endif
