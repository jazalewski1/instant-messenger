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
	m_isReceiveThreadRunning{false}
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
	std::cout << "(type \"/sendfile <fileName>\" to send a file)" << std::endl;

	while(true)
	{
		std::cout << "YOU> ";
		getline(std::cin, input);

		if(!input.empty())
		{
			if(input[0] == '/')
			{
				auto whiteItr {std::find_if(input.begin(), input.end(), [](char c)
				{
					return (c == ' ' ||  c == '\n'); 
				})};

				std::string command {input.begin(), whiteItr};


				if(command == "/close")
				{
					m_isReceiveThreadRunning = false;
					return 0;
				}
				else if(command == "/sendfile")
				{
					sendMsg(input);
					std::this_thread::sleep_for(std::chrono::milliseconds(50));

					std::string fileName {whiteItr + 1, input.end()};
					sendFile(fileName);

					sendMsg("/endfile");
				}
				else
					std::cerr << "Unknown command: " << command << std::endl;
			}
			else
			{
				long int sentBytes {sendMsg(input)};
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

void Client::startReceiving()
{
	m_isReceiveThreadRunning = true;
	while(m_isReceiveThreadRunning)
	{
		unsigned int bufferSize {4096};
		char buffer [bufferSize];

		long int receivedBytes {receive(buffer, static_cast<int>(bufferSize))};
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

	if(receivedData[0] == '/')
	{
		std::cout << "Attempting to receive a file." << std::endl;
		std::string fileName {receivedData.begin() + receivedData.find_first_not_of("/receivefile "), receivedData.end()};
		std::cout << "Filename: \"" << fileName << "\"" << std::endl;
		
		receiveFile(fileName);
	}
	else
	{
		std::cout << "CHAT> " << receivedData << "" << std::endl;
	}
}

void Client::receiveFile(const std::string& fileName)
{

	std::cout << "Opening a new file." << std::endl;
	std::ofstream outFile;
	outFile.open("result/" + fileName);

	if(!outFile.is_open())
	{
		std::cerr << "Error: can't create file!" << std::endl;
		sendMsg("/noaccept");
	}
	else
	{		
		std::cout << "Starting to receive data." << std::endl;
		unsigned int bufferSize {4096};
		char buffer [bufferSize];
		long int totalReceivedBytes {0};

		while(true)
		{
			memset(buffer, 0, bufferSize);

			long int receivedBytes {receive(buffer, bufferSize)};
			std::cout << "received bytes = " << receivedBytes << std::endl;
			std::cout << "buffer = " << buffer << std::endl;
			if(receivedBytes == -1)
			{
				std::cerr << "Error: can't receive the file!" << std::endl;
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
				std::string str {buffer};
				if(str == "/endfile")
					break;

				outFile << buffer << std::endl;
				totalReceivedBytes += receivedBytes;
			}
		}
		std::cout << "Total received bytes: " << totalReceivedBytes << std::endl;
		outFile.close();
	}
}

void Client::sendFile(const std::string& fileName)
{
	std::cout << "Opening file." << std::endl;
	std::ifstream inFile;
	inFile.open(fileName);

	if(!inFile.is_open())
	{
		std::cerr << "Error: can't open file: " << fileName << "!" << std::endl;
	}
	else
	{
		std::cout << "Start reading from file." << std::endl;
		std::vector<char> buffer (1024, 0);

		inFile.read(buffer.data(), buffer.size());
		std::streamsize s = inFile.gcount();

		while(s > 0)
		{
			long int sentBytes {send(m_sockfd, buffer.data(), 1024, 0)};
			if(sentBytes <= -1)
			{
				std::cerr << "Error: can't send data!" << std::endl;
			}

			inFile.read(buffer.data(), buffer.size());
			s = inFile.gcount();
		}

		std::cout << "Finished reading file." << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		sendMsg("/endfile");
	}
}