#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void displayInfo(const std::string& name, sockaddr_in* saddrPtr)
{
	char ip [NI_MAXHOST];
	memset(ip, 0, NI_MAXHOST);
	char port [NI_MAXSERV];
	memset(port, 0, NI_MAXSERV);

	getnameinfo((sockaddr*)&saddrPtr, sizeof(sockaddr_in), 
				ip, NI_MAXHOST, 
				nullptr, 0, 0);

	inet_ntop(AF_INET, &saddrPtr->sin_addr, ip, NI_MAXHOST);
	std::cout << name << "IP: " << ip << ", port: " << ntohs(saddrPtr->sin_port) << "\n";
}


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
	int sockfdListener;
	addrinfo* result;
	for(result = listBegin; result != nullptr; result = result->ai_next)
	{
		sockfdListener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if(sockfdListener == -1)
			continue;

		int bindResult {bind(sockfdListener, result->ai_addr, result->ai_addrlen)};
		if(bindResult == -1)
		{
			close(sockfdListener);
			continue;
		}

		break;
	}

	if(result == nullptr)
	{
		std::cerr << "Error binding to socket!" << std::endl;
	}

	freeaddrinfo(listBegin);

	std::cout << "Server ";
	displayInfo("Server", (sockaddr_in*)result->ai_addr);

	// Listen for connection
	int listenResult {listen(sockfdListener, 3)};
	if(listenResult == -1)
	{
		std::cerr << "Error listening!" << std::endl;
		return -4;
	}

	fd_set master;
	FD_ZERO(&master);
	FD_SET(sockfdListener, &master);

	int sockfdMax {sockfdListener};

	unsigned int maxBufferSize {4096};
	char buffer [maxBufferSize];

	std::cout << "Entering main loop.\n\n";
	while(true)
	{
		fd_set copySet {master};
		int socketCount {select(sockfdMax + 1, &copySet, nullptr, nullptr, nullptr)};
		if(socketCount == -1)
		{
			std::cerr << "Error selecting!" << std::endl;
			break;	
		}

		for(int sockX {0}; sockX <= sockfdMax; ++sockX)
		{
			if(FD_ISSET(sockX, &copySet))
			{
				if(sockX == sockfdListener) // Listening socket 
				{
					sockaddr_storage remoteAddr;
					socklen_t addrSize {sizeof(remoteAddr)};
					int newfd {accept(sockfdListener, (sockaddr*)&remoteAddr, &addrSize)};

					if(newfd <= -1)
					{
						std::cerr << "Error accepting!" << std::endl;
					}
					else
					{
						FD_SET(newfd, &master);
						std::cout << "Connected new client. ";
						displayInfo("Client", (sockaddr_in*)&remoteAddr);
						if(newfd > sockfdMax)
							sockfdMax = newfd;
					}
				}
				else // Get data from clients
				{
					long int bytesReceived {recv(sockX, buffer, maxBufferSize, 0)};
					if(bytesReceived <= -1)
					{
						std::cerr << "Error receiving data! Socket #" << sockX << std::endl;
						close(sockX);
						FD_CLR(sockX, &master);
						return -5;
					}
					else if(bytesReceived == 0)
					{
						std::cout << "Client disconnected. Socket #" << sockX << "\n";
						close(sockX);
						FD_CLR(sockX, &master);
					}
					else
					{
						// Send data to other clients
						for(int sockY {0}; sockY <= sockfdMax; ++sockY)
						{
							if(FD_ISSET(sockY, &master))
							{
								if(sockY != sockfdListener && sockY != sockX) // Don't send to listening socket and itself
								{
									long int bytesSent {send(sockY, buffer, bytesReceived, 0)};
									if(bytesSent <= -1)
									{
										std::cerr << "Error sending data! From Socket #" << sockX << " to " << sockY << std::endl;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	close(sockfdListener);

	return 0;
}