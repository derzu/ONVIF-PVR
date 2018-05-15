#include "VideoPlayer.h"
#include <iostream>

#if WIN32
  #include <windows.h>
#else
  #include <X11/Xlib.h>
#endif

#include <opencv2/imgproc/imgproc.hpp>

// usleep
#include <unistd.h>

#define SHOW_WINDOW
//#define DEBUG_VALGRIND

VideoPlayer::VideoPlayer(std::vector<Camera*> &cameras, const char * title) {
	this->cameras = cameras;
	this->title = title;
	stopped = false;
	
	std::cout << "Initializing, title: " << title << std::endl;
}

VideoPlayer::~VideoPlayer() {
	stop();

	// espera as threads das cameras finalizarem.
	for (int i=0 ; i<cameras.size() ; i++)
		while (cameras[i]->isRunning())
			usleep(50000);

	for (int i=0 ; i<cameras.size() ; i++)
		if (cameras[i])
			delete cameras[i];
	cameras.clear();
}

/**
 * Stop all cameras.
 * Stop main video player loop.
 */
void VideoPlayer::stop() {
	for (int i=0 ; i<cameras.size() ; i++)
		cameras[i]->stop();

	stopped = true;
}


/**
 * Multi-platform method to get the screen resolution of the system.
 *
 * @param width variable where the width will be set. 
 * @param height variable where the height will be set. 
 */ 
void VideoPlayer::getScreenResolution(int &width, int &height) {
#if WIN32
	width  = (int) GetSystemMetrics(SM_CXSCREEN);
	height = (int) GetSystemMetrics(SM_CYSCREEN);
#else
	Display* disp = XOpenDisplay(NULL);
	Screen*  scrn = DefaultScreenOfDisplay(disp);
	width  = scrn->width;
	height = scrn->height;
#endif
}



// http://answers.opencv.org/question/13876/read-multiple-images-from-folder-and-concat-display-images-in-single-window-opencv-c-visual-studio-2010/
void VideoPlayer::createOne(std::vector<cv::Mat> & images, cv::Mat &result, int cols, int min_gap_size)
{
    // let's first find out the maximum dimensions
    int max_width = 0;
    int max_height = 0;
	if (max_width==0)
		for ( int i = 0; i < images.size(); i++) {
		    // check if type is correct 
		    // you could actually remove that check and convert the image 
		    // in question to a specific type
		    if ( i > 0 && images[i].type() != images[i-1].type() ) {
		        std::cerr << "WARNING:createOne failed, different types of images";
		        return ;
		    }
		    max_height = std::max(max_height, images[i].rows);
		    max_width = std::max(max_width, images[i].cols);
		}
    // number of images in y direction
    int rows = std::ceil(images.size() / (float)cols);

    // create our result-matrix.
	if (result.empty()) {
		//printf("result.empty\n");
	    result = cv::Mat::zeros(rows*max_height + (rows-1)*min_gap_size,
                                cols*max_width  + (cols-1)*min_gap_size, images[0].type());
	}
    size_t i = 0;
    int current_height = 0;
    int current_width = 0;
    for ( int y = 0; y < rows; y++ ) {
        for ( int x = 0; x < cols; x++ ) {
            if ( i >= images.size() ) // shouldn't happen, but let's be safe
                return ;
            // get the ROI in our result-image
			//printf("%d::%d::%d::%d::%d::%d\n", current_height, current_height + images[i].rows, current_width,  current_width  + images[i].cols, images[i].rows, images[i].cols);
            cv::Mat roi(result,
                       cv::Range(current_height, current_height + images[i].rows),
                       cv::Range(current_width,  current_width  + images[i].cols));
            // copy the current image to the ROI
            images[i++].copyTo(roi);
            current_width += max_width + min_gap_size;
        }
        // next line - reset width and update height
        current_width = 0;
        current_height += max_height + min_gap_size;
    }
}




//https://stackoverflow.com/questions/5089927/show-multiple-2-3-4-images-in-the-same-window-in-opencv
/**
 * @brief makeCanvas Makes composite image from the given images
 * @param vecMat Vector of Images.
 * @param windowHeight The height of the new composite image to be formed.
 * @param nRows Number of rows of images. (Number of columns will be calculated
 *              depending on the value of total number of images).
 * @return new composite image.
 */
