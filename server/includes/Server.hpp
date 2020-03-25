#pragma once

#include <Listener.hpp>
#include <iostream>
#include <string>
#include <thread>

class Server : public Listener
{
private:
	std::thread m_pollThread;
	bool m_isPollThreadRunning;

private:
	void startPolling();

	void receiveHandler(const char* receiveBuffer, long int receivedBytes, int senderSockfd) override;

	void receiveFile(int sourceSockfd, const std::string& fileName);

	int waitForAcceptFile(int senderSockfd);

public:
	Server(int portNumber);

	~Server() override;

	int start();
};