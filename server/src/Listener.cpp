#include <Listener.hpp>
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

// RETURNS: 0 on success, -1 on getaddrinfo() error, -2 on binding error
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

	displayInfo("Server", (sockaddr_in*)listResult->ai_addr);
	return 0;
}

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

int Listener::acceptHost()
{
	sockaddr_storage remoteAddr;
	socklen_t addrSize {sizeof(remoteAddr)};

	int newSockfd {accept(m_listeningSockfd, (sockaddr*)&remoteAddr, &addrSize)};
	if(newSockfd <= -1)
		return -1;

	FD_SET(newSockfd, &m_master);
	displayInfo("Client", (sockaddr_in*)&remoteAddr);

	return newSockfd;
}

void Listener::removeSocket(int sockfd)
{
	FD_CLR(sockfd, &m_master);
	close(sockfd);
}

// RETURNS: 0 on success, -1 on select() error, -2 on acceptHost() error
int Listener::poll()
{
	unsigned int maxBufferSize {4096};
	char buffer [maxBufferSize];

	fd_set copySet {m_master};

	timeval timeout;
	timeout.tv_usec = 1000;
	int socketCount {select(m_sockfdCount + 1, &copySet, nullptr, nullptr, &timeout)};
	if(socketCount == -1)
		return -1;

	for(int sockfdItr {0}; sockfdItr <= m_sockfdCount; ++sockfdItr)
	{
		if(FD_ISSET(sockfdItr, &copySet))
		{
			if(sockfdItr == m_listeningSockfd)
			{
				int newSockfd {acceptHost()};
				if(newSockfd <= -1)
					return -2;

				if(newSockfd > m_sockfdCount)
					m_sockfdCount = newSockfd;

				std::cout << "Connected new client. Socket #" << newSockfd << std::endl;
			}
			else
			{
				memset(buffer, 0, maxBufferSize);
				
				long int bytesReceived {recv(sockfdItr, buffer, maxBufferSize, 0)};
				receiveHandler(buffer, bytesReceived, sockfdItr);
			}
		}
	}

	return 0;
}

long int Listener::sendMsg(int receiverSockfd, const std::string& msg)
{
	return send(receiverSockfd, msg.c_str(), msg.size(), 0);
}

long int Listener::sendAll(const std::string& msg)
{
	for(int sockfdItr {0}; sockfdItr <= m_sockfdCount; ++sockfdItr)
	{
		if(FD_ISSET(sockfdItr, &m_master))
		{
			if(sockfdItr != m_listeningSockfd)
				sendMsg(sockfdItr, msg);
		}
	}
}

long int Listener::sendAllExcept(int exceptSockfd, const std::string& msg)
{
	for(int sockfdItr {0}; sockfdItr <= m_sockfdCount; ++sockfdItr)
	{
		if(FD_ISSET(sockfdItr, &m_master))
		{
			if(sockfdItr != m_listeningSockfd && sockfdItr != exceptSockfd)
				sendMsg(sockfdItr, msg);
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
