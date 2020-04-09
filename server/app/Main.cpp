#include <App.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " portNumber\n";
		return -1;
	}

	App app {argv[1]};

	app.start();

	return 0;
}