#pragma once

#include <arpa/inet.h>
#include <string>

class Host
{
private:
	int m_sockfd;

public:
	Host();

	~Host();

	int createSocket();

	int conn(const std::string& ipAddress, int portNumber);

	long int receiveNonblocking(char* buffer, int bufferSize);

	long int receiveBlocking(char* buffer, int bufferSize);

	long int sendData(const std::string& data);

	static void displayInfo(const std::string& name, sockaddr_in* saddrPtr);
};