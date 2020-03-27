#include <Client.hpp>
#include <Host.hpp>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " ipAddress portNumber" << std::endl;
		return -1;
	}

	std::string ipAddress {argv[1]};
	int portNumber {std::stoi(argv[2])};

	Host host {};
	Client client {&host};

	int connectResult {client.connect(ipAddress, portNumber)};
	if(connectResult <= -1)
	{
		std::cerr << "Error: can't connect to server!" << std::endl;
		return -1;
	}
	std::cout << "Connected to server succesfully!" << std::endl;

	int clientStartResult {client.start()};
	if(clientStartResult <= -1)
	{
		std::cerr << "Error: running client!" << std::endl;
		return -1;
	}

	return 0;
}