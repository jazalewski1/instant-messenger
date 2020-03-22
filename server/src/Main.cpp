#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <Listener.hpp>


int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " portNumber" << std::endl;
		return -1;
	}

	Listener server {std::stoi(argv[1])};

	server.run();

	return 0;
}