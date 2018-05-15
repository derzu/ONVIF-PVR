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

		void stop();
		bool isStopped();
		void join();
		void setUDP(bool udp);
		bool isToSave();
		bool isUDP();
		bool isReady();
		bool isRunning();
		const char * getTitle();

		/**
		 * Start the run method over a thread.
		 */
		void start();

		/**
		 * Verify if has movement on the camera looking for the 2 last frames.
		 * 
		 * @param frame1 first frame to be compared.
		 * @param frame2 second frame to be compared.
		 *
		 * @return true if had movement, false if not.
		 */
		bool verifyMovement(uchar* frame1, uchar* frame2, int size);

		/**
		 * Verify if the frame is empty. It is Mat is empty or if it is the default 600x600 frame.
		 * 
		 * @return if the frame is or not empty.
		 */
		bool empty();

		/**
		 * Verify if the frame f is empty. It is Mat is empty or if it is the default 600x600 frame.
		 * 
		 * @param f Mat to be verified. 
		 *
		 * @return if the frame is or not empty.
		 */
		bool empty(cv::Mat f);

		/**
		 * Returns the last grabbed frame.
		 *
		 * @return last grabbed frame.
		 */
		cv::Mat getFrame();

		// listeners
		void notifyMovementListeners(cv::Mat &colorImage, cv::Mat &grayImage);
		void notifyAllFramesListeners(cv::Mat &colorImage);
		void registerMovementListener(VideoListener * listener);
		void registerAllFramesListener(VideoListener * listener);

		cv::VideoCapture * stream = NULL;

	private:
		/**
		 * Camera init loop.
		 * Open, and init the camera.
		 * It is a loop to to be more robust.
		 *
		 * @see playVideo()
		 */
		void run();

		/**
		 * Camera main loop. 
		 * It grabs and process the frame from the camera.
		 * 
		 * @param stream the VideoCapture.
		 *
		 * @return the exit status (error or not)
		 */
		int playVideo(cv::VideoCapture * stream);

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
