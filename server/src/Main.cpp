#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " portNumber" << std::endl;
		return -1;
	}

	std::string portNumber {argv[1]};

	// Load up hints
	addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	addrinfo* listBegin;
	int addrInfoResult {getaddrinfo(nullptr, portNumber.c_str(), &hints, &listBegin)};
	if(addrInfoResult != 0)
	{
		std::cerr << "Error getting address info! Error #" << addrInfoResult << std::endl;
		return -2;
	}

	// Iterate through list to find possible address
	int sockfd;
	addrinfo* result;
	for(result = listBegin; result != nullptr; result = result->ai_next)
	{
		sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if(sockfd == -1)
			continue;

		int bindResult {bind(sockfd, result->ai_addr, result->ai_addrlen)};
		if(bindResult == 0)
			break;

		close(sockfd);
	}

	if(result == nullptr)
	{
		std::cerr << "Error binding to socket!" << std::endl;
	}

	freeaddrinfo(listBegin);

	// Print server info
	char serverName [NI_MAXHOST];
	memset(&serverName, 0, NI_MAXHOST);
	char serverServiceName [NI_MAXSERV];
	memset(&serverServiceName, 0, NI_MAXSERV);

	sockaddr_in* saddrin;
	saddrin = (sockaddr_in*)result->ai_addr;
	inet_ntop(AF_INET, (in_addr*)&saddrin->sin_addr, serverName, NI_MAXHOST);
	std::cout << "Server IP: " << serverName << ", port: " << portNumber << "\n\n";

	// Close socket
	close(sockfd);

	return 0;
}