#include <Client.hpp>
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Client::Client(const std::string& ipAddress, int portNumber) :
	m_serverIpAddress{ipAddress}, m_serverPortNumber{portNumber}
{
	// std::cout << "Client constructor.\n";
}

Client::~Client()
{
	close(m_sockfd);
	// std::cout << "Client destructor.\n";
}

int Client::createSocket()
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

int Client::conn()
{
	m_hint.sin_family = AF_INET;
	m_hint.sin_port = htons(m_serverPortNumber);
	inet_pton(AF_INET, m_serverIpAddress.c_str(), &m_hint.sin_addr);

	return connect(m_sockfd, (sockaddr*)&m_hint, sizeof(m_hint));
}

long int Client::sendAll(const std::string& msg)
{
	return send(m_sockfd, msg.c_str(), static_cast<int>(msg.length()), 0);
}

void Client::run()
{
	m_sockfd = createSocket();
	if(m_sockfd == -1)
	{
		std::cerr << "Error: create a socket!" << std::endl;
		return;
	}

	std::cout << "Connecting to server...\n\n";
	int connectResult {conn()};
	if(connectResult <= -1)
	{
		std::cerr << "Error: connect to server!" << std::endl;
		return;
	}

	std::cout << "Connected.\n";
	displayInfo("Server", &m_hint);


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

		long int sentBytes {sendAll(input)};
		if(sentBytes == -1)
		{
			std::cerr << "Error in sending data." << std::endl;
			break;
		}
		else
		{
			memset(receiveBuffer, 0, bufferSize);

			long int receivedBytes {recv(m_sockfd, receiveBuffer, static_cast<int>(bufferSize), 0)};
			if(receivedBytes == -1)
			{
				std::cerr << "Error receiving data." << std::endl;
				break;
			}
			else
			{
				std::cout << "SERVER> " << std::string(receiveBuffer, 0, receivedBytes) << "\n";
			}
		}
	}
}

void Client::displayInfo(const std::string& name, sockaddr_in* saddrPtr)
{
	char ip [NI_MAXHOST];
	memset(ip, 0, NI_MAXHOST);
	char port [NI_MAXSERV];
	memset(port, 0, NI_MAXSERV);

	getnameinfo((sockaddr*)&saddrPtr, sizeof(sockaddr_in), 
				ip, NI_MAXHOST, 
				nullptr, 0, 0);

	inet_ntop(AF_INET, &saddrPtr->sin_addr, ip, NI_MAXHOST);
	std::cout << name << " IP: " << ip << ", port: " << ntohs(saddrPtr->sin_port) << "\n";
}