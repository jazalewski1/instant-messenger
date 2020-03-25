#pragma once

#include <Host.hpp>
#include <arpa/inet.h>
#include <string>
#include <thread>

class Client : public Host
{
private:
	bool m_isReceiveThreadRunning;
	std::thread m_receiveThread;
	bool m_receiveThreadWantsInput;
	std::string m_receiveInput;

protected:
	void startReceiving();

	void receiveHandler(const char* buffer, long int receivedBytes) override;

	int receiveFile(const std::string& fileName);

	int sendFile(const std::string& filePath);

	int waitForAcceptFile();

public:
	Client(const std::string& ipAddress, int portNumber);

	~Client();

	int start();
};