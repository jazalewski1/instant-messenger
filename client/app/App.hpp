#pragma once

#include <Client.hpp>
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
	bool m_is_app_running;


public:
	App() :
		m_host{}, m_client{&m_host},
		m_input{}, m_sender{}, m_receiver{},
		m_is_app_running{false}
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
		
		std::cout << "Client closing.\n";
	}


	int start(std::string ip_address, int port_number)
	{
		auto connect_result = m_client.connect(ip_address, port_number);
		if(connect_result <= -1)
		{
			std::cerr << "Error: can't connect to server!\n";
			return -1;
		}
		std::cout << "Succesfully connected to server!\n\n";
		std::cout << "(type \"/close\" to disconnect)\n";
		std::cout << "(type \"/sendfile <absolutePathToFile>\" to send a file)\n\n";

		m_is_app_running = true;

		m_receive_thread = std::thread{&App::receive_loop, this};
		m_send_thread = std::thread{&App::send_loop, this};
		input_loop();
	}

	void input_loop()
	{
		std::string input {};
		while(m_is_app_running)
		{
			std::getline(std::cin, input);
			std::cout << "YOU> " << input << "\n";

			if(input.find("/close") == 0)
			{
				m_is_app_running = false;
				break;
			}
			else
			{
				m_input.notify(input);
			}
		}
	}

	std::string request_input(InputObserver& observer) // TODO: this has to throw when app is shut down (or come up with different design)
	{
		observer.set_wants_input(true);
		while(!observer.has_input() && m_is_app_running);
		observer.set_wants_input(false);
		return observer.get_input();
	}

	void receive_loop()
	{
		while(m_is_app_running)
		{
			constexpr auto buffer_size = 4096U;
			char buffer [buffer_size];

			auto received_bytes = m_client.receive_data_nonblocking(buffer, buffer_size);
			if(received_bytes == -1)
			{
				std::cerr << "Error: can't receive data!" << std::endl;
				m_is_app_running = false;
				break;
			}
			else if(received_bytes == 0)
			{
				std::cerr << "Server shut down." << std::endl;
				m_is_app_running = false;
				break;
			}
			else if(received_bytes > 0)
			{
				std::string received_data {buffer};

				if(received_data.find("/receivefile") == 0)
				{
					std::string filename {received_data.begin() + received_data.find_first_not_of("/receivefile "), received_data.end()};

					std::ofstream outfile;

					auto accept_file_result = accept_file(filename, outfile);
					if(accept_file_result <= -1)
					{
						std::cout << "File not accepted." << std::endl;
						m_client.send_data("/rejectfile");
					}
					else
					{
						m_client.send_data("/acceptfile");
						std::cout << "Starting to receive data." << std::endl;

						auto file_received_bytes = m_client.receive_file(outfile);
						if(file_received_bytes <= -1)
						{
							std::cerr << "Error: can't receive file!" << std::endl;
						}
						else
						{
							std::cout << "Total received bytes: " << file_received_bytes << std::endl;
						}
					}
					outfile.close();
				}
				else
				{
					std::cout << "CHAT> " << received_data << std::endl;
				}
			}

		}
	}

	// RETURNS: 0 on accepted; -1 on rejected
	int accept_file(const std::string& filename, std::ofstream& outfile)
	{
		std::cout << "Incoming file. Filename: \"" << filename << "\"" << std::endl;
		std::cout << "Do you accept? [Y / N]" << std::endl;

		auto input = request_input(m_receiver);

		if(input.find("Y") != 0)
			return -1;

		std::cout << "Enter absolute path to save file: " << std::endl;
		auto filepath = request_input(m_receiver);
		filepath.append(filename);

		outfile.open(filepath);

		if(!outfile.is_open())
		{
			std::cerr << "Error: can't open file! Path:" << std::endl;
			std::cout << filepath << std::endl;
			return -1;
		}
		return 0;
	}

	void send_loop()
	{
		while(m_is_app_running)
		{
			auto input = request_input(m_sender);

			if(input.find("/sendfile") == 0)
			{
				auto whitespace_index = input.find_first_not_of("/sendfile");
				if(whitespace_index == std::string::npos || whitespace_index == input.length() - 1)
				{
					std::cerr << "Usage: \"/sendfile <absolutePathToFile>\"" << std::endl;
					continue;
				}
				else
				{
					std::string filepath {input.begin() + whitespace_index + 1, input.end()};

					std::ifstream in_file;
					auto request_result = request_send_file(filepath, in_file);
					if(request_result <= -1)
					{
						std::cerr << "Error: request failed!" << std::endl;
						continue;
					}

					auto send_file_result = m_client.send_file(in_file);
					if(send_file_result <= -1)
					{
						std::cerr << "Error: failed to send file!" << std::endl;
						continue;
					}
					in_file.close();

					std::this_thread::sleep_for(std::chrono::milliseconds(50));
					m_client.send_data("/endfile");

					std::cout << "File sent succesfully." << std::endl;
				}
			}
			else if(input.find("/") == 0)
			{
				std::cerr << "Unknown command: " << input << std::endl;
			}
			else
			{
				auto sent_bytes = m_client.send_data(input);
				if(sent_bytes == -1)
				{
					std::cerr << "Error: can't send data!" << std::endl;
					m_is_app_running = false;
					break;
				}
				else if(sent_bytes == 0)
				{
					std::cout << "Disconnecting..." << std::endl;
					m_is_app_running = false;
					break;
				}
			}
		}
	}

	int request_send_file(const std::string& filepath, std::ifstream& in_file)
	{
		in_file.open(filepath);

		std::string filename {filepath.begin() + filepath.find_last_of('/') + 1, filepath.end()};

		if(!in_file.is_open())
		{
			std::cerr << "Error: can't open file: \"" << filename << "\"!" << std::endl;
			return -1;
		}

		m_client.send_data("/sendfile " + filename);
		if(wait_for_accept_file() <= -1)
		{
			std::cerr << "Error: file not accepted." << std::endl;
			return -1;
		}
		return 0;
	}

	// RETURNS: 0 on accepted; -1 on rejected
	int wait_for_accept_file()
	{
		auto buffer_size = 4096U;
		char buffer [buffer_size];

		std::cout << "Waiting for accepting the file..." << std::endl;
		auto received_bytes = m_client.receive_data_blocking(buffer, buffer_size);
		if(received_bytes <= -1)
		{
			std::cerr << "Error: can't receive data!" << std::endl;
			m_is_app_running = false;
			return -1;
		}
		else if(received_bytes == 0)
		{
			std::cerr << "Server shut down." << std::endl;
			m_is_app_running = false;
			return -1;
		}
		else if(received_bytes > 0)
		{
			std::string data {buffer};
			if(data.find("/acceptfile") == 0)
				return 0;
			else if(data.find("/rejectfile") == 0)
				return -1;
		}
		return -1;
	}
};