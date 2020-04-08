#include <Client.hpp>
#include <Host.hpp>
#include <arpa/inet.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <netdb.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

Client::Client(IHost* host) :
	m_host{host},
	m_is_receive_thread_running{false},
	m_receive_thread_wants_input{false},
	m_receive_input{}
{
}

Client::~Client()
{
	m_is_receive_thread_running = false;
	if(m_receive_thread.joinable())
		m_receive_thread.join();

	std::cout << "Closing client." << std::endl;
}

// RETURNS: 0 on success; -1 on error
int Client::connect(const std::string& ip_address, int port_number)
{
	if(m_host->create_socket() <= -1)
	{
		std::cerr << "Erorr: can't create a socket!" << std::endl;
		return -1;
	}

	if(m_host->conn(ip_address, port_number) <= -1)
	{
		std::cerr << "Error: can't connect!" << std::endl;
		return -1;
	}

	return 0;
}

void Client::start()
{
	m_is_receive_thread_running = true;
	m_receive_thread = std::thread{&Client::start_receiving, this};
}

int Client::update(const std::string& input)
{
	if(m_receive_thread_wants_input)
	{
		m_receive_input = input;
		m_receive_thread_wants_input = false;
		return 1;
	}

	if(input.find("/close") == 0)
	{
		return 0;
	}
	else if(input.find("/sendfile") == 0)
	{
		auto whitespace_index = input.find_first_not_of("/sendfile");
		if(whitespace_index == std::string::npos || whitespace_index == input.length() - 1)
		{
			std::cerr << "Usage: \"/sendfile <absolutePathToFile>\"" << std::endl;
			return 1;
		}
		else
		{
			std::string filepath {input.begin() + whitespace_index + 1, input.end()};

			std::ifstream in_file;
			auto request_result = request_send_file(filepath, in_file);
			if(request_result <= -1)
			{
				std::cerr << "Error: request failed!" << std::endl;
				return 1;
			}

			auto send_file_result = send_file(in_file);
			if(send_file_result <= -1)
			{
				std::cerr << "Error: failed to send file!" << std::endl;
				return 0;
			}
			in_file.close();

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			m_host->send_data("/endfile");

			std::cout << "File sent succesfully." << std::endl;
		}
	}
	else if(input.find("/") == 0)
	{
		std::cerr << "Unknown command: " << input << std::endl;
		return 1;
	}
	else
	{
		auto sent_bytes = m_host->send_data(input);
		if(sent_bytes == -1)
		{
			std::cerr << "Error: can't send data!" << std::endl;
			return -1;
		}
		else if(sent_bytes == 0)
		{
			std::cout << "Disconnecting..." << std::endl;
			return -1;
		}
	}
	return 1;
}

void Client::start_receiving()
{
	while(m_is_receive_thread_running)
	{
		constexpr auto buffer_size = 4096U;
		char buffer [buffer_size];

		auto received_bytes = m_host->receive_nonblocking(buffer, static_cast<int>(buffer_size));
		if(received_bytes == -1)
		{
			std::cerr << "Error: can't receive data!" << std::endl;
			m_is_receive_thread_running = false;
			break;
		}
		else if(received_bytes == 0)
		{
			std::cerr << "Server shut down." << std::endl;
			m_is_receive_thread_running = false;
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
					m_host->send_data("/rejectfile");
				}
				else
				{
					m_host->send_data("/acceptfile");
					std::cout << "Starting to receive data." << std::endl;

					auto file_received_bytes = receive_file(outfile);
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
int Client::accept_file(const std::string& filename, std::ofstream& outfile)
{
	std::cout << "Incoming file. Filename: \"" << filename << "\"" << std::endl;
	std::cout << "Do you accept? [Y / N]" << std::endl;

	m_receive_thread_wants_input = true;
	while(m_receive_thread_wants_input)
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	auto input = m_receive_input;
	m_receive_input.clear();

	if(input.find("Y") != 0)
		return -1;

	std::cout << "Enter absolute path to save file: " << std::endl;
	m_receive_thread_wants_input = true;
	while(m_receive_thread_wants_input)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	auto filepath = m_receive_input;
	m_receive_input.clear();

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

// RETURNS: total received bytes on success; -1 on error
long int Client::receive_file(std::ostream& outfile)
{
	constexpr auto buffer_size = 4096U;
	char buffer [buffer_size];
	auto total_received_bytes = 0;
	while(true)
	{
		memset(buffer, 0, buffer_size);

		auto received_bytes = m_host->receive_nonblocking(buffer, buffer_size); // TODO: check if blocking works, because we don't want messages to be saved in a file
		if(received_bytes == -1)
		{
			std::cerr << "Error: failed to receive data!" << std::endl;
			return -1;
		}
		else if(received_bytes == 0)
		{
			std::cerr << "Server shut down." << std::endl;
			m_is_receive_thread_running = false;
			return -1;
		}
		else if(received_bytes > 0)
		{
			std::string data {buffer};
			if(data.find("/endfile") == 0)
				break;

			outfile << buffer << std::endl;
			total_received_bytes += received_bytes;
		}
	}
	return total_received_bytes;
}

int Client::request_send_file(const std::string& filepath, std::ifstream& in_file)
{
	in_file.open(filepath);

	std::string filename {filepath.begin() + filepath.find_last_of('/') + 1, filepath.end()};

	if(!in_file.is_open())
	{
		std::cerr << "Error: can't open file: \"" << filename << "\"!" << std::endl;
		return -1;
	}

	m_host->send_data("/sendfile " + filename);
	if(wait_for_accept_file() <= -1)
	{
		std::cerr << "Error: file not accepted." << std::endl;
		return -1;
	}
	return 0;
}

// RETURNS: 0 on accepted; -1 on rejected
int Client::wait_for_accept_file()
{
	auto buffer_size = 4096U;
	char buffer [buffer_size];
	memset(&buffer, 0, buffer_size);

	std::cout << "Waiting for accepting the file..." << std::endl;
	auto received_bytes = m_host->receive_blocking(buffer, buffer_size);
	if(received_bytes <= -1)
	{
		std::cerr << "Error: can't receive data!" << std::endl;
		m_is_receive_thread_running = false;
		return -1;
	}
	else if(received_bytes == 0)
	{
		std::cerr << "Server shut down." << std::endl;
		m_is_receive_thread_running = false;
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

// RETURN: total sent bytes on success; -1 on error
int Client::send_file(std::ifstream& in_file)
{
	std::cout << "Starting to read from file." << std::endl;
	std::vector<char> buffer (4096U, 0);

	in_file.read(buffer.data(), buffer.size());
	auto s = in_file.gcount();

	auto total_sent_bytes = 0L;

	while(s > 0)
	{
		auto sent_bytes = m_host->send_data(buffer.data());
		if(sent_bytes <= -1)
		{
			std::cerr << "Error: can't send data!" << std::endl;
			return -1;
		}
		total_sent_bytes += sent_bytes;

		in_file.read(buffer.data(), buffer.size());
		s = in_file.gcount();
	}
	return total_sent_bytes;
}