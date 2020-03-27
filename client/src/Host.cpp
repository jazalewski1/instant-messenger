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

Host::Host() :
	m_sockfd{-1}
{
}

Host::~Host()
{
	close(m_sockfd);
}

// RETURNS: sockfd on success; -1 on error
int Host::createSocket()
{
	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	return m_sockfd;
}

// RETURNS: 0 on success; -1 on error
int Host::conn(const std::string& ipAddress, int portNumber)
{
	sockaddr_in hints;
	hints.sin_family = AF_INET;
	hints.sin_port = htons(portNumber);
	inet_pton(AF_INET, ipAddress.c_str(), &hints.sin_addr);

	displayInfo("Server", &hints); // TODO: move elsewhere, best to Client

	return connect(m_sockfd, (sockaddr*)&hints, sizeof(hints));
}

// RETURNS: received bytes on success; 0 on disconnection; -1 on error; -2 on timeout
long int Host::receiveNonblocking(char* buffer, int bufferSize)
{
	pollfd pfd;
	pfd.fd = m_sockfd;
	pfd.events = POLLIN;
	int pollResult {poll(&pfd, 1, 100)};
	if(pollResult == -1)
		return -1;
	else if(pollResult == 0)
		return -2;

	memset(buffer, 0, bufferSize);
	return recv(m_sockfd, buffer, bufferSize, 0);
}

long int Host::receiveBlocking(char* buffer, int bufferSize)
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