#include <Client.hpp>
#include <Host.hpp>
#include <arpa/inet.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <netdb.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

Client::Client(IHost* host) :
	m_host{host},
	m_isReceiveThreadRunning{false},
	m_receiveThreadWantsInput{false},
	m_receiveInput{}
{
}

Client::~Client()
{
	m_isReceiveThreadRunning = false;
	if(m_receiveThread.joinable())
		m_receiveThread.join();

	std::cout << "Closing client." << std::endl;
}

// RETURNS: 0 on success; -1 on error
int Client::connect(const std::string& ipAddress, int portNumber)
{
	if(m_host->createSocket() <= -1)
	{
		std::cerr << "Erorr: can't create a socket!" << std::endl;
		return -1;
	}

	if(m_host->conn(ipAddress, portNumber) <= -1)
	{
		std::cerr << "Error: can't connect!" << std::endl;
		return -1;
	}

	return 0;
}

void Client::start()
{
	m_isReceiveThreadRunning = true;
	m_receiveThread = std::thread{&Client::startReceiving, this};
}

int Client::update(const std::string& input)
{
	if(m_receiveThreadWantsInput)
	{
		m_receiveInput = input;
		m_receiveThreadWantsInput = false;
		return 1;
	}

	if(input.find("/close") == 0)
	{
		return 0;
	}
	else if(input.find("/sendfile") == 0)
	{
		long unsigned int whiteIndex {input.find_first_not_of("/sendfile")};
		if(whiteIndex == std::string::npos || whiteIndex == input.length() - 1)
		{
			std::cerr << "Usage: \"/sendfile <absolutePathToFile>\"" << std::endl;
			return 1;
		}
		else
		{
			std::string filePath {input.begin() + whiteIndex + 1, input.end()};

			std::ifstream inFile;
			int requestResult {requestSendFile(filePath, inFile)};
			if(requestResult <= -1)
			{
				std::cerr << "Error: request failed!" << std::endl;
				return 1;
			}

			int sendFileResult {sendFile(inFile)};
			if(sendFileResult <= -1)
			{
				std::cerr << "Error: failed to send file!" << std::endl;
				return 0;
			}
			inFile.close();

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			m_host->sendData("/endfile");

			std::cout << "File sent succesfully." << std::endl;
		}
	}
	else if(input.find("/") == 0)
	{
		std::cerr << "Unknown command: " << input << std::endl;
		return 1;
	}
	else
	{
		long int sentBytes {m_host->sendData(input)};
		if(sentBytes == -1)
		{
			std::cerr << "Error: can't send data!" << std::endl;
			return -1;
		}
		else if(sentBytes == 0)
		{
			std::cout << "Disconnecting..." << std::endl;
			return -1;
		}
	}
	return 1;
}

void Client::startReceiving()
{
	while(m_isReceiveThreadRunning)
	{
		unsigned int bufferSize {4096};
		char buffer [bufferSize];

		long int receivedBytes {m_host->receiveNonblocking(buffer, static_cast<int>(bufferSize))};
		if(receivedBytes == -1)
		{
			std::cerr << "Error: can't receive data!" << std::endl;
			m_isReceiveThreadRunning = false;
			break;
		}
		else if(receivedBytes == 0)
		{
			std::cerr << "Server shut down." << std::endl;
			m_isReceiveThreadRunning = false;
			break;
		}
		else if(receivedBytes > 0)
		{
			std::string receivedData {buffer};

			if(receivedData.find("/receivefile") == 0)
			{
				std::string fileName {receivedData.begin() + receivedData.find_first_not_of("/receivefile "), receivedData.end()};

				std::ofstream outFile;

				int acceptFileResult {acceptFile(fileName, outFile)};
				if(acceptFileResult <= -1)
				{
					std::cout << "File not accepted." << std::endl;
					m_host->sendData("/rejectfile");
				}
				else
				{
					m_host->sendData("/acceptfile");
					std::cout << "Starting to receive data." << std::endl;

					long int fileReceivedBytes {receiveFile(outFile)};
					if(fileReceivedBytes <= -1)
					{
						std::cerr << "Error: can't receive file!" << std::endl;
					}
					else
					{
						std::cout << "Total received bytes: " << fileReceivedBytes << std::endl;
					}
				}
				outFile.close();
			}
			else
			{
				std::cout << "CHAT> " << receivedData << std::endl;
			}
		}
	}
}

