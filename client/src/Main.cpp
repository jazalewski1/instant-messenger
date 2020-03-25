#include <Client.hpp>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " ipAddress portNumber" << std::endl;
		return -1;
	}

	std::string location {argv[1]};
	int portNumber {std::stoi(argv[2])};

	Client client {location, portNumber};

	int clientStartResult {client.start()};
	if(clientStartResult <= -1)
	{
		std::cerr << "Error: can't start the client!" << std::endl;
	}

	return 0;
}