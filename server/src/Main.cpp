#include <Listener.hpp>
#include <string>


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