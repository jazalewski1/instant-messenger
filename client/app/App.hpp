#pragma once

#include <Client.hpp>
#include <Exceptions.hpp>
#include <Host.hpp>
#include <InputObservable.hpp>
#include <InputObserver.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

class App
{
private:
	Host m_host;
	Client m_client;
	InputObservable m_input;
	InputObserver m_receiver;
	InputObserver m_sender;
	std::thread m_receive_thread;
	std::thread m_send_thread;
	std::thread m_input_thread;
	bool m_is_app_running;

	bool m_block_receive;

public:
	App() :
		m_host{}, m_client{&m_host},
		m_input{}, m_sender{}, m_receiver{},
		m_is_app_running{false}, m_block_receive{false}
	{
		m_input.add(&m_receiver);
		m_input.add(&m_sender);
	}

	~App()
	{
		if(m_receive_thread.joinable())
			m_receive_thread.join();
		if(m_send_thread.joinable())
			m_send_thread.join();
		if(m_input_thread.joinable())
			m_input_thread.join();
		
		std::cout << "Client closing.\n";
	}


	void start(std::string ip_address, int port_number)
	{
		try { m_client.connect(ip_address, port_number); }
		catch(const Util::create_socket_error&)
		{
			std::cerr << "Error: can't create socket!\n";
			return;
		}
		catch(const Util::connect_error&)
		{
			std::cerr << "Error: can't connect to server!\n";
			return;
		}

		std::cout << "Succesfully connected to server!\n\n";
		std::cout << "Enter \"/close\" to disconnect\n";
		std::cout << "Enter \"/sendfile <absolute_path_to_file>\" to send a file\n\n";

		m_is_app_running = true;

		m_input_thread = std::thread{&App::input_loop, this};
		m_receive_thread = std::thread{&App::receive_loop, this};
		m_send_thread = std::thread{&App::send_loop, this};
	}

	void input_loop()
	{
		std::string input {};
		while(m_is_app_running)
		{
			std::getline(std::cin, input);

			if(input.find("/close") == 0)
			{
				m_is_app_running = false;
				break;
			}
			else
				m_input.notify(input);
		}
	}

	std::string get_input(InputObserver& observer)
	{
		observer.set_wants_input(true);
		while(!observer.has_input())
		{
			if(!m_is_app_running)
				throw Util::close_exception{};
		}
		observer.set_wants_input(false);
		return observer.get_input();
	}

	void receive_loop()
	{
		while(m_is_app_running)
		{
			try
			{
				if(!m_block_receive)
				{
					auto received_data = m_client.receive_data_nonblocking();

					if(received_data.find("/receivefile") == 0)
					{
						std::string filename {received_data.begin() + received_data.find_first_not_of("/receivefile "), received_data.end()};

						if(accept_file(filename))
						{
							std::cout << "Enter absolute path to save file:\n";
							auto filepath = get_input(m_receiver);
							filepath.append(filename);

							std::ofstream outfile;
							outfile.open(filepath);
							if(outfile.is_open())
							{
								m_client.send_data("/acceptfile");
								receive_file(outfile);
							}
							else
							{
								std::cerr << "Error: can't create file!\n";
								std::cout << "File not accepted.";
							}
							outfile.close();
						}
						else
						{
							std::cout << "File not accepted.";
						}
					}
					else
					{
						std::cout << "CHAT> " << received_data << "\n";
					}
				}
			}
			catch(const Util::timeout_exception&)
			{
				continue;
			}
			catch(const Util::receive_error&)
			{
				std::cerr << "Error: can't receive data!\n";
				continue;
			}
			catch(const Util::disconnected_exception&)
			{
				std::cerr << "Server shut down.\n";
				m_is_app_running = false;
				break;
			}
		}
	}

	bool accept_file(const std::string& filename)
	{
		std::cout << "Incoming file. Filename: \"" << filename << "\"\n";
		std::cout << "Do you accept? [Y / N]\n";

		try
		{
			auto input = get_input(m_receiver);

			if(input.find("Y") == 0)
				return true;
		}
		catch(const Util::close_exception&) { /* Don't do anything */ }
		return false;
	}

	void receive_file(std::ofstream& outfile)
	{
		std::cout << "Starting to receive file.\n";
		while(true)
		{
			try
			{
				std::string received_data;

				m_block_receive = true;
				received_data = m_client.receive_data_blocking();
				m_block_receive = false;

				if(received_data.find("/endfile") == 0)
					break;

				outfile << received_data;
			}
			catch(const Util::receive_error&)
			{
				std::cerr << "Error: failed to receive file data!\n";
				m_block_receive = false;
				return;
			}
			catch(const Util::disconnected_exception&)
			{
				std::cerr << "Error: server disconnected while receiving file!\n";
				m_block_receive = false;
				return;
			}
		}
		std::cout << "File received successfully.\n";
	}

	void send_loop()
	{
		while(m_is_app_running)
		{
			std::string input;
			try { input = get_input(m_sender); }
			catch(const Util::close_exception&) { break; }

			if(input.find("/sendfile") == 0)
			{
				auto whitespace_index = input.find_first_not_of("/sendfile");
				if(whitespace_index == std::string::npos || whitespace_index == input.length() - 1)
				{
					std::cerr << "Usage: \"/sendfile <absolute_path_to_file>\"\n";
					continue;
				}
				else
				{
					std::string filepath {input.begin() + whitespace_index + 1, input.end()};

					send_file(filepath);
				}
			}
			else if(input.find("/") == 0)
			{
				std::cerr << "Unknown command: " << input << "\n";
			}
			else
			{
				send_message(input);
			}
		}
	}

	void send_message(const std::string& message)
	{
		try { m_client.send_data(message); }
		catch(const Util::send_error&)
		{
			std::cerr << "Error: can't send data!\n";
			m_is_app_running = false;
		}
		catch(const Util::disconnected_exception&)
		{
			std::cout << "Disconnecting...\n";
			m_is_app_running = false;
		}
	}
	
	void send_file(const std::string& filepath)
	{
		std::ifstream infile;
		infile.open(filepath);
		if(infile.is_open())
		{
			try
			{
				auto filename = std::string{filepath.begin() + filepath.find_last_of('/') + 1, filepath.end()};
				m_client.send_data("/sendfile " + filename);

				std::cout << "Waiting for accepting the file...\n";
				if(wait_for_accept_file())
				{
					std::cerr << "File accepted.\n";
					std::cout << "Starting to send file.\n";

					std::vector<char> buffer (4096U, 0);

					infile.read(buffer.data(), buffer.size());
					while(infile.gcount() > 0)
					{
						m_client.send_data(buffer.data());

						infile.read(buffer.data(), buffer.size());
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(50));
					m_client.send_data("/endfile");
					infile.close();
					
					std::cout << "File sent successfully.\n";
				}
				else
				{
					std::cout << "File not accepted.\n";
				}
			}
			catch(const Util::send_error&)
			{
				std::cerr << "Error: can't send file data!\n";
			}
		}
		else
		{
			std::cerr << "Error: can't open file!\n";
		}
	}

	bool wait_for_accept_file()
	{
		try 
		{
			std::string received_data;

			m_block_receive = true;
			received_data = m_client.receive_data_blocking();

			if(received_data.find("/acceptfile") == 0)
				return true;
		}
		catch(const Util::receive_error&)
		{
			std::cerr << "Error: can't receive accept data!\n";
		}
		catch(const Util::disconnected_exception&)
		{
			std::cerr << "Error: server disconnected while accepting file!\n";
		}
		m_block_receive = false;
		return false;
	}
};