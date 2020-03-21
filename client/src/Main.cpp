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
	if(argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " ipAddress portNumber" << std::endl;
		return -1;
	}

	std::string location {argv[1]};
	int portNumber {std::stoi(argv[2])};

	// Create socket
	int sockfd {socket(AF_INET, SOCK_STREAM, 0)};
	if(sockfd == -1)
	{
		std::cerr << "Can't create a socket!" << std::endl;
		return -1;
	}

	// Fill hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(portNumber);
	inet_pton(AF_INET, location.c_str(), &hint.sin_addr);

	// Connect to server
	std::cout << "Connecting to server.\n\n";
	int connectResult {connect(sockfd, (sockaddr*)&hint, sizeof(hint))};
	if(connectResult == -1)
	{
		std::cerr << "Can't connect to server!" << std::endl;
		close(sockfd);
		return -2;
	}

	char serverName [NI_MAXHOST];
	memset(serverName, 0, NI_MAXHOST);
	char serverServiceName [NI_MAXSERV];
	memset(serverServiceName, 0, NI_MAXSERV);

	getnameinfo((sockaddr*)&hint, sizeof(hint), 
				serverName, NI_MAXHOST, 
				serverServiceName, NI_MAXSERV, 0);

	inet_ntop(AF_INET, &hint.sin_addr, serverName, NI_MAXHOST);
	std::cout << "Connected.\nServer IP: " << serverName << ", port: " << ntohs(hint.sin_port) << "\n\n";

	unsigned int bufferSize {4096};
	char receiveBuffer [bufferSize];
	std::string input;

	std::cout << "(type \"/close\" to disconnect)\n\n";

	while(true)
	{
		std::cout << "YOU> ";
		getline(std::cin, input);

		if(input == "/close")
		{
			std::cout << "Disconnecting..." << std::endl;
			break;
		}

		long int sentBytes {send(sockfd, input.c_str(), static_cast<int>(input.length()), 0)};
		if(sentBytes == -1)
		{
			std::cerr << "Error in sending data." << std::endl;
			break;
		}
		else
		{
			memset(receiveBuffer, 0, bufferSize);

			long int receivedBytes {recv(sockfd, receiveBuffer, static_cast<int>(bufferSize), 0)};
			if(receivedBytes == -1)
			{
				std::cerr << "Error receiving data." << std::endl;
				break;
			}
			else
			{
				std::cout << "SERVER> " << std::string(receiveBuffer, 0, bufferSize) << "\n";
			}
		}

	}

	close(sockfd);

	return 0;
}