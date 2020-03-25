#pragma once

#include <arpa/inet.h>
#include <string>

class Host
{
protected:
	std::string m_serverIpAddress;
	int m_serverPortNumber;
	int m_sockfd;
	sockaddr_in m_hint;

protected:
	int createSocket();

	int conn();

	long int receive(void* buffer, int bufferSize);

	virtual void receiveHandler(const char* buffer, long int receivedBytes) = 0;

	long int sendMsg(const std::string& msg);

public:
	Host(const std::string& ipAddress, int portNumber);

	virtual ~Host();

	static void displayInfo(const std::string& name, sockaddr_in* saddrPtr);
};