#include <Data.hpp>
#include <Listener.hpp>
#include <Server.hpp>
#include <iostream>
#include <string>
#include <string.h>

Server::Server(IListener* listener) :
	m_listener{listener}
{
}

int Server::connect(const std::string& portNumber)
{
	int createListeningResult {m_listener->createListeningSocket(portNumber)};
	if(createListeningResult == -1)
	{
		std::cerr << "Error: can't get address info!\n";
		return -1;
	}
	else if(createListeningResult == -2)
	{
		std::cerr << "Error: can't bind listening socket!\n";
		return -1;
	}

	int listeningResult {m_listener->startListening()};
	if(listeningResult <= -1)
	{
		std::cerr << "Error: can't start listening!\n";
		return -1;
	}

	return 0;
}

int Server::addClient()
{
	int acceptResult {m_listener->acceptHost()};
	if(acceptResult <= -1)
	{
		std::cerr << "Error: can't connect new client! Client #" << acceptResult << "\n";
		return -1;
	}
	std::cout << "New client connected. Client #" << acceptResult << "\n";
	return 0;
}

int Server::removeClient(int sockfd)
{
	if(m_listener->removeSocket(sockfd) == -1)
	{
		std::cerr << "Error: can't remove client! Client # " << sockfd << "\n";
		return -1;
	}
	std::cout << "Removed client # " << sockfd << "\n";
	return 0;
}

int Server::poll()
{
	int pollResult {m_listener->poll()};
	if(pollResult == -1)
	{
		std::cerr << "Error: can't select socket!\n";
		return -1;
	}
	return pollResult;
}

Data Server::receive(int sourceFd)
{
	unsigned int bufferSize {4096};
	char buffer [bufferSize];

	memset(buffer, 0, bufferSize);

	long int receivedBytes {m_listener->receive(sourceFd, buffer, bufferSize)};
	if(receivedBytes == -1)
	{
		std::cerr << "Error: can't receive data! From client #" << sourceFd << "\n";
		return Data{-1, ""};
	}
	return Data{receivedBytes, std::string{buffer}};
}

int Server::sendTo(int destinationFd, const std::string& data)
{
	long int bytesSent {m_listener->sendData(destinationFd, data)};
	if(bytesSent == -1)
	{
		std::cerr << "Error: can't send data! To client #" << destinationFd << '\n';
		return -1;
	}
	return 0;
}

int Server::sendExcept(int exceptFd, const std::string& data)
{
	long int bytesSent {m_listener->sendAllExcept(exceptFd, data)};
	if(bytesSent == -1)
	{
		std::cerr << "Error: can't send data!\n";
		return -1;
	}
	return 0;
}


int Server::waitForAcceptFile(int senderSockfd)
{
	while(true)
	{
		int pollResult {poll()};
		if(pollResult == -1)
		{
			std::cerr << "Error: select socket!\n";
			return -1;
		}
		else if(pollResult > 0)
		{
			if(pollResult != senderSockfd)
			{
				Data dataReceived {receive(pollResult)};
				if(dataReceived.bytes <= -1)
				{
					std::cerr << "Error: can't receive data! Socket #" << pollResult << "\n";
					return -1;
				}
				else if(dataReceived.bytes == 0)
				{
					std::cout << "Client disconnected. Socket #" << pollResult << "\n";
					removeClient(pollResult);
					return -1;
				}
				else
				{
					if(dataReceived.data.find("/acceptfile") == 0)
						return 0;
					else
						return -1;
				}
			}
		}
	}
	return -1;
}

int Server::transferFile(int sourceFd)
{
	while(true)
	{
		Data dataReceived {receive(sourceFd)};
		if(dataReceived.bytes <= -1)
		{
			std::cerr << "Error: can't receive file!\n";
			return -1;
		}
		else if(dataReceived.bytes == 0)
		{
			std::cerr << "Error: client disconnected!\n";
			removeClient(sourceFd);
			return -1;
		}
		else
		{
			m_listener->sendAllExcept(sourceFd, dataReceived.data);

			if(dataReceived.data.find("/endfile") == 0)
				break;
		}
	}
	return 0;
}