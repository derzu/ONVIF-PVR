#ifndef DISCOVERYRTSP_IP_H
#define DISCOVERYRTSP_IP_H

#include <sys/types.h>
#include <arpa/inet.h>

#include <string>
#include <list>

class DiscoveryRTSP_IP {
	public:
		DiscoveryRTSP_IP(std::string initialIP);
		virtual ~DiscoveryRTSP_IP();
		std::list<char *> discovery();
		
	private:
		bool connectIP(char * ip);
		void initSocket();
	
		int port;
		char ip[20];
		struct sockaddr_in server;
		int socket_desc;
		char * initialIP;
		const char * ipBegin;
		std::string begin;
		int ipIni;
		struct timeval timeout;
};

#endif