// RETURNS: 0 on accepted; -1 on rejected
int Client::acceptFile(const std::string& fileName, std::ofstream& outFile)
{
	std::cout << "Incoming file. Filename: \"" << fileName << "\"" << std::endl;
	std::cout << "Do you accept? [Y / N]" << std::endl;

	m_receiveThreadWantsInput = true;
	while(m_receiveThreadWantsInput)
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	std::string input = m_receiveInput;
	m_receiveInput.clear();

	if(input.find("Y") != 0)
		return -1;

	std::cout << "Enter absolute path to save file: " << std::endl;
	m_receiveThreadWantsInput = true;
	while(m_receiveThreadWantsInput)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::string filePath {m_receiveInput};
	m_receiveInput.clear();

	filePath.append(fileName);

	outFile.open(filePath);

	if(!outFile.is_open())
	{
		std::cerr << "Error: can't open file! Path:" << std::endl;
		std::cout << filePath << std::endl;
		return -1;
	}
	return 0;
}

// RETURNS: total received bytes on success; -1 on error
long int Client::receiveFile(std::ostream& outFile)
{
	unsigned int bufferSize {4096};
	char buffer [bufferSize];
	long int totalReceivedBytes {0};
	while(true)
	{
		memset(buffer, 0, bufferSize);

		long int receivedBytes {m_host->receiveNonblocking(buffer, bufferSize)}; // TODO: check if blocking works, because we don't want messages to be saved in a file
		if(receivedBytes == -1)
		{
			std::cerr << "Error: failed to receive data!" << std::endl;
			return -1;
		}
		else if(receivedBytes == 0)
		{
			std::cerr << "Server shut down." << std::endl;
			m_isReceiveThreadRunning = false;
			return -1;
		}
		else if(receivedBytes > 0)
		{
			std::string str {buffer};
			if(str.find("/endfile") == 0)
				break;

			outFile << buffer << std::endl;
			totalReceivedBytes += receivedBytes;
		}
	}
	return totalReceivedBytes;
}

int Client::requestSendFile(const std::string& filePath, std::ifstream& inFile)
{
	inFile.open(filePath);

	std::string fileName {filePath.begin() + filePath.find_last_of('/') + 1, filePath.end()};

	if(!inFile.is_open())
	{
		std::cerr << "Error: can't open file: \"" << fileName << "\"!" << std::endl;
		return 1;
	}

	m_host->sendData("/sendfile " + fileName);
	if(waitForAcceptFile() <= -1)
	{
		std::cerr << "Error: file not accepted." << std::endl;
		return 1;
	}
	return 0;
}

// RETURNS: 0 on accepted; -1 on rejected
int Client::waitForAcceptFile()
{
	unsigned int bufferSize {4096};
	char buffer [bufferSize];
	memset(&buffer, 0, bufferSize);

	std::cout << "Waiting for accepting the file..." << std::endl;
	long int receivedBytes {m_host->receiveBlocking(buffer, bufferSize)};
	if(receivedBytes <= -1)
	{
		std::cerr << "Error: can't receive data!" << std::endl;
		m_isReceiveThreadRunning = false;
		return -1;
	}
	else if(receivedBytes == 0)
	{
		std::cerr << "Server shut down." << std::endl;
		m_isReceiveThreadRunning = false;
		return -1;
	}
	else if(receivedBytes > 0)
	{
		std::string data {buffer};
		if(data.find("/acceptfile") == 0)
			return 0;
		else if(data.find("/rejectfile") == 0)
			return -1;
	}
	return -1;
}

// RETURN: total sent bytes on success; -1 on error
int Client::sendFile(std::ifstream& inFile)
{
	std::cout << "Starting to read from file." << std::endl;
	std::vector<char> buffer (1024, 0);

	inFile.read(buffer.data(), buffer.size());
	std::streamsize s {inFile.gcount()};

	long int totalBytesSent {0};

	while(s > 0)
	{
		long int sentBytes {m_host->sendData(buffer.data())};
		if(sentBytes <= -1)
		{
			std::cerr << "Error: can't send data!" << std::endl;
			return -1;
		}
		totalBytesSent += sentBytes;

		inFile.read(buffer.data(), buffer.size());
		s = inFile.gcount();
	}
	return totalBytesSent;
}