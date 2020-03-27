#pragma once

#include <Host.hpp>
#include <arpa/inet.h>
#include <string>
#include <thread>

class Client
{
private:
	IHost* m_host;
	bool m_isInputThreadRunning;
	bool m_isReceiveThreadRunning;
	std::thread m_receiveThread;
	bool m_receiveThreadWantsInput;
	std::string m_receiveInput;

private:
	void startReceiving();

	int waitForAcceptFile();

public:
	Client(IHost* host);

	~Client();

	int connect(const std::string& ipAddress, int portNumber);

	void start();

	int update(const std::string& input);

	int acceptFile(const std::string& fileName, std::ofstream& outFile);

	long int receiveFile(std::ostream& outFile);

	int requestSendFile(const std::string& filePath, std::ifstream& inFile);

	int sendFile(std::ifstream& inFile);
};