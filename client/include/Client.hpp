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

	void startReceiving();

	void receive();

	long int sendData(const std::string& msg);

	void startSendingFile(const std::string& fileName);

	void sendFile(const std::string& fileName);

public:
	Client(const std::string& ipAddress, int portNumber);

	~Client();

	void run();

	static void displayInfo(const std::string& name, sockaddr_in* saddrPtr);
};