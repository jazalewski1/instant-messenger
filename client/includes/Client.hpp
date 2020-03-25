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

protected:
	void startReceiving();

	void receiveHandler(const char* buffer, long int receivedBytes) override;

	void receiveFile(const std::string& fileName);

	void sendFile(const std::string& fileName);

public:
	Client(const std::string& ipAddress, int portNumber);

	~Client();

	int start();
};