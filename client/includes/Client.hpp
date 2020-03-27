#pragma once

#include <Host.hpp>
#include <arpa/inet.h>
#include <string>
#include <thread>

class Client
{
private:
	IHost* m_host;
	bool m_isReceiveThreadRunning;
	std::thread m_receiveThread;
	bool m_receiveThreadWantsInput;
	std::string m_receiveInput;

private:
	void startReceiving();

	int receiveFile(const std::string& fileName);

	int sendFile(const std::string& filePath);

	int waitForAcceptFile();

public:
	Client(IHost* host);

	~Client();

	int connect(const std::string& ipAddress, int portNumber);

	int start();
};