#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdio.h>
#include "DiscoveryRTSP_IP.h"

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <string.h>
#include "VideoPlayer.h"
#include "VideoWriterManager.h"

#define M_FOTO 1
#define M_VIDEO_IPCAM  2
#define M_VIDEO_WEBCAM 3

using namespace std;

/**
TODO's

Descobrir url automaticamente.
Testar de fora.
Testar se so salva movimento
Salvar videos diferentes a cada 2GB.

**/

std::vector<Camera *> readConfigurationFile(const char * file);

int main(int argc, char* argv[]) {
	std::list<char *> urls;
	std::vector<Camera *> cameras;
	const char * url = NULL;
	const char * dev = NULL;
	char *title;
	int modo=0;


	// ######### ANALISA PARAMETROS ##########
	if (argc == 1) {
		printf("Usage:\n\t./VideoOnvif IP (will just search the onvifs IPs)\n\t./VideoOnvif RTSP_URL (udp)\n\t./VideoOnvif WEBCAM (or device)\n\t./VideoOnvif configFile.txt\n\n" );
		return -1;
	}

    std::string argv1 = std::string(argv[1]);
	int lpos = argv1.find_last_of('.');
	std::string fim = argv1.substr(lpos+1);

	// webcam
	if (argv1.find("webcam")!=string::npos ) {
		modo = M_VIDEO_WEBCAM;
	// webcam
	} else if (((int)argv1.find("/dev/"))>=0) {
		modo = M_VIDEO_WEBCAM;
		dev = argv1.c_str();
	// config file.
 	} else if (lpos>=0 && (argv1.find(".txt")!=string::npos || argv1.find(".dat")!=string::npos )) {
		modo = M_VIDEO_IPCAM;
		cameras = readConfigurationFile(argv[1]);
	// if rtsp
	} else if ( argv1.find("rtsp://")!=string::npos ) {
		modo = M_VIDEO_IPCAM;
		url = argv[1];	
	// try IPCAM again.
	} else {
		modo = M_VIDEO_IPCAM;
	}

//printf("argv1.find(/dev/) = %ld \n", argv1.find("/dev/") );
//printf("modo = %d %s\n", modo, dev);

	if (modo == M_VIDEO_IPCAM || modo == M_VIDEO_WEBCAM)
	{
		if (modo == M_VIDEO_IPCAM)
		{
			printf("modo M_VIDEO_IPCAM\n");
			if (cameras.empty()) {
				if (url==NULL) {
					printf("Looking for IP\n");
					DiscoveryRTSP_IP disc("192.168.15.2"); // TODO dectar IP/rede auto
					if (argc>2)
						disc = DiscoveryRTSP_IP(argv[2]);
						
					std::list<char *> ips = disc.discovery();
				
					if (ips.empty()) {
						std::cout << "Not possible to found the camera ip." << std::endl;
						return -1;
					}

					printf("Just looking for the IPs. I don't know the url. Exiting.\n");

					// TODO discovery the rtsp url.

					return 0;
				}
				else {
					char*url2 = new char[100];
					memcpy(url2, url, strlen(url)+1);
					urls.push_back(url2);
				}

				int i=0;
				for (std::list<char*>::iterator it=urls.begin(); it!=urls.end(); ++it) {
					title = new char[50];
					sprintf(title, "cam%02d", i++);
					bool udp = (argc>2 && strcmp(argv[2], "udp")==0);
						cameras.push_back(new Camera(title, *it, udp, true));
				}
			}
		}
		else if (modo == M_VIDEO_WEBCAM) {
			printf("modo M_VIDEO_WEBCAM\n");
			if (dev==NULL)
				dev = "/dev/video0"; 
			char *url = new char[50];
			title = new char[50];
			sprintf(title, "webcam");
			sprintf(url, "%s", dev);
			cameras.push_back(new Camera(title, url, false, true) );
		}

		
		for (int i=0 ; i<cameras.size() ; i++) {
			if (cameras[i]->isToSave())
				cameras[i]->registerMovementListener(new VideoWriterManager(cameras[i]->getTitle()));
			cameras[i]->start(); // start the thread
		}

		VideoPlayer *vp = new VideoPlayer(cameras, "Onvif PVR");
		vp->run(); // not a thread
		delete vp;
	}

	printf("Exiting...\n");
	
    return 0;
}

std::vector<Camera *> readConfigurationFile(const char * file) {
	std::ifstream stream;
	stream.open(file);
	std::string line;
	std::string url;
	bool udp;
	bool save;
	std::vector<Camera *> cameras;
	int index = 1;
	char *title;
	char *url_c;

	if (stream.is_open()) {
		while(!stream.eof()) {
			std::getline(stream, line);
			if (line[0]!='#' && line[0]!='\0') // # comments the line. # must be the first character.
			{
				size_t pos = line.find_first_of (" \t\n\r");
				std::string url = line.substr( 0, pos );

				pos = line.find("udp");
				udp = pos!=std::string::npos;

				pos = line.find("save");
				save = pos!=std::string::npos;
				
				url_c = new char[100];
				title = new char[50];
				sprintf(url_c, "%s", url.c_str());
				sprintf(title, "cam%02d", index++);

				Camera * cam = new Camera(title, url_c, udp, save);
				cameras.push_back(cam);
			}
		}
		stream.close();
	}
	else {
		printf("Error opening file %s\n", file);
	}

	return cameras;
}
