#include <Host.hpp>
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Host::Host(const std::string& ipAddress, int portNumber) :
		m_serverIpAddress{ipAddress}, m_serverPortNumber{portNumber},
		m_sockfd{-1}
{
}

Host::~Host()
{
	close(m_sockfd);
}

int Host::createSocket()
{
	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	return m_sockfd;
}

int Host::conn()
{
	m_hint.sin_family = AF_INET;
	m_hint.sin_port = htons(m_serverPortNumber);
	inet_pton(AF_INET, m_serverIpAddress.c_str(), &m_hint.sin_addr);

	displayInfo("Server", &m_hint);

	return connect(m_sockfd, (sockaddr*)&m_hint, sizeof(m_hint));
}

long int Host::receiveNonblocking(void* buffer, int bufferSize)
{
	// Poll socket with timeout
	pollfd pfd;
	pfd.fd = m_sockfd;
	pfd.events = POLLIN;
	int pollResult {poll(&pfd, 1, 100)};
	if(pollResult == -1)
	{
		// Error
		return -1;
	}
	else if(pollResult == 0)
	{
		// Timeout
		return -2;
	}

	memset(buffer, 0, bufferSize);
	return recv(m_sockfd, buffer, bufferSize, 0);
}

long int Host::receiveBlocking(void* buffer, int bufferSize)
{
	memset(buffer, 0, bufferSize);
	return recv(m_sockfd, buffer, bufferSize, 0);
}

long int Host::sendData(const std::string& data)
{
	return send(m_sockfd, data.c_str(), static_cast<int>(data.length()), 0);
}


void Host::displayInfo(const std::string& name, sockaddr_in* saddrPtr)
{
	char ip [NI_MAXHOST];
	memset(ip, 0, NI_MAXHOST);
	char port [NI_MAXSERV];
	memset(port, 0, NI_MAXSERV);

	getnameinfo((sockaddr*)&saddrPtr, sizeof(sockaddr_in), ip, NI_MAXHOST, nullptr, 0, 0);

	inet_ntop(AF_INET, &saddrPtr->sin_addr, ip, NI_MAXHOST);
	std::cout << name << " IP: " << ip << ", port: " << ntohs(saddrPtr->sin_port) << std::endl;
}