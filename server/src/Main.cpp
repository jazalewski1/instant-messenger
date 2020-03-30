#include <Data.hpp>
#include <Listener.hpp>
#include <Server.hpp>
#include <string>
#include <thread>

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " portNumber\n";
		return -1;
	}

	Listener listener {};

	Server server {&listener};

	if(server.connect(argv[1]) <= -1)
	{
		std::cerr << "Error: can't connect the server!\n";
		return -1;
	}

	std::cout << "Enter \"/close\" to close server.\n";

	bool isClientRunning {true};
	std::thread inputThread {[&](){
		std::string input;
		while(isClientRunning)
		{
			std::getline(std::cin, input);
			if(input.find("/close") == 0)
				isClientRunning = false;
		}
	}};

	while(isClientRunning)
	{
		int pollResult {server.poll()};
		if(pollResult == -1)
		{
			break;
		}
		else if(pollResult == 0)
		{
			if(server.addClient() == -1)
				break;
		}
		else if(pollResult > 0)
		{
			Data dataReceived {server.receive(pollResult)};

			if(dataReceived.bytes == -1)
			{
				break;
			}
			else if(dataReceived.bytes == 0)
			{
				if(server.removeClient(pollResult) == -1)
					break;
			}
			else
			{
				std::cout << "CLIENT #" << pollResult << "> " << dataReceived.data << std::endl;

				if(dataReceived.data.find("/sendfile") == 0)
				{
					std::string filename {dataReceived.data.begin() + dataReceived.data.find(' ') + 1, dataReceived.data.end()};
					std::cout << "Request to send file: \"" << filename << "\"" << std::endl;

					server.sendExcept(pollResult, "/receivefile " + filename);
					std::this_thread::sleep_for(std::chrono::milliseconds(50));

					if(server.waitForAcceptFile(pollResult) == -1)
					{
						std::cout << "File not accepted." << std::endl;
						server.sendTo(pollResult, "/rejectfile");
					}
					else
					{
						std::cout << "File accepted." << std::endl;
						server.sendTo(pollResult, "/acceptfile");

						std::this_thread::sleep_for(std::chrono::milliseconds(50));

						server.transferFile(pollResult);
					}
				}
				else
				{
					server.sendExcept(pollResult, dataReceived.data);
				}
			}
		}
	}

	if(inputThread.joinable())
		inputThread.join();

	std::cout << "Closing.\n";

	return 0;
}