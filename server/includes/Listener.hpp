#pragma once

#include <iostream>
#include <netdb.h>
#include <string>

class Listener
{
private:
	int m_listeningSockfd;
	int m_sockfdCount;
	fd_set m_master;

public:
	Listener();

	~Listener();

	int createListeningSocket(const std::string& portNumber);

	int startListening();

	int acceptHost();

	int removeSocket(int sockfd);

	int poll();

	long int receive(int sockfd, char* buffer, unsigned int bufferSize);

	long int sendData(int receiverSockfd, const std::string& data);

	long int sendAll(const std::string& data);

	long int sendAllExcept(int exceptSockfd, const std::string& data);

	static void displayInfo(const std::string& name, sockaddr_in* saddrPtr);
};