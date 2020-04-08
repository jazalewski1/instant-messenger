#include <Data.hpp>
#include <Listener.hpp>
#include <Server.hpp>
#include <string>
#include <thread>

class App
{
private:
	const std::string m_port_number;

	Listener listener;
	Server server;

	std::thread input_thread;

public:
	App(std::string port_number) :
		m_port_number{port_number},
		listener{},
		server{&listener}
	{
	}

	~App()
	{
		if(input_thread.joinable())
			input_thread.join();

		std::cout << "Closing.\n";
	}

	int run()
	{
		std::cout << "Starting server.\n";
		if(server.connect(m_port_number) <= -1)
		{
			std::cerr << "Error: can't connect the server!\n";
			return -1;
		}

		std::cout << "Enter \"/close\" to close server.\n";

		bool is_server_running {true};
		input_thread  = std::thread{[&]{
			std::string input;
			while(is_server_running)
			{
				std::getline(std::cin, input);
				if(input.find("/close") == 0)
					is_server_running = false;
			}
		}};

		while(is_server_running)
		{
			int poll_result {server.poll()};
			if(poll_result == -1)
			{
				break;
			}
			else if(poll_result == 0)
			{
				if(server.add_client() == -1)
					break;
			}
			else if(poll_result > 0)
			{
				Data data_received {server.receive(poll_result)};

				if(data_received.bytes == -1)
				{
					break;
				}
				else if(data_received.bytes == 0)
				{
					if(server.remove_client(poll_result) == -1)
						break;
				}
				else
				{
					std::cout << "CLIENT #" << poll_result << "> " << data_received.data << std::endl;

					if(data_received.data.find("/sendfile") == 0)
					{
						std::string filename {data_received.data.begin() + data_received.data.find(' ') + 1, data_received.data.end()};
						std::cout << "Request to send file: \"" << filename << "\"" << std::endl;

						server.send_to_except(poll_result, "/receivefile " + filename);
						std::this_thread::sleep_for(std::chrono::milliseconds(50));

						if(server.wait_for_accept_file(poll_result) == -1)
						{
							std::cout << "File not accepted." << std::endl;
							server.send_to(poll_result, "/rejectfile");
						}
						else
						{
							std::cout << "File accepted." << std::endl;
							server.send_to(poll_result, "/acceptfile");

							std::this_thread::sleep_for(std::chrono::milliseconds(50));

							server.transfer_file(poll_result);
						}
					}
					else
					{
						server.send_to_except(poll_result, data_received.data);
					}
				}
			}
		}
	}
};
