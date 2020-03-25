#include <Client.hpp>
#include <algorithm>
#include <chrono>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <netdb.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

Client::Client(const std::string& ipAddress, int portNumber) :
	Host{ipAddress, portNumber},
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

int Client::start()
{
	int createSocketResult {createSocket()};
	if(createSocketResult <= -1)
	{
		std::cerr << "Error: can't create a socket!" << std::endl;
		return -1;
	}

	std::cout << "Connecting to server..." << std::endl;
	int connectResult {conn()};
	if(connectResult <= -1)
	{
		std::cerr << "Error: can't connect to server!" << std::endl;
		return -1;
	}
	std::cout << "Connected successfully." << std::endl;

	m_receiveThread = std::thread{&Client::startReceiving, this};

	std::string input;
	std::cout << "\n(type \"/close\" to disconnect)" << std::endl;
	std::cout << "(type \"/sendfile <aboluteFilePath>\" to send a file)" << std::endl;

	while(true)
	{
		std::cout << "YOU> ";
		std::getline(std::cin, input);
		if(m_receiveThreadWantsInput)
		{
			m_receiveInput = input;
			m_receiveThreadWantsInput = false;
			continue;
		}

		if(!input.empty())
		{
			if(input[0] == '/')
			{
				auto whiteItr {std::find_if(input.begin(), input.end(), [](char c)
				{
					return (c == ' ' ||  c == '\n'); 
				})};

				std::string command {input.begin(), whiteItr};


				if(command.find("/close") == 0)
				{
					m_isReceiveThreadRunning = false;
					return 0;
				}
				else if(command.find("/sendfile") == 0)
				{
					if(whiteItr == input.end())
					{
						std::cerr << "Usage: /sendfile <aboluteFilePath>" << std::endl;
					}
					else
					{
						std::string filePath {whiteItr + 1, input.end()};
						int sendFileResult {sendFile(filePath)};
						if(sendFileResult <= -1)
						{
							std::cout << "File not sent." << std::endl;
						}
					}
				}
				else
					std::cerr << "Unknown command: " << command << std::endl;
			}
			else
			{
				long int sentBytes {sendData(input)};
				if(sentBytes == -1)
				{
					std::cerr << "Error: can't send data!" << std::endl;
					break;
				}
				else if(sentBytes == 0)
				{
					std::cout << "Disconnecting..." << std::endl;
					break;
				}
			}
		}
	}
}

int Client::waitForAcceptFile()
{
	unsigned int bufferSize {4096};
	char buffer [bufferSize];
	memset(&buffer, 0, bufferSize);

	std::cout << "Waiting for accepting the file..." << std::endl;
	long int receivedBytes {receiveBlocking(buffer, bufferSize)};
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

void Client::startReceiving()
{
	m_isReceiveThreadRunning = true;
	while(m_isReceiveThreadRunning)
	{
		unsigned int bufferSize {4096};
		char buffer [bufferSize];

		long int receivedBytes {receiveNonblocking(buffer, static_cast<int>(bufferSize))};
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
			receiveHandler(buffer, receivedBytes);
		}
	}
}

void Client::receiveHandler(const char* buffer, long int receivedBytes)
{
	std::string receivedData {buffer, 0, static_cast<long unsigned int>(receivedBytes)};

	if(receivedData.find("/receivefile") == 0)
	{
		std::string fileName {receivedData.begin() + receivedData.find_first_not_of("/receivefile "), receivedData.end()};
		
		int receiveFileResult {receiveFile(fileName)};
		if(receiveFileResult <= -1)
		{
			std::cerr << "Error: file not received." << std::endl;
		}
	}
	else
	{
		std::cout << "CHAT> " << receivedData << std::endl;
	}
}

int Client::receiveFile(const std::string& fileName)
{
	std::cout << "Incoming file. Filename: \"" << fileName << "\"" << std::endl;
	std::cout << "Do you accept? [Y / N]" << std::endl;
	m_receiveThreadWantsInput = true;
	while(m_receiveThreadWantsInput)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::string input = m_receiveInput;
	m_receiveInput.clear();

	if(input.find("Y") != 0)
	{
		std::cout << "File not accepted." << std::endl;
		m_receiveThreadWantsInput = false;
		sendData("/rejectfile");
		return -1;
	}

	std::cout << "Enter absolute path to save file: " << std::endl;
	m_receiveThreadWantsInput = true;
	while(m_receiveThreadWantsInput)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::string filePath;
	filePath = m_receiveInput;
	m_receiveInput.clear();

	filePath.append(fileName);

	std::ofstream outFile;
	outFile.open(filePath);

	if(!outFile.is_open())
	{
		std::cerr << "Error: can't save file!" << std::endl;
		sendData("/rejectfile");
		return -1;
	}

	sendData("/acceptfile");
	std::cout << "Starting to receive data." << std::endl;
	unsigned int bufferSize {4096};
	char buffer [bufferSize];
	long int totalReceivedBytes {0};

	while(true)
	{
		memset(buffer, 0, bufferSize);

		long int receivedBytes {receiveNonblocking(buffer, bufferSize)};
		if(receivedBytes == -1)
		{
			std::cerr << "Error: can't receive the file!" << std::endl;
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
			std::string str {buffer};
			if(str.find("/endfile") == 0)
				break;

			outFile << buffer << std::endl;
			totalReceivedBytes += receivedBytes;
		}
	}
	std::cout << "Total received bytes: " << totalReceivedBytes << std::endl;
	outFile.close();
	std::cout << "File created in:" << std::endl;
	std::cout << filePath << std::endl;
	return 0;
}

int Client::sendFile(const std::string& filePath)
{
	std::ifstream inFile;
	inFile.open(filePath);

	std::string fileName {filePath.begin() + filePath.find_last_of('/') + 1, filePath.end()};
	std::cout << "sendFile()" << std::endl;
	std::cout << "filePath = \"" << filePath << "\"" << std::endl;
	std::cout << "fileName = \"" << fileName << "\"" << std::endl;
	// std::string filePath2 {getcwd()}

	if(!inFile.is_open())
	{
		std::cerr << "Error: can't open file: \"" << fileName << "\"!" << std::endl;
		return -1;
	}

	sendData("/sendfile " + fileName);
	if(waitForAcceptFile() <= -1)
	{
		std::cerr << "Error: file not accepted." << std::endl;
		return -1;
	}

	std::cout << "Starting to read from file." << std::endl;
	std::vector<char> buffer (1024, 0);

	inFile.read(buffer.data(), buffer.size());
	std::streamsize s = inFile.gcount();

	while(s > 0)
	{
		long int sentBytes {sendData(buffer.data())};
		if(sentBytes <= -1)
		{
			std::cerr << "Error: can't send data!" << std::endl;
			return -1;
		}

		inFile.read(buffer.data(), buffer.size());
		s = inFile.gcount();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	std::cout << "Finished reading file." << std::endl;
	sendData("/endfile");

	return 0;
}