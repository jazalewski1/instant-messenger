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
	m_serverIpAddress{ipAddress}, m_serverPortNumber{portNumber},
	m_isReceiveThreadRunning{false}
{
}

Client::~Client()
{
	if(m_isReceiveThreadRunning && m_receiveThread.joinable())
	{
		m_isReceiveThreadRunning = false;
		m_receiveThread.join();
	}
	close(m_sockfd);

	std::cout << "Closing." << std::endl;
}

int Client::createSocket()
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

int Client::conn()
{
	m_hint.sin_family = AF_INET;
	m_hint.sin_port = htons(m_serverPortNumber);
	inet_pton(AF_INET, m_serverIpAddress.c_str(), &m_hint.sin_addr);

	return connect(m_sockfd, (sockaddr*)&m_hint, sizeof(m_hint));
}

void Client::startReceiving()
{
	m_receiveThread = std::thread {[&]() { receive(); }};
	m_receiveThread.detach();
}

void Client::receive()
{
	unsigned int maxBufferSize {4096};
	char buffer [maxBufferSize];

	m_isReceiveThreadRunning = true;
	while(m_isReceiveThreadRunning)
	{
		memset(buffer, 0, maxBufferSize);

		long int receivedBytes {recv(m_sockfd, buffer, static_cast<int>(maxBufferSize), 0)};
		if(receivedBytes == -1)
		{
			std::cerr << "Error: can't receive data." << std::endl;
			return;
		}
		else if(receivedBytes == 0)
		{
			std::cerr << "Server shut down." << std::endl;
			break;
		}
		else
		{
			std::string receivedData {buffer, 0, static_cast<long unsigned int>(receivedBytes)};
			if(receivedData[0] == '/')
			{
				std::string fileName {receivedData.begin() + receivedData.find_first_not_of("/receivefile "), receivedData.end()};
				std::cout << "Filename: >" << fileName << "<" << std::endl;
				
				receiveFile(fileName);
			}
			else
			{
				std::cout << "CHAT> " << receivedData << "" << std::endl;
			}
		}
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
	}
	else
	{
		unsigned int bufferSize {4096};
		char buffer [bufferSize];

		long int totalReceivedBytes {0};
		while(true)
		{
			memset(buffer, 0, bufferSize);

			long int receivedBytes {recv(m_sockfd, buffer, bufferSize, 0)};
			if(receivedBytes <= -1)
			{
				std::cerr << "Error: can't receive data!" << std::endl;
				break;
			}
			else if(receivedBytes == 0)
			{
				std::cerr << "Server shut down." << std::endl;
				break;
			}
			else
			{
				std::string str {buffer};
				if(str == "/endfile")
				{
					break;
				}
				else
				{
					outFile << buffer << std::endl;
					totalReceivedBytes += receivedBytes;
				}
			}
		}
		std::cout << "Total received bytes: " << totalReceivedBytes << std::endl;
		outFile.close();
	}
}

long int Client::sendData(const std::string& msg)
{
	return send(m_sockfd, msg.c_str(), static_cast<int>(msg.length()), 0);
}

void Client::run()
{
	m_sockfd = createSocket();
	if(m_sockfd == -1)
	{
		std::cerr << "Error: can't create a socket!" << std::endl;
		return;
	}

	std::cout << "Connecting to server..." << std::endl;
	int connectResult {conn()};
	if(connectResult <= -1)
	{
		std::cerr << "Error: can't connect to server!" << std::endl;
		return;
	}

	std::cout << "Connected." << std::endl;
	displayInfo("Server", &m_hint);

	startReceiving();

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
					break;
				}
				else if(command == "/sendfile")
				{
					if(whiteItr == input.end())
					{
						std::cerr << "Usage: /sendfile <fileName>" << std::endl;
					}
					else
					{
						std::string fileName {whiteItr + 1, input.end()};

						std::cout << "Opening file." << std::endl;
						std::ifstream inFile;
						inFile.open(fileName);

						if(!inFile.is_open())
						{
							std::cerr << "Error: can't open file: " << fileName << "!" << std::endl;
						}
						else
						{
							long int sentBytes {sendData(input)};
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
							if(sentBytes == -1)
							{
								std::cerr << "Error: can't send data." << std::endl;
								break;
							}
							else if(sentBytes == 0)
							{
								std::cout << "Disconnecting..." << std::endl;
								break;
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
									else if(sentBytes == 0)
									{

									}

									inFile.read(buffer.data(), buffer.size());
									s = inFile.gcount();
								}

								std::cout << "Finished reading file." << std::endl;
								std::this_thread::sleep_for(std::chrono::milliseconds(100));
								sendData("/endfile");
							}
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
					std::cerr << "Error: can't send data." << std::endl;
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

void Client::displayInfo(const std::string& name, sockaddr_in* saddrPtr)
{
	char ip [NI_MAXHOST];
	memset(ip, 0, NI_MAXHOST);
	char port [NI_MAXSERV];
	memset(port, 0, NI_MAXSERV);

	getnameinfo((sockaddr*)&saddrPtr, sizeof(sockaddr_in), ip, NI_MAXHOST, nullptr, 0, 0);

	inet_ntop(AF_INET, &saddrPtr->sin_addr, ip, NI_MAXHOST);
	std::cout << name << " IP: " << ip << ", port: " << ntohs(saddrPtr->sin_port) << std::endl;
}