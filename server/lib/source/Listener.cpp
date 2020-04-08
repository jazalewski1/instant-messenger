#include <Listener.hpp>
#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <string>
#include <string.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>

Listener::Listener() :
	m_listeningSockfd{-1}, m_sockfdCount{0}
{
}

Listener::~Listener()
{
	for(int sockfdItr {0}; sockfdItr <= m_sockfdCount; ++sockfdItr)
	{
		if(FD_ISSET(sockfdItr, &m_master))
		{
			if(sockfdItr != m_listeningSockfd)
				removeSocket(sockfdItr);
		}
	}

	if(m_listeningSockfd != -1)
		removeSocket(m_listeningSockfd);
}

// RETURNS: 0 on success; -1 on getaddrinfo() error; -2 on binding error
int Listener::createListeningSocket(const std::string& portNumber)
{
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	addrinfo* listBegin;
	int addrInfoResult {getaddrinfo(nullptr, portNumber.c_str(), &hints, &listBegin)};
	if(addrInfoResult != 0)
		return -1;

	addrinfo* listResult;
	for(listResult = listBegin; listResult != nullptr; listResult = listResult->ai_next)
	{
		m_listeningSockfd = socket(listResult->ai_family, listResult->ai_socktype, listResult->ai_protocol);
		if(m_listeningSockfd == -1)
			continue;

		int sockopt {1};
		setsockopt(m_listeningSockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));

		int bindResult {bind(m_listeningSockfd, listResult->ai_addr, listResult->ai_addrlen)};
		if(bindResult == 0)
			break;

		close(m_listeningSockfd);
	}

	if(listResult == nullptr)
		return -2;

	freeaddrinfo(listBegin);

	std::cout << "Listening socket #" << m_listeningSockfd << std::endl;

	displayInfo("Server", (sockaddr_in*)listResult->ai_addr);
	return 0;
}

// RETURNS: 0 on success; -1 on listen() error
int Listener::startListening()
{
	int listeningResult {listen(m_listeningSockfd, 2)};
	if(listeningResult <= -1)
		return -1;

	m_sockfdCount = m_listeningSockfd;

	FD_ZERO(&m_master);
	FD_SET(m_listeningSockfd, &m_master);

	return 0;
}

// RETURNS: new host's sockfd on connection; -1 on accept() error
int Listener::acceptHost()
{
	sockaddr_storage remoteAddr;
	socklen_t addrSize {sizeof(remoteAddr)};

	int newSockfd {accept(m_listeningSockfd, (sockaddr*)&remoteAddr, &addrSize)};
	if(newSockfd <= -1)
		return -1;

	FD_SET(newSockfd, &m_master);
	displayInfo("Client", (sockaddr_in*)&remoteAddr);

	if(newSockfd > m_sockfdCount)
		m_sockfdCount = newSockfd;

	return newSockfd;
}

// RETURNS: 0 on success, -1 on close() error
int Listener::removeSocket(int sockfd)
{
	FD_CLR(sockfd, &m_master);
	return close(sockfd);
}

// RETURNS: 0 on connection request; sender's sockfd on success; -1 on select() error; -2 on timeout
int Listener::poll()
{
	fd_set copySet {m_master};

	timeval timeout;
	timeout.tv_usec = 1000;
	timeout.tv_sec = 0;
	int selectResult {select(m_sockfdCount + 1, &copySet, nullptr, nullptr, &timeout)};
	if(selectResult == 0)
		return -2;
	if(selectResult == -1)
		return -1;

	for(int sockfdItr {0}; sockfdItr <= m_sockfdCount; ++sockfdItr)
	{
		if(FD_ISSET(sockfdItr, &copySet))
		{
			if(sockfdItr == m_listeningSockfd)
				return 0; // It's a connection request
			else
				return sockfdItr; // It's a sending data request
		}
	}
	return -2;
}

long int Listener::receive(int sockfd, char* buffer, unsigned int bufferSize)
{
	memset(buffer, 0, bufferSize);

	return recv(sockfd, buffer, bufferSize, 0);
}

long int Listener::sendData(int receiverSockfd, const std::string& data)
{
	return send(receiverSockfd, data.c_str(), data.size(), 0);
}

long int Listener::sendAll(const std::string& data)
{
	for(int sockfdItr {0}; sockfdItr <= m_sockfdCount; ++sockfdItr)
	{
		if(FD_ISSET(sockfdItr, &m_master))
		{
			if(sockfdItr != m_listeningSockfd)
			{
				if(sendData(sockfdItr, data) == -1)
					return -1;
			}
		}
	}
	return 0;
}

long int Listener::sendAllExcept(int exceptSockfd, const std::string& data)
{
	for(int sockfdItr {0}; sockfdItr <= m_sockfdCount; ++sockfdItr)
	{
		if(FD_ISSET(sockfdItr, &m_master))
		{
			if(sockfdItr != m_listeningSockfd && sockfdItr != exceptSockfd)
			{
				if(sendData(sockfdItr, data) == -1)
					return -1;
			}
		}
	}
	return 0;
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
