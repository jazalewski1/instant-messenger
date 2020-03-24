#include <Server.hpp>
#include <memory>
#include <string>


int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " portNumber" << std::endl;
		return -1;
	}

	Server server {std::stoi(argv[1])};

	int serverStartResult {server.start()};
	if(serverStartResult <= -1)
	{
		std::cerr << "Error: can't start the server!" << std::endl;
	}

	return 0;
}