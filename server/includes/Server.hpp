#pragma once

#include <Listener.hpp>
#include <iostream>
#include <string>
#include <thread>

class Server
{
private:
	IListener* m_listener;
	std::thread m_pollThread;
	bool m_isPollThreadRunning;

private:
	void startPolling();

	void receive(int senderSockfd, const std::string& receivedData);

	void transferFile(int sourceSockfd, const std::string& fileName);

	int waitForAcceptFile(int senderSockfd);

public:
	Server(IListener* listener);

	~Server();

	int connect(const std::string& portNumber);

	int start();
};