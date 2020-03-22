#include <Listener.hpp>
#include <memory>
#include <string>


int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " portNumber" << std::endl;
		return -1;
	}

	Listener server {std::stoi(argv[1])};
	std::string input;

	server.startRunning();

	std::cout << "(type \"/close\" to disconnect)\n\n";

	while(true)
	{
		std::getline(std::cin, input);
		if(input == "/close")
		{
			break;
		}
	}


	return 0;
}