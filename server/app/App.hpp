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
			int poll_result;
			try { poll_result = m_server.poll(); }
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

			if(poll_result == 0)
			{
				try { m_server.add_client(); }
				catch(const Util::accept_error&)
				{
					std::cerr << "Error: can't add client!\n";
				}
			}
			else if(poll_result > 0)
			{
				std::string received_data;

				try { received_data = m_server.receive_from(poll_result); }
				catch(const Util::receive_error&)
				{
					std::cerr << "Error: can't receive data!\n";
					m_is_app_running = false;
					break;
				}
				catch(const Util::disconnected_exception&)
				{
					try { m_server.remove_client(poll_result); }	
					catch(Util::remove_error& rem_exc)
					{
						std::cerr << "Error: can't remove client!\n";
						m_is_app_running = false;
						break;
					}
					continue;
				}

				std::cout << "CLIENT #" << poll_result << "> " << received_data << std::endl;

				if(received_data.find("/sendfile") == 0)
				{
					std::string filename {received_data.begin() + received_data.find(' ') + 1, received_data.end()};
					std::cout << "Request to send file: \"" << filename << "\"" << std::endl;

					try { m_server.send_to_except(poll_result, "/receivefile " + filename); }
					catch(const Util::send_error&)
					{
						std::cerr << "Error: can't send data!\n";
						m_is_app_running = false;
						break;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(50));

					if(wait_for_accept_file(poll_result))
					{
						std::cout << "File accepted." << std::endl;
						try { m_server.send_to(poll_result, "/acceptfile"); }
						catch(const Util::send_error&)
						{
							std::cerr << "Error: can't send data!\n";
							m_is_app_running = false;
							break;
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(50));

						transfer_file(poll_result);
					}
					else
					{
						std::cout << "File not accepted." << std::endl;
						try { m_server.send_to(poll_result, "/rejectfile"); }
						catch(const Util::send_error&)
						{
							std::cerr << "Error: can't send data!\n";
							m_is_app_running = false;
							break;
						}
					}
				}
				else
				{
					try { m_server.send_to_except(poll_result, received_data); }
					catch(const Util::send_error&)
					{
						std::cerr << "Error: can't send data!\n";
						m_is_app_running = false;
						break;
					}
				}
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

		m_is_app_running = true;

		std::cout << "Enter \"/close\" to close server.\n";

		m_input_thread = std::thread{&App::input_loop, this};
		receive_loop();
	}

	bool wait_for_accept_file(int sender_fd)
	{
		while(true)
		{
			int poll_result;
			try
			{
				poll_result = m_server.poll();
			}
			catch(Util::poll_error& poll_exc)
			{
				std::cerr << "Error: polling error!\n";
				break;
			}

			if(poll_result != sender_fd)
			{
				std::string received_data;
				try { received_data = m_server.receive_from(poll_result); }
				catch(Util::receive_error& rec_exc)
				{
					std::cerr << "Error: can't receive data! Socket #" << poll_result << "\n";
					break;
				}
				catch(Util::disconnected_exception& dis_exc)
				{
					std::cout << "Client disconnected. Socket #" << poll_result << "\n";
					m_server.remove_client(poll_result);
					break;
				}

				if(received_data.find("/acceptfile") == 0)
					return true;
				else
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
			try { received_data = m_server.receive_from(source_fd); }
			catch(const Util::receive_error&)
			{
				std::cerr << "Error: can't receive file!\n";
				break;
			}
			catch(const Util::disconnected_exception&)
			{
				std::cerr << "Error: client disconnected file receiving file!\n";
				m_server.remove_client(source_fd);
				break;
			}

			try { m_server.send_to_except(source_fd, received_data); }
			catch(const Util::send_error&)
			{
				std::cerr << "Error: can't send file data!\n";
				break;
			}

			if(received_data.find("/endfile") == 0)
				break;
		}
	}
};