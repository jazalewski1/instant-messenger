#include <Exceptions.hpp>
#include <Listener.hpp>
#include <Server.hpp>
#include <string>
#include <thread>

class App
{
private:
	const std::string m_port_number;
	Listener m_listener;
	Server m_server;
	std::thread m_input_thread;
	bool m_is_app_running;

public:
	App(std::string port_number) :
		m_port_number{port_number},
		m_listener{}, m_server{&m_listener},
		m_is_app_running{false}
	{
	}

	~App()
	{
		if(m_input_thread.joinable())
			m_input_thread.join();

		std::cout << "Closing server.\n";
	}

	void input_loop()
	{
		std::string input {};
		while(m_is_app_running)
		{
			std::getline(std::cin, input);
			if(input == "/close")
			{
				m_is_app_running = false;
				break;
			}
		}
	}

	void receive_loop()
	{
		while(m_is_app_running)
		{
			try
			{
				auto poll_result = m_server.poll();

				if(poll_result == 0)
					m_server.add_client();
				else if(poll_result > 0)
				{
					auto received_data = m_server.receive_from(poll_result);

					std::cout << "CLIENT #" << poll_result << "> " << received_data << std::endl;

					if(received_data.find("/sendfile") == 0)
					{
						auto filename = std::string{received_data.begin() + received_data.find(' ') + 1, received_data.end()};
						std::cout << "Request to send file: \"" << filename << "\"" << std::endl;

						m_server.send_to_except(poll_result, "/receivefile " + filename);

						std::this_thread::sleep_for(std::chrono::milliseconds(50));

						if(wait_for_accept_file(poll_result))
						{
							std::cout << "File accepted." << std::endl;

							m_server.send_to(poll_result, "/acceptfile"); 
							std::this_thread::sleep_for(std::chrono::milliseconds(50));

							transfer_file(poll_result);
						}
						else
						{
							std::cout << "File not accepted." << std::endl;
							m_server.send_to(poll_result, "/rejectfile");
						}
					}
					else
					{
						m_server.send_to_except(poll_result, received_data);
					}
				}			
			}
			catch(const Util::poll_error&)
			{
				std::cerr << "Error: can't poll!\n";
				m_is_app_running = false;
				break;
			}
			catch(const Util::timeout_exception&)
			{
				continue;
			}
			catch(const Util::accept_error&)
			{
				std::cerr << "Error: can't add client!\n";
			}
			catch(const Util::receive_error&)
			{
				std::cerr << "Error: can't receive data!\n";
				m_is_app_running = false;
				break;
			}
			catch(const Util::disconnected_exception& exc)
			{
				try
				{
					m_server.remove_client(exc.sock_fd());
					continue;
				}
				catch(Util::remove_error& remove_exc)
				{
					std::cerr << "Error: can't remove client!\n";
					m_is_app_running = false;
					break;
				}
			}
			catch(const Util::send_error&)
			{
				std::cerr << "Error: can't send data!\n";
				m_is_app_running = false;
				break;
			}
			catch(...)
			{
				std::cerr << "Error: unknown!\n";
				m_is_app_running = false;
				break;
			}
		}
	}

	void start()
	{
		std::cout << "Starting server.\n";

		try { m_server.connect(m_port_number); }
		catch(const Util::exception&)
		{
			std::cerr << "Error: can't connect the server!\n";
			return;
		}

		std::cout << "Enter \"/close\" to close server.\n";

		m_is_app_running = true;
		m_input_thread = std::thread{&App::input_loop, this};
		receive_loop();
	}

	bool wait_for_accept_file(int sender_fd)
	{
		while(true)
		{
			try
			{
				auto poll_result = m_server.poll();

				if(poll_result != sender_fd)
				{
					auto received_data = m_server.receive_from(poll_result);

					if(received_data.find("/acceptfile") == 0)
						return true;
					else
						break;
				}
			}
			catch(const Util::poll_error&)
			{
				std::cerr << "Error: polling error!\n";
				break;
			}
			catch(const Util::receive_error&)
			{
				std::cerr << "Error: can't receive file data!\n";
				break;
			}
			catch(const Util::disconnected_exception& exc)
			{
				std::cerr << "Error: client disconnected while accepting file!\n";
				m_server.remove_client(exc.sock_fd());
				break;
			}
		}
		return false;
	}

	void transfer_file(int source_fd)
	{
		while(true)
		{
			std::string received_data;
			try
			{
				received_data = m_server.receive_from(source_fd);

				m_server.send_to_except(source_fd, received_data);

				if(received_data.find("/endfile") == 0)
					break;
			}
			catch(const Util::receive_error&)
			{
				std::cerr << "Error: can't receive file!\n";
				break;
			}
			catch(const Util::disconnected_exception&)
			{
				std::cerr << "Error: client disconnected while transfering file!\n";
				m_server.remove_client(source_fd);
				break;
			}
			catch(const Util::send_error&)
			{
				std::cerr << "Error: can't send file data!\n";
				break;
			}
		}
	}
};