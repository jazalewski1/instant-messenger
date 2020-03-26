#pragma once

#include <IListener.hpp>
#include <iostream>
#include <netdb.h>
#include <string>

class Listener : public IListener
{
private:
	int m_listeningSockfd;
	int m_sockfdCount;
	fd_set m_master;

public:
	Listener();

	~Listener();

	int createListeningSocket(const std::string& portNumber) override;

	int startListening() override;

	int acceptHost() override;

	int removeSocket(int sockfd) override;

	int poll() override;

	long int receive(int sockfd, char* buffer, unsigned int bufferSize) override;

	long int sendData(int receiverSockfd, const std::string& data) override;

	long int sendAll(const std::string& data) override;

	long int sendAllExcept(int exceptSockfd, const std::string& data) override;

	static void displayInfo(const std::string& name, sockaddr_in* saddrPtr);
};