#include <Server.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <string.h>

Server::Server(int portNumber) :
	Listener{portNumber},
	m_isPollThreadRunning{false}
{
}

Server::~Server()
{
	// TODO: add goodbye message
	m_isPollThreadRunning = false;
	if(m_pollThread.joinable())
		m_pollThread.join();

	std::cout << "Closing server." << std::endl;
}

int Server::start()
{
	int createListeningResult {createListeningSocket()};
	if(createListeningResult == -1)
	{
		std::cerr << "Error: can't get address info!" << std::endl;
		return -1;
	}
	else if(createListeningResult == -2)
	{
		std::cerr << "Error: can't bind listening socket!" << std::endl;
		return -1;
	}

	int listeningResult {startListening()};
	if(listeningResult <= -1)
	{
		std::cerr << "Error: can't start listening!" << std::endl;
		return -1;
	}

	m_pollThread = std::thread{&Server::startPolling, this};


	std::string input;
	std::cout << "\n(type \"/close\" to disconnect)\n\n";
	
	while(true)
	{
		std::getline(std::cin, input);
		if(input == "/close")
			return 0;
	}
}

void Server::startPolling()
{
	m_isPollThreadRunning = true;
	while(m_isPollThreadRunning)
	{
		int pollResult {poll()};
		if(pollResult == -1)
		{
			std::cerr << "Error: select socket!" << std::endl;
			m_isPollThreadRunning = false;
		}
		else if(pollResult == -2)
		{
			std::cerr << "Error: can't accept a client!" << std::endl;
			m_isPollThreadRunning = false;
		}
	}
}

void Server::receiveHandler(const char* receiveBuffer, long int receivedBytes, int senderSockfd)
{
	if(receivedBytes <= -1)
	{
		std::cerr << "Error: receiving data! Socket #" << senderSockfd << std::endl;
	}
	else if(receivedBytes == 0)
	{
		std::cout << "Client disconnected. Socket #" << senderSockfd << std::endl;
		removeSocket(senderSockfd);
	}
	else
	{
		std::string receivedData {receiveBuffer};
		std::cout << "CLIENT #" << senderSockfd << "> " << receivedData << std::endl;

		if(receivedData.find("/sendfile") == 0)
		{
			std::string fileName {receivedData.begin() + receivedData.find_first_not_of("/sendfile "), receivedData.end()};
			std::cout << "Request to send file: \"" << fileName << "\"" << std::endl;

			sendAllExcept(senderSockfd, "/receivefile " + fileName);
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

			if(waitForAcceptFile(senderSockfd) <= -1)
			{
				std::cout << "File not accepted." << std::endl;
				sendMsg(senderSockfd, "/rejectfile");
			}
			else
			{
				std::cout << "File accepted." << std::endl;
				sendMsg(senderSockfd, "/acceptfile");

				std::this_thread::sleep_for(std::chrono::milliseconds(50));

				receiveFile(senderSockfd, fileName);
			}
		}
		else
		{
			sendAllExcept(senderSockfd, receivedData);
		}
	}
}

int Server::waitForAcceptFile(int senderSockfd)
{
	for(int sockfdItr {0}; sockfdItr <= m_sockfdCount; ++sockfdItr) // Temporary solution to wait for /acceptfile
	{
		if(FD_ISSET(sockfdItr, &m_master))
		{
			if(sockfdItr != m_listeningSockfd && sockfdItr != senderSockfd)
			{
				unsigned int maxBufferSize {1024};
				char buffer [maxBufferSize];

				memset(buffer, 0, maxBufferSize);
				long int bytesReceived {recv(sockfdItr, buffer, maxBufferSize, 0)};

				std::string data {buffer};
				if(data.find("/acceptfile") == 0)
					return 0;
				else if(data.find("/rejectfile") == 0)
					return -1;
			}
		}
	}
	return -1;
}

void Server::receiveFile(int sourceSockfd, const std::string& fileName)
{
	unsigned int maxBufferSize {1024};
	char buffer [maxBufferSize];

	std::cout << "Starting to receive and send a file." << std::endl;
	while(true)
	{
		memset(buffer, 0, maxBufferSize);
		long int bytesReceived {recv(sourceSockfd, buffer, maxBufferSize, 0)};
		if(bytesReceived <= -1)
		{
			std::cerr << "Error: receiving file!" << std::endl;
			break;
		}
		else if(bytesReceived == 0)
		{
			std::cout << "Client disconnected." << std::endl;
			removeSocket(sourceSockfd);
			break;
		}
		else
		{
			std::string data {buffer};
			if(data.find("/endfile") == 0)
			{
				sendAllExcept(sourceSockfd, data);
				break;
			}

			sendAllExcept(sourceSockfd, buffer);
		}
	}
	std::cout << "Finished sending file.\n";
}