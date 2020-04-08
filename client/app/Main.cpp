#include <App.hpp>
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

	App app {ipAddress, portNumber};

	app.run();

	return 0;
}