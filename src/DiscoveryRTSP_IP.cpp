#include "DiscoveryRTSP_IP.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace std;

DiscoveryRTSP_IP::DiscoveryRTSP_IP(std::string initialIP) {
	timeout.tv_sec = 2;
    timeout.tv_usec = 0;

	//port = 81;	
	port = 554;
	//port = 1018;	
	//port = 3128;
	//port = 5000;
	//port = 3702;
	//port = 8080;
	//port = 8899;

    server.sin_family = AF_INET;
    server.sin_port = htons( port );
    
    int last = initialIP.find_last_of('.');
    begin = initialIP.substr(0, last+1);
    ipBegin = begin.c_str();
    string end = initialIP.substr(last+1, initialIP.size()-last);
    ipIni = atoi(end.c_str());
}

DiscoveryRTSP_IP::~DiscoveryRTSP_IP() {
	if (socket_desc)
		close(socket_desc);
}

void DiscoveryRTSP_IP::initSocket() {
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
//	socket_desc = socket(AF_INET , SOCK_PACKET	 , 0);

    if (setsockopt (socket_desc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        printf("setsockopt failed\n");

    if (setsockopt (socket_desc, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        printf("setsockopt failed\n");
}

/**
 * Find all IPs that have the RTSP port (554) openned.
 *
 * @return the last IP that have RTSP port openned.
 */
std::list<char*> DiscoveryRTSP_IP::discovery() {
	char ip[20];
	char * ip2 = NULL;
	bool r;
	std::list<char*> iplist;

	//struct sockaddr_in localAddress;
	//socklen_t addressLength = sizeof(localAddress);
	/*struct sockaddr_in localAddress;
	getsockname(socket_desc, (struct sockaddr*)&localAddress, &addressLength);
	printf("local address1: %s\n", inet_ntoa( localAddress.sin_addr));
	server.sin_addr.s_addr = inet_addr("localhost");
	connect(socket_desc , (struct sockaddr *)&server, sizeof(server));
	getsockname(socket_desc, (struct sockaddr*)&localAddress, &addressLength);
	printf("local address2: %s\n", inet_ntoa( localAddress.sin_addr));
	connect(socket_desc , (struct sockaddr *)&server, sizeof(server));
	getsockname(socket_desc, (struct sockaddr*)&localAddress, &addressLength);
	printf("local address3: %s\n", inet_ntoa( localAddress.sin_addr));*/

    for (int i=ipIni ; i<15 ; i++) {
    	sprintf(ip, "%s%d", ipBegin, i);
        r = connectIP(ip);
        
        
        //getsockname(socket_desc, (struct sockaddr*)&localAddress, &addressLength);
		//printf("local address3: %s\n", inet_ntoa( localAddress.sin_addr));
        
        
        if (r) {
	        cout << "Founded: " << ip << endl;
	        ip2 = new char[20];
		    memcpy(ip2, ip, strlen(ip)+1);
			iplist.push_back(ip2);
			//break;
	    }
		else
			cout << "Search : " << ip << endl;
    }
    
    return iplist;
}


bool DiscoveryRTSP_IP::connectIP(char * ip) {
	server.sin_addr.s_addr = inet_addr(ip);

	initSocket();

	//Connect to cam server	
	int connected = connect(socket_desc , (struct sockaddr *)&server, sizeof(server));

	close(socket_desc);

	if (connected>=0) {
		return true;
	}
	else
		return false;


}

