#pragma once

#include <IHost.hpp>
#include <arpa/inet.h>
#include <string>

class Host : public IHost
{
private:
	int m_sockfd;

public:
	Host();

	~Host();

	int createSocket() override;

	int conn(const std::string& ipAddress, int portNumber) override;

	long int receiveNonblocking(char* buffer, int bufferSize) override;

	long int receiveBlocking(char* buffer, int bufferSize) override;

	long int sendData(const std::string& data) override;

	static void displayInfo(const std::string& name, sockaddr_in* saddrPtr);
};