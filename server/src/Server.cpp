#include <Server.hpp>
#include <algorithm>
#include <iostream>
#include <string>


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
	
	std::cout << "waiting for input" << std::endl;
	while(true)
	{
		std::getline(std::cin, input);
		if(input == "/close")
			break;
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
		sendMsg(senderSockfd, "Disconnecting from server.");
		removeSocket(senderSockfd);
	}
	else
	{
		std::string receivedData {receiveBuffer};
		std::cout << "CLIENT #" << senderSockfd << "> " << receivedData << std::endl;

		if(receivedData.find("/sendfile") == 0)
		{
			std::cout << "Transfering file." << std::endl;

			std::string fileName {receivedData.begin() + receivedData.find_first_not_of("/sendfile "), receivedData.end()};
			std::cout << "Filename: \"" << fileName << "\"" << std::endl;

			sendAllExcept(senderSockfd, "/receivefile " + fileName);
			receiveFile(senderSockfd, fileName);
		}
		else
		{
			sendAllExcept(senderSockfd, receivedData);
		}
	}
}

void Server::receiveFile(int sourceSockfd, const std::string& fileName)
{
	std::cout << "Server::receiveFile()" << std::endl;
	std::cout << "from socket = " << sourceSockfd << std::endl;
	std::cout << "filename = \"" << fileName << "\"" << std::endl;

	for(int i {0}; i < 32; ++i)
	{
		sendAllExcept(sourceSockfd, "THATSAFILE" + std::to_string(i));
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}