#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <string>
#include <string.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>

class Listener
{
private:
	std::string m_port;
	addrinfo m_hint;
	int m_listenSockfd;
	fd_set m_master;
	int m_sockfdCount;

private:
	int createListeningSocket();

	int acceptClient();

	void removeSocket(int sockfd);

	void sendAll(int sourceSockfd, const std::string& msg);

	void sendMsg(int receiverSockfd, const std::string& msg);

public:
	Listener(int portNumber);

	~Listener();

	void run();

	static void displayInfo(const std::string& name, sockaddr_in* saddrPtr);
};