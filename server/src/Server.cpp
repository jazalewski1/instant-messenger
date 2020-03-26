#include <Listener.hpp>
#include <Server.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <string.h>

Server::Server(Listener* listener) :
	m_listener{listener},
	m_isPollThreadRunning{false}
{
}

Server::~Server()
{
	m_isPollThreadRunning = false;
	if(m_pollThread.joinable())
		m_pollThread.join();

	std::cout << "Closing server." << std::endl;
}

int Server::connect(const std::string& portNumber)
{
	int createListeningResult {m_listener->createListeningSocket(portNumber)};
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

	int listeningResult {m_listener->startListening()};
	if(listeningResult <= -1)
	{
		std::cerr << "Error: can't start listening!" << std::endl;
		return -1;
	}

	return 0;
}

int Server::start()
{
	m_pollThread = std::thread{&Server::startPolling, this};

	std::string input;
	std::cout << "\n(type \"/close\" to disconnect)\n\n";
	
	while(true)
	{
		std::getline(std::cin, input);
		if(input.find("/close") == 0)
			break;
	}
	return 0;
}

void Server::startPolling()
{
	m_isPollThreadRunning = true;
	while(m_isPollThreadRunning)
	{
		int pollResult {m_listener->poll()};
		if(pollResult == -1)
		{
			std::cerr << "Error: select socket!" << std::endl;
			m_isPollThreadRunning = false;
			break;
		}
		else if(pollResult == 0)
		{
			int acceptResult {m_listener->acceptHost()};
			if(acceptResult <= -1)
			{
				std::cerr << "Error: can't connect new client!" << std::endl;
				m_isPollThreadRunning = false;
				break;
			}
			else
			{
				std::cout << "New client connected on socket #" << acceptResult << std::endl;
			}
		}
		else if(pollResult > 0)
		{
			unsigned int bufferSize {4096};
			char buffer [bufferSize];

			long int receivedBytes {m_listener->receive(pollResult, buffer, bufferSize)};
			if(receivedBytes <= -1)
			{
				std::cerr << "Error: can't receive data! Socket #" << pollResult << std::endl;
			}
			else if(receivedBytes == 0)
			{
				std::cout << "Client disconnected. Socket #" << pollResult << std::endl;
				m_listener->removeSocket(pollResult);
			}
			else
			{
				std::string receivedData {buffer};
				receive(pollResult, receivedData);
			}
		}
	}
}

void Server::receive(int senderSockfd, const std::string& receivedData)
{
	std::cout << "CLIENT #" << senderSockfd << "> " << receivedData << std::endl;

	if(receivedData.find("/sendfile") == 0)
	{
		std::string fileName {receivedData.begin() + receivedData.find_first_not_of("/sendfile "), receivedData.end()};
		std::cout << "Request to send file: \"" << fileName << "\"" << std::endl;

		m_listener->sendAllExcept(senderSockfd, "/receivefile " + fileName);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if(waitForAcceptFile(senderSockfd) <= -1)
		{
			std::cout << "File not accepted." << std::endl;
			m_listener->sendData(senderSockfd, "/rejectfile");
		}
		else
		{
			std::cout << "File accepted." << std::endl;
			m_listener->sendData(senderSockfd, "/acceptfile");

			std::this_thread::sleep_for(std::chrono::milliseconds(50));

			transferFile(senderSockfd, fileName);
		}
	}
	else
	{
		m_listener->sendAllExcept(senderSockfd, receivedData);
	}
}

int Server::waitForAcceptFile(int senderSockfd)
{
	while(true)
	{
		int pollResult {m_listener->poll()};
		if(pollResult == -1)
		{
			std::cerr << "Error: select socket!" << std::endl;
			return -1;
		}
		else if(pollResult > 0)
		{
			if(pollResult != senderSockfd)
			{
				unsigned int bufferSize {4096};
				char buffer [bufferSize];

				long int receivedBytes {m_listener->receive(pollResult, buffer, bufferSize)};
				if(receivedBytes <= -1)
				{
					std::cerr << "Error: can't receive data! Socket #" << pollResult << std::endl;
					return -1;
				}
				else if(receivedBytes == 0)
				{
					std::cout << "Client disconnected. Socket #" << pollResult << std::endl;
					m_listener->removeSocket(pollResult);
					return -1;
				}
				else
				{
					std::string data {buffer};
					if(data.find("/acceptfile") == 0)
						return 0;
					else
						return -1;
				}
			}
		}
	}
	return -1;
}

void Server::transferFile(int sourceSockfd, const std::string& fileName)
{
	unsigned int bufferSize {1024};
	char buffer [bufferSize];

	std::cout << "Starting to receive and send a file." << std::endl;
	while(true)
	{
		memset(buffer, 0, bufferSize);
		long int bytesReceived {m_listener->receive(sourceSockfd, buffer, bufferSize)};
		if(bytesReceived <= -1)
		{
			std::cerr << "Error: receiving file!" << std::endl;
			break;
		}
		else if(bytesReceived == 0)
		{
			std::cout << "Client disconnected." << std::endl;
			m_listener->removeSocket(sourceSockfd);
			break;
		}
		else
		{
			std::string data {buffer};
			if(data.find("/endfile") == 0)
			{
				m_listener->sendAllExcept(sourceSockfd, data);
				break;
			}

			m_listener->sendAllExcept(sourceSockfd, data);
		}
	}
	std::cout << "Finished sending file.\n";
}