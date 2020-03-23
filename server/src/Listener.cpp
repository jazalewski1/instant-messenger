#include <Listener.hpp>
#include <algorithm>
#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <string>
#include <string.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>


Listener::Listener(int portNumber) :
	m_port{std::to_string(portNumber)}, 
	m_listenSockfd{-1}, m_sockfdCount{0},
	m_isThreadRunning{false}
{
}

Listener::~Listener()
{
	std::string msg {"Server is shutting down."};
	for(int sockfdItr {0}; sockfdItr <= m_sockfdCount; ++sockfdItr)
	{
		if(FD_ISSET(sockfdItr, &m_master))
		{
			if(sockfdItr != m_listenSockfd)
			{
				sendMsg(sockfdItr, msg);
				removeSocket(sockfdItr);
			}
		}
	}

	if(m_listenSockfd != -1)
	{
		removeSocket(m_listenSockfd);
	}

	if(m_isThreadRunning && m_thread.joinable())
	{
		m_isThreadRunning = false;

		m_thread.join();
	}

	std::cout << "Closing." << std::endl;
}

int Listener::createListeningSocket()
{
	memset(&m_hint, 0, sizeof(m_hint));

	m_hint.ai_family = AF_INET;
	m_hint.ai_socktype = SOCK_STREAM;
	m_hint.ai_flags = AI_PASSIVE;

	addrinfo* listBegin;
	int addrInfoResult {getaddrinfo(nullptr, m_port.c_str(), &m_hint, &listBegin)};
	if(addrInfoResult != 0)
		return -1;

	addrinfo* listResult;
	for(listResult = listBegin; listResult != nullptr; listResult = listResult->ai_next)
	{
		m_listenSockfd = socket(listResult->ai_family, listResult->ai_socktype, listResult->ai_protocol);
		if(m_listenSockfd == -1)
			continue;

		int sockopt {1};
		if(setsockopt(m_listenSockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) == -1)
		{
			std::cerr << "Error: setsockopt." << std::endl;
		}

		int bindResult {bind(m_listenSockfd, listResult->ai_addr, listResult->ai_addrlen)};
		if(bindResult == -1)
		{
			close(m_listenSockfd);
			continue;
		}

		break;
	}

	if(listResult == nullptr)
		return -1;

	freeaddrinfo(listBegin);

	displayInfo("Server", (sockaddr_in*)listResult->ai_addr);

	int listenResult {listen(m_listenSockfd, 4)};
	if(listenResult <= -1)
		return listenResult;

	return m_listenSockfd;
}

int Listener::acceptClient()
{
	sockaddr_storage remoteAddr;
	socklen_t addrSize {sizeof(remoteAddr)};
	int newSockfd {accept(m_listenSockfd, (sockaddr*)&remoteAddr, &addrSize)};

	if(newSockfd <= -1)
	{
		std::cerr << "Error: can't accept client!" << std::endl;
	}
	else
	{
		FD_SET(newSockfd, &m_master);
		std::cout << "Connected new client. Socket #" << newSockfd << std::endl;
		displayInfo("Client", (sockaddr_in*)&remoteAddr);
	}
	return newSockfd;
}

void Listener::removeSocket(int sockfd)
{
	FD_CLR(sockfd, &m_master);
	close(sockfd);
}

void Listener::receiveFile(int sourceSockfd, const std::string& fileName)
{
	unsigned int maxBufferSize {1024};
	char buffer [maxBufferSize];

	while(true)
	{
		memset(buffer, 0, maxBufferSize);
		long int bytesReceived {recv(sourceSockfd, buffer, maxBufferSize, 0)};
		if(bytesReceived <= -1)
		{
			std::cerr << "Error: receiving data! Socket #" << sourceSockfd << std::endl;
			break;
		}
		else if(bytesReceived == 0)
		{
			std::cout << "Client disconnected. Socket #" << sourceSockfd << std::endl;
			sendMsg(sourceSockfd, "Disconnecting from server.");
			removeSocket(sourceSockfd);
			break;
		}
		else
		{
			std::string data {buffer};
			if(data == "/endfile")
				break;

			sendAll(sourceSockfd, buffer);
		}
	}
	std::cout << "Finished sending file.\n";
	sendAll(sourceSockfd, "/endfile");
}

void Listener::sendAll(int sourceSockfd, const std::string& msg)
{
	for(int sockfdItr {0}; sockfdItr <= m_sockfdCount; ++sockfdItr)
	{
		if(FD_ISSET(sockfdItr, &m_master))
		{
			if(sockfdItr != m_listenSockfd && sockfdItr != sourceSockfd)
				sendMsg(sockfdItr, msg);
		}
	}
}

void Listener::sendMsg(int receiverSockfd, const std::string& msg)
{
	long int bytesSent {send(receiverSockfd, msg.c_str(), msg.size(), 0)};
	if(bytesSent <= -1)
	{
		std::cerr << "Error: can't send message!" << std::endl;
	}
}

void Listener::startRunning()
{
	m_thread = std::thread{[&]() { this->run(); }};
	m_thread.detach();
}

void Listener::run()
{
	int listenResult {createListeningSocket()};
	if(listenResult <= -1)
	{
		std::cerr << "Error: can't create listening socket!" << std::endl;
		return;
	}
	m_sockfdCount = m_listenSockfd;

	FD_ZERO(&m_master);
	FD_SET(m_listenSockfd, &m_master);


	unsigned int maxBufferSize {4096};
	char buffer [maxBufferSize];
	
	m_isThreadRunning = true;
	while(m_isThreadRunning)
	{
		fd_set copySet {m_master};
		int socketCount {select(m_sockfdCount + 1, &copySet, nullptr, nullptr, nullptr)};
		if(socketCount == -1)
		{
			std::cerr << "Error: select socket!" << std::endl;
			break;	
		}


		for(int sockfdItr {0}; sockfdItr <= m_sockfdCount; ++sockfdItr)
		{
			if(FD_ISSET(sockfdItr, &copySet))
			{
				if(sockfdItr == m_listenSockfd)
				{
					int newSockfd {acceptClient()};
					if(newSockfd > m_sockfdCount)
						m_sockfdCount = newSockfd;
				}
				else
				{
					memset(buffer, 0, maxBufferSize);
					
					long int bytesReceived {recv(sockfdItr, buffer, maxBufferSize, 0)};
					if(bytesReceived <= -1)
					{
						std::cerr << "Error: receiving data! Socket #" << sockfdItr << std::endl;
					}
					else if(bytesReceived == 0)
					{
						std::cout << "Client disconnected. Socket #" << sockfdItr << std::endl;
						sendMsg(sockfdItr, "Disconnecting from server.");
						removeSocket(sockfdItr);
					}
					else
					{
						std::string receivedData {buffer};
						std::cout << "CLIENT #" << sockfdItr << "> " << receivedData << std::endl;

						if(receivedData.find("/sendfile") == 0)
						{
							std::cout << "Transfering file." << std::endl;

							std::string fileName {receivedData.begin() + receivedData.find_first_not_of("/sendfile "), receivedData.end()};
							std::cout << "Filename: " << fileName << std::endl;

							sendAll(sockfdItr, "/receivefile " + fileName);
							receiveFile(sockfdItr, fileName);
						}
						else
							sendAll(sockfdItr, receivedData);
					}
				}
			}
		}
	}
}

void Listener::displayInfo(const std::string& name, sockaddr_in* saddrPtr)
{
	char ip [NI_MAXHOST];
	memset(ip, 0, NI_MAXHOST);
	char port [NI_MAXSERV];
	memset(port, 0, NI_MAXSERV);

	getnameinfo((sockaddr*)&saddrPtr, sizeof(sockaddr_in), 
				ip, NI_MAXHOST, 
				nullptr, 0, 0);

	inet_ntop(AF_INET, &saddrPtr->sin_addr, ip, NI_MAXHOST);
	std::cout << name << "IP: " << ip << ", port: " << ntohs(saddrPtr->sin_port) << std::endl;
}