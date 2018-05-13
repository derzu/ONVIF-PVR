#include "VideoWriterManager.h"
#include <iostream>

// usleep
#include <unistd.h>

int VideoWriterManager::quantInstances = 0;

VideoWriterManager::VideoWriterManager(const char * title) {
	this->width = 0;
	this->height = 0;
	this->qPixels = 0;
	this->title = title;
	stopped = false;
	camNumber = quantInstances++;

	threadWrite = NULL;
}

VideoWriterManager::~VideoWriterManager() {
	stop();

	if (threadWrite && threadWrite->joinable())
		threadWrite->join();

	if (threadWrite)
		delete threadWrite;
}

void VideoWriterManager::setWidthHeight(int width, int height) {
	this->width = width;
	this->height = height;
	this->qPixels = width * height;
}

void VideoWriterManager::onMovement(cv::Mat &colorImage, cv::Mat &grayImage) {
	if (writeMat.data==NULL) {
		writeMat = colorImage.clone();
		threadWrite = new std::thread(&VideoWriterManager::writeFrameVideo, this);
	}
	videoQueue.push(copy(colorImage.data, qPixels*3));
}

void VideoWriterManager::onFrame(cv::Mat &colorImage) {

}

uchar * VideoWriterManager::copy(unsigned char * data, int size) {
	uchar *data2 = new uchar[size]; // TODO, nao alocar sempre.
	memcpy(data2, data, size);
	return data2;
}


void VideoWriterManager::stop() {
	stopped = true;
}

/**
 * Thread que escreve o video.
 *
 **/
int VideoWriterManager::writeFrameVideo() {
	// Setup output video
	cv::VideoWriter * output = NULL;
	//cv::VideoWriter * output = (cv::VideoWriter *)1;
	time_t ti = time(NULL);
	struct tm *t = localtime(&ti);
	char fileName[50];
	sprintf(fileName, "videos/%d_%02d_%02d__%02d:%02d:%02d_%s.avi", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, title);
	std::cout << "TIME: " << fileName << "\n";
	
	std::cout << "Enter Writer\n";
	
	// while inicial de configuracao.
	while (writeMat.data==NULL || output==NULL)
	{
		if (output==NULL && width) {
			output = new cv::VideoWriter(
				//"video.mp4",
				//"video.mjpg",
				//"video.mpeg",
				//"video.ts",
				//"video2.avi",
				fileName,
	           //*CV_FOURCC('M','P','4','2'),
	           //*CV_FOURCC('M','P','G','4'),
	           //CV_FOURCC('M','J','P','G'),
	           //CV_FOURCC('M','P','E','G'),
	           //*CV_FOURCC('M','P','4','V'),
	           //CV_FOURCC('W', 'M', 'V', '2'),
	           CV_FOURCC('D','I','V','X'),
	           //CV_FOURCC('H','2','6','4'),
	           //*CV_FOURCC('X','2','6','4'),
	           //CV_FOURCC('X','I','V','D'),
	           13,
	           cv::Size(width, height) );
			if (!output->isOpened())
			{
				std::cout << "!!! Output video could not be opened " << width << "x" << height << std::endl;
				delete output;
				return -1;
			}
		}
					
		if (stopped) break;
		
		usleep(30);
	}
	
	// while de gravacao.
	while (writeMat.data && output) {
		if (videoQueue.empty()) {
			if (stopped) break;
			usleep(30);
		}
		else {
			writeMat.data = videoQueue.front();
			videoQueue.pop();
			output->write(writeMat);
			//std::cout << "escrevendo para arquivo o frame\n";
			delete [] writeMat.data;
		}
	}

	// libera a memoria
	while (!videoQueue.empty()) {
		std::cout << "freezing memory\n";
		writeMat.data = videoQueue.front();
		videoQueue.pop();
		delete [] writeMat.data;	
	}
	
	std::cout << camNumber << "::EXITING writer\n";
	
	if (output)
		delete output;
	writeMat.data = NULL;
	writeMat.release();
}

