#pragma once

#include <arpa/inet.h>
#include <string>
#include <thread>

class Client
{
private:
	std::string m_serverIpAddress;
	int m_serverPortNumber;
	int m_sockfd;
	sockaddr_in m_hint;
	bool m_isReceiveThreadRunning;
	std::thread m_receiveThread;

	int createSocket();

	int conn();

	long int sendAll(const std::string& msg);

public:
	Client(const std::string& ipAddress, int portNumber);

	~Client();

	void run();

	void startReceiving();

	void receive();

	static void displayInfo(const std::string& name, sockaddr_in* saddrPtr);
};