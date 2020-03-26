#pragma once

#include <string>

class IListener
{
public:
	virtual int createListeningSocket(const std::string& portNumber) = 0;

	virtual int startListening() = 0;

	virtual int acceptHost() = 0;

	virtual int removeSocket(int sockfd) = 0;

	virtual int poll() = 0;

	virtual long int receive(int sockfd, char* buffer, unsigned int bufferSize) = 0;

	virtual long int sendData(int receiverSockfd, const std::string& data) = 0;

	virtual long int sendAll(const std::string& data) = 0;

	virtual long int sendAllExcept(int exceptSockfd, const std::string& data) = 0;
};