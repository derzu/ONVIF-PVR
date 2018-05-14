#include "Camera.h" 

#include <iostream>

#include <opencv2/imgproc/imgproc.hpp>

// usleep
#include <unistd.h>

//#define DEBUG_VALGRIND

Camera::Camera(char * title, char * url, bool udp, bool save) {
	this->title = title;
	this->url = url;
	qPixels = 0;
	stopped = false;
	this->udp = udp;
	this->save = save;
	ready = false;
	running = false;

	mtx  = new std::mutex();
	mtx2 = new std::mutex();

	lastFrame = NULL;

	width = height = 0;
	hasMovement=false;
	
	frameListener = NULL;

	grayImage.release();
	colorImage.release();

	threadPlayer = NULL;

	gettimeofday(&lastMovementTime, NULL);

	loading = cv::imread("imgs/loading.png", CV_LOAD_IMAGE_COLOR);   // Read the file
}

Camera::~Camera() {
	stop();

	if (lastFrame)
		delete [] lastFrame;
		
	grayImage.release();
	colorImage.release();
	frame.release();
		
	for (std::vector<VideoListener*>::iterator it = moveListeners.begin(); it != moveListeners.end(); it++) {
		std::cout << title << "::Deletando::" << *it << std::endl;
		if (*it) {
			delete (*it);
			if (*it == frameListener)
				frameListener = NULL;
		}
	}
	moveListeners.clear();
	
	if (title)
		delete [] title;
	if (url)
		delete [] url;

	if (frameListener)
		delete frameListener;
	frameListener = NULL;

	join();

	if (threadPlayer)
		delete threadPlayer;

	if (mtx)
		delete mtx;
	if (mtx2)
		delete mtx2;
}

void Camera::setUDP(bool udp) {
	this->udp = udp;
}

bool Camera::isUDP() {
	return udp;
}

bool Camera::isReady() {
	return ready;
}

bool Camera::isToSave() {
	return save;
}

void Camera::join() {
	if (threadPlayer && threadPlayer->joinable())
		threadPlayer->join();
}

void Camera::stop() {
	stopped = true;
}

bool Camera::isStopped() {
	return stopped;
}

bool Camera::isRunning() {
	bool ret;
mtx2->lock();
	ret = running;
mtx2->unlock();

	return ret;
}

const char * Camera::getTitle() {
	return title;	
}

void Camera::run() {
	threadPlayer = new std::thread(&Camera::start, this);
}

bool Camera::empty() {
	cv::Mat f = getFrame();

	return f.empty() || (f.rows==600 && f.cols==600);
}

bool Camera::empty(cv::Mat f) {
	return f.empty() || (f.rows==600 && f.cols==600);
}

void Camera::start() {
mtx2->lock();
	running = true;
mtx2->unlock();
	int qFailures = 0;
	bool window = false;
	int k=-1;

	std::cout << std::endl << title << "::starting url:: " << url << std::endl;

	// While para se o player quebrar ele reiniciar automatico.
	cv::VideoCapture * stream = NULL;
#ifndef DEBUG_VALGRIND
	while (!stopped)
#else
	for (k=0 ; k<2 ; k++)
#endif
	{
		if (stream) { delete stream; stream=NULL; }

		if (udp) {
#if WIN32
		_putenv_s("OPENCV_FFMPEG_CAPTURE_OPTIONS", "rtsp_transport;udp");
#else
		setenv("OPENCV_FFMPEG_CAPTURE_OPTIONS", "rtsp_transport;udp", 1);
#endif

		stream = new cv::VideoCapture(url, cv::CAP_FFMPEG);

#if WIN32
	_putenv_s("OPENCV_FFMPEG_CAPTURE_OPTIONS", "");
#else
	unsetenv("OPENCV_FFMPEG_CAPTURE_OPTIONS");
#endif
		}
		else {
			stream = new cv::VideoCapture(url);
			//stream = new cv::VideoCapture(url, cv::CAP_GSTREAMER);
			//stream = new cv::VideoCapture(url, cv::CAP_FFMPEG);
			//stream = new cv::VideoCapture(url, cv::CAP_UNICAP);
		}


		if (!stream->isOpened()) { // if not success, exit program
			std::cout << title << "::Not possible to connect " << url << std::endl;
			qFailures++;

			if (qFailures>10)
				break;
			else {
				usleep(4000000);
				continue;
			}
		}

		qFailures = 0;
	
		width = stream->get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
		height = stream->get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
		std::cout << title << "::Frame size : " << width << " x " << height << std::endl;
		
		for (std::vector<VideoListener*>::iterator it = moveListeners.begin(); it != moveListeners.end(); it++) {
			(*it)->setWidthHeight(width, height);
		}

		int ret = playVideo(stream);
	
		if (ret==USER_QUIT) break; // So sai daqui no USER_QUIT, o INTERNAL_QUIT nao sai.
	}
	stop(); // force user_quit
	if (stream) delete stream;
	std::cout << title << " EXITING! " << title << std::endl;
mtx2->lock();
	running = false;
mtx2->unlock();
}



int Camera::playVideo(cv::VideoCapture * stream) {
    int qFailures = 0;
    int ret = INTERNAL_QUIT;
    int diffTime=0;
    struct timeval actualTime;

	this->width = width;
	this->height = height;
	qPixels = width*height;
	if (lastFrame==NULL) {
		std::cout << title << "::lastFrame Null\n";
		lastFrame = new uchar[qPixels];
		bzero(lastFrame, qPixels);
	}

#ifndef DEBUG_VALGRIND
    while (!stopped)
#else
    for (int j=0 ; j<100 ; j++)
#endif
    {
		// read a new colorImage from video. First grab, than retrive.
	    if (!stream->grab()) //if not success, try some times and break;
        {
             std::cout << title << "::Cannot grab a frame from video stream. Openned: " << stream->isOpened() << std::endl;
             qFailures++;
             if (qFailures>10) {
	             ret = INTERNAL_QUIT;
	             break;
             }

			 usleep(1000000);

             continue;
        }
        else {
			qFailures = 0;
			//colorImage.release();
			mtx->lock();
	        bool val = stream->retrieve(colorImage, 0);
			mtx->unlock();
        	if (!val)
        	{
        		std::cout << title << "::Cannot retrieve a frame from video stream" << std::endl;
			 	usleep(30000);
        		continue;
        	}
        	else {
				// enters here every time, except the first
				if (grayImage.data && !grayImage.empty()) {
					//std::cout << "memcpy lastFrame \n";
					memcpy(lastFrame, grayImage.data, qPixels);
				}
				// enters here just the first time
				else {
					hasMovement = false;
					//std::cout << title << "::release grayImage \n";
					//grayImage.release();
					//grayImage = cv::Mat(colorImage.size(), CV_8UC1);
				}
        		cvtColor(colorImage, grayImage, cv::COLOR_BGR2GRAY);
        	
				ready = true;
        	
        		if (!verifyMovement(lastFrame, grayImage.data, qPixels)) {
    				gettimeofday(&actualTime, NULL);
    				diffTime = (actualTime.tv_sec-lastMovementTime.tv_sec)*1000000 + (actualTime.tv_usec-lastMovementTime.tv_usec);
    				//std::cout << diffTime << "\n";
					if (diffTime > 1300000) {
						hasMovement = false;
					}
        		}

				if (hasMovement) {
					notifyMovementListeners(colorImage, grayImage);
				}
				notifyAllFramesListeners(colorImage);
        	}
        }

		usleep(15000);
    }
    
	std::cout << title << " EXITING playVideo()\n";

	//grayImage.release();
	//colorImage.release();

    return ret; // Exit by user or by error.
}


bool Camera::verifyMovement(uchar* frame1, uchar* frame2, int size) {
	float percentual = 0.015;
	int maxDiff = 20;
	int diff = 0;

	for (int i=0 ; i<size ; i++)
		if (abs(*(frame1++) - *(frame2++)) > maxDiff)
			diff++;

	//printf("%s::diff = %d, size*percentual=%f\n", title, diff, size*percentual);
	if (diff > size*percentual) {
		//printf("%s::diff = %d, size*percentual=%.1f\n", title, diff, size*percentual);
		//std::cout << title << "::Movimento\n";
		gettimeofday(&lastMovementTime, NULL);
		hasMovement = true;
		return true;
	} else
		return false;
}

cv::Mat Camera::getFrame() {
	mtx->lock();
	bool notAvailable = false;

	if (frame.empty()) {
		if (!colorImage.empty())
			colorImage.copyTo(frame);
		else {
			notAvailable = true;
		}
	}
	else {
		// if on loading image, verify if the vidoe frame is available.
		if (frame.rows == 600 && frame.cols==600) {
			if (!colorImage.empty())
				colorImage.copyTo(frame);
			else {
				notAvailable = true;
			}
		} else
			memcpy(frame.data, colorImage.data, qPixels*3);
	}
	if (notAvailable) {
		// if doesn't start to capture the frames, genereate a loading image 600x600.
		frame = cv::Mat(600, 600, CV_8UC3, cv::Scalar(100, 100, 100));
		if (!frame.empty()) {
			if (!loading.empty()) loading.copyTo(frame.rowRange(108, 108 + loading.rows).colRange(108, 108 + loading.cols));
			cv::putText(frame, "Loading...", cv::Point(220, 310), cv::FONT_HERSHEY_DUPLEX, 1.0, CV_RGB(220, 220, 220), 2);
		}
	}

	mtx->unlock();

	return frame;
}

void Camera::notifyMovementListeners(cv::Mat &colorImage, cv::Mat &grayImag) {
	for (std::vector<VideoListener*>::iterator it = moveListeners.begin(); it != moveListeners.end(); it++) {
		(*it)->onMovement(colorImage, grayImage);
	}
}

void Camera::notifyAllFramesListeners(cv::Mat &colorImage) {
	if (frameListener)
		frameListener->onFrame(colorImage);
}

void Camera::registerMovementListener(VideoListener * listener) {
	moveListeners.push_back(listener);
}

void Camera::registerAllFramesListener(VideoListener * listener) {
	frameListener = listener;
}


