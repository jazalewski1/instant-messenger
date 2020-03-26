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

	Listener listener {};

	Server server {&listener};

	if(server.connect(argv[1]) <= -1)
	{
		std::cerr << "Error: can't connect the server!" << std::endl;
		return -1;
	}

	if(server.start() <= -1)
	{
		std::cerr << "Error: running server!" << std::endl;
		return -2;
	}

	return 0;
}