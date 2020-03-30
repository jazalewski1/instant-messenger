#pragma once

#include <Data.hpp>
#include <Listener.hpp>
#include <iostream>
#include <string>
#include <thread>

class Server
{
private:
	IListener* m_listener;

public:
	Server(IListener* listener);

	int connect(const std::string& portNumber);

	int addClient();

	int removeClient(int sockfd);

	int poll();

	Data receive(int sourceFd);

	int sendTo(int destinationFd, const std::string& data);

	int sendExcept(int exceptFd, const std::string& data);

	int waitForAcceptFile(int sourceFd);

	int transferFile(int sourceFd);
};