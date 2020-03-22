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
	m_serverIpAddress{ipAddress}, m_serverPortNumber{portNumber},
	m_isReceiveThreadRunning{false}
{
	// std::cout << "Client constructor.\n";
}

Client::~Client()
{
	// std::cout << "Client destructor.\n";
	if(m_isReceiveThreadRunning && m_receiveThread.joinable())
	{
		m_isReceiveThreadRunning = false;
		m_receiveThread.join();
	}
	close(m_sockfd);

	std::cout << "Closing." << std::endl;
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

void Client::startReceiving()
{
	m_receiveThread = std::thread {[&]() { receive(); }};
	m_receiveThread.detach();
}

void Client::receive()
{
	unsigned int bufferSize {4096};
	char receiveBuffer [bufferSize];

	m_isReceiveThreadRunning = true;
	while(m_isReceiveThreadRunning)
	{
		memset(receiveBuffer, 0, bufferSize);

		long int receivedBytes {recv(m_sockfd, receiveBuffer, static_cast<int>(bufferSize), 0)};
		if(receivedBytes == -1)
		{
			std::cerr << "Error: can't receive data." << std::endl;
			return;
		}
		else if(receivedBytes == 0)
		{
			std::cerr << "Server shut down." << std::endl;
			break;
		}
		else
		{
			std::cout << "SERVER> " << std::string(receiveBuffer, 0, receivedBytes) << "\n";
		}
	}
}

long int Client::sendData(const std::string& msg)
{
	return send(m_sockfd, msg.c_str(), static_cast<int>(msg.length()), 0);
}

void Client::startSendingFile(const std::string& fileName)
{
	std::thread t {[&](){
		sendFile(fileName);
	}};
	t.join();
}

void Client::sendFile(const std::string& fileName)
{
	std::cout << "Opening file.\n";
	FILE* filePtr;
	filePtr = fopen(fileName.c_str(), "r");

	if(filePtr == nullptr)
	{
		std::cerr << "Error: can't open file: " << fileName << "!" << std::endl;
	}
	else
	{
		long int totalSentBytes {0};
		while(true)
		{
			unsigned char buffer [1024];
			memset(buffer, 0, 1024);

			long unsigned int readBytes {fread(buffer, 1, 1024, filePtr)};
			std::cout << "readBytes = " << readBytes << "\n";
			if(readBytes > 0)
			{
				long int sentBytes {send(m_sockfd, buffer, readBytes, 0)};
				totalSentBytes += sentBytes;
				if(sentBytes <= -1)
				{
					std::cerr << "Error: can't send data!" << std::endl;
				}
			}
			else if(readBytes < 1024)
			{
				if(feof(filePtr))
					std::cout << "End of file.\n";
				else if(ferror(filePtr))
					std::cerr << "Error: can't read file!" << std::endl;
				break;
			}
		}

		std::cout << "Total bytes sent: " << totalSentBytes << "\n";
	}
}

void Client::run()
{
	m_sockfd = createSocket();
	if(m_sockfd == -1)
	{
		std::cerr << "Error: can't create a socket!" << std::endl;
		return;
	}

	std::cout << "Connecting to server...\n\n";
	int connectResult {conn()};
	if(connectResult <= -1)
	{
		std::cerr << "Error: can't connect to server!" << std::endl;
		return;
	}

	std::cout << "Connected.\n";
	displayInfo("Server", &m_hint);

	startReceiving();

	std::string input;
	std::cout << "(type \"/close\" to disconnect)\n\n";
	std::cout << "(type \"/sendfile\" to send a file)\n\n";

	while(true)
	{
		std::cout << "YOU> ";
		getline(std::cin, input);

		if(input == "/close")
			break;
		else if(input == "/sendfile")
		{
			std::cout << "Enter filename:\n";
			std::string fileName;
			getline(std::cin, fileName);

			sendData(input);

			startSendingFile(fileName);
		}
		else
		{
			long int sentBytes {sendData(input)};
			if(sentBytes == -1)
			{
				std::cerr << "Error: can't send data." << std::endl;
				break;
			}
			else if(sentBytes == 0)
			{
				std::cout << "Disconnecting..." << std::endl;
				break;
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

	getnameinfo((sockaddr*)&saddrPtr, sizeof(sockaddr_in), ip, NI_MAXHOST, nullptr, 0, 0);

	inet_ntop(AF_INET, &saddrPtr->sin_addr, ip, NI_MAXHOST);
	std::cout << name << " IP: " << ip << ", port: " << ntohs(saddrPtr->sin_port) << "\n";
}