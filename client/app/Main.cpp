#include <App.hpp>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " ipAddress port_number" << std::endl;
		return -1;
	}

	std::string ip_address {argv[1]};
	int port_number {std::stoi(argv[2])};

	App app {ip_address, port_number};

	app.run();

	return 0;
}