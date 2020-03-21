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

	// Listen for connection
	int listenResult {listen(sockfd, SOMAXCONN)};
	if(listenResult == -1)
	{
		std::cerr << "Error listening!" << std::endl;
		return -4;
	}

	sockaddr_in hostHint;
	socklen_t hostSize {sizeof(hostHint)};

	int sockfdHost {accept(sockfd, (sockaddr*)&hostHint, &hostSize)};
	if(sockfdHost == -1)
	{
		std::cerr << "Error accepting client!" << std::endl;
		return -5;
	}

	// Close listening socket
	close(sockfd);

	// Print client info
	char hostName [NI_MAXHOST];
	memset(hostName, 0, NI_MAXHOST);
	char hostServiceName [NI_MAXSERV];
	memset(hostServiceName, 0, NI_MAXSERV);

	getnameinfo((sockaddr*)&hostHint, hostSize, 
				hostName, NI_MAXHOST, 
				hostServiceName, NI_MAXSERV, 0);

	inet_ntop(AF_INET, &hostHint.sin_addr, hostName, NI_MAXHOST);
	std::cout << "Client IP: " << hostName << ", port: " << ntohs(hostHint.sin_port) << std::endl;

	// Start receiving/sending data
	unsigned int bufferSize {4096};
	char buffer [bufferSize];
	
	while(true)
	{
		memset(buffer, 0, bufferSize);

		long int bytesReceived {recv(sockfdHost, buffer, bufferSize, 0)};
		if(bytesReceived == -1)
		{
			std::cerr << "Error receiving data!" << std::endl;
			break;
		}
		else if(bytesReceived == 0)
		{
			std::cout << "Client disconnected. Closing." << std::endl;
			break;
		}
		else
		{
			std::string message {buffer, 0, static_cast<long unsigned int>(bytesReceived)};
			std::cout << "CLIENT> " << message << std::endl;

			std::string confirmMsg {"Message received"};

			long int sentBytes {send(sockfdHost, confirmMsg.c_str(), static_cast<int>(confirmMsg.length()), 0)};
			if(sentBytes == -1)
			{
				std::cerr << "Error sending data! Disconnecting..." << std::endl;
				break;
			}
			else
			{
				std::cout << "Message sent back.\n";
			}
		}
	}


	close(sockfdHost);

	return 0;
}