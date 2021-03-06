#include <App.hpp>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " ip_address port_number" << std::endl;
		return -1;
	}

	std::string ip_address {argv[1]};
	int port_number {std::stoi(argv[2])};

	App app {};

	app.start(ip_address, port_number);

	return 0;
}