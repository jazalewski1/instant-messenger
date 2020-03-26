#pragma once

#include <iostream>
#include <netdb.h>
#include <string>

class Listener
{
protected:
	std::string m_port;
	addrinfo m_hint;
	int m_listeningSockfd;
	fd_set m_master;
	int m_sockfdCount;

protected:
	int createListeningSocket();

	int startListening();

	int acceptHost();

	void removeSocket(int sockfd);

	int poll();

	virtual void receiveHandler(const char* receiveBuffer, long int receivedBytes, int senderSockfd) = 0;

	long int sendData(int receiverSockfd, const std::string& data);

	long int sendAll(const std::string& data);

	long int sendAllExcept(int exceptSockfd, const std::string& data);

public:
	Listener(int portNumber);

	virtual ~Listener();

	static void displayInfo(const std::string& name, sockaddr_in* saddrPtr);
};