cv::Mat VideoPlayer::makeCanvas(std::vector<cv::Mat>& vecMat, cv::Mat &canvasImage, int windowHeight, int nRows) {
    int N = vecMat.size();
    nRows  = nRows > N ? N : nRows; 
    int edgeThickness = 10;
    int imagesPerRow = ceil(double(N) / nRows);
    int resizeHeight = floor(2.0 * ((floor(double(windowHeight - edgeThickness) / nRows)) / 2.0)) - edgeThickness;
    int maxRowLength = 0;

    std::vector<int> resizeWidth;
    for (int i = 0; i < N;) {
        int thisRowLen = 0;
        for (int k = 0; k < imagesPerRow; k++) {
            double aspectRatio = double(vecMat[i].cols) / vecMat[i].rows;
            int temp = int( ceil(resizeHeight * aspectRatio));
            resizeWidth.push_back(temp);
            thisRowLen += temp;
            if (++i == N) break;
        }
        if ((thisRowLen + edgeThickness * (imagesPerRow + 1)) > maxRowLength) {
            maxRowLength = thisRowLen + edgeThickness * (imagesPerRow + 1);
        }
    }
    int windowWidth = maxRowLength;
	if (canvasImage.empty()) // create a new canvas, if had some change.
        canvasImage = cv::Mat(windowHeight, windowWidth, CV_8UC3, cv::Scalar(0, 0, 0));

    for (int k = 0, i = 0; i < nRows; i++) {
        int y = i * resizeHeight + (i + 1) * edgeThickness;
        int x_end = edgeThickness;
        for (int j = 0; j < imagesPerRow && k < N; k++, j++) {
            int x = x_end;
            cv::Rect roi(x, y, resizeWidth[k], resizeHeight);
            //cv::Size s = canvasImage(roi).size();
			cv::Size s(resizeWidth[k], resizeHeight);
            // change the number of channels to three
            cv::Mat target_ROI(cv::Size(resizeWidth[k], resizeHeight), CV_8UC3);

            if (vecMat[k].channels() != canvasImage.channels()) {
                if (vecMat[k].channels() == 1) {
                    cv::cvtColor(vecMat[k], target_ROI, CV_GRAY2BGR);
                }
            } else {             
                vecMat[k].copyTo(target_ROI);
            }
            cv::resize(target_ROI, target_ROI, s);
            if (target_ROI.type() != canvasImage.type()) {
                target_ROI.convertTo(target_ROI, canvasImage.type());
            }
			//printf("%3d::%3d::%3d::%3d::%d::%d::%d::%d\n", x, y, resizeWidth[k], resizeHeight, windowWidth, windowHeight, s.width, s.height);
            target_ROI.copyTo(canvasImage(roi));
            x_end += resizeWidth[k] + edgeThickness;
        }
    }
    return canvasImage;
}



/**
 * Main loop where the camera frames are shown on the screen.
 * 
 */
void VideoPlayer::run() {
    int key=0;
    int ret = INTERNAL_QUIT;
	int k=-1;
	int width, height;
	int typeView = ALL_CAMS;
	int camIndex=0;
	cv::Mat fullImage;

	getScreenResolution(width, height);

	int lastEmptyQuant = 0;
	// While to show the frames.
#ifndef DEBUG_VALGRIND
	while (!stopped)
#else
	for (k=0 ; k<2 ; k++)
#endif
	{

#ifdef SHOW_WINDOW
		cv::namedWindow(title, cv::WINDOW_NORMAL); //create a window

		cv::resizeWindow(title, width, height);
#endif

		if (typeView == ALL_CAMS)
		{
			std::vector<cv::Mat> frames;
			int emptyQuant = 0;
			bool layoutChange = false;
			// Atualiza os frames.
			for (int i=0 ; i<cameras.size() ; ) {
				// camera was finalized.
				if (cameras[i]->isStopped() && !cameras[i]->isRunning()) {
					delete cameras[i];
					cameras[i] = NULL;
					cameras.erase(cameras.begin() + i);
					layoutChange = true;
					continue;
				}
				cv::Mat f = cameras[i]->getFrame();
				if (cameras[i]->empty(f))
					emptyQuant++;
				frames.push_back(f);

				i++;
			}
			if (cameras.empty())
				break;

			if (emptyQuant != lastEmptyQuant) {
				layoutChange = true;
				lastEmptyQuant = emptyQuant;
			}

			if (layoutChange) {
				colorImage.release(); // make Mat empty()
				fullImage.release(); // make Mat empty()
			}

			int lines = (int)round(sqrt(frames.size()));
			makeCanvas(frames, colorImage, height, lines);

			//createOne(frames, fullImage, lines, 10);
			//cv::resize(fullImage, colorImage, cv::Size(width, height));//resize image

			//printf("w=%d h=%d\n", fullImage.cols, fullImage.rows);
		}
		else
			colorImage = cameras[camIndex]->getFrame();

#ifdef SHOW_WINDOW
        cv::imshow(title, colorImage); //show the colorImage in window
		key = cv::waitKey(15);
        if ( (key == 27 || key=='q' || key=='Q') ||                  // If 'esc' or 'Q' key is pressed, break loop
			 (cv::getWindowProperty(title, cv::WND_PROP_AUTOSIZE)) ) // if x (close button) of the window is pressed
        {
            std::cout << "Quitted by user" << std::endl;
			ret = USER_QUIT;
            break;
        } else
		// next view. Change the view to the next camera.
		if (key == 'N' || key == 'n') {
			typeView = ONE_CAM;
			cv::Mat f;
			do {
				camIndex = (camIndex + 1) % cameras.size();
			} while (cameras[camIndex]->empty());
		} else // Change the view to show all the cameras.
		if (key == 'A' || key == 'a') {
			typeView = ALL_CAMS;
			colorImage.release();
		}
#endif
		

		if (ret==USER_QUIT) break; // So sai daqui no USER_QUIT, o INTERNAL_QUIT nao sai.
	}
	std::cout << title << "Exiting VideoPlayer::run()\n";
	stop(); // force user_quit
}

