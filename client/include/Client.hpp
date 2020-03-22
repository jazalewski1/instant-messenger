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

	void receiveFile(const std::string& fileName);

	long int sendData(const std::string& msg);

	void startSendingFile(FILE* filePtr);

	void sendFile(FILE* filePtr);

public:
	Client(const std::string& ipAddress, int portNumber);

	~Client();

	void run();

	static void displayInfo(const std::string& name, sockaddr_in* saddrPtr);
};