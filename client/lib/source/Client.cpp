#include <Client.hpp>
#include <Host.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

Client::Client(IHost* host) :
	m_host{host}
{
}

Client::~Client()
{
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

long int Client::send_data(const std::string& data)
{
	std::cout << "Client::send_data()\n";
	std::cout << "data = " << data << "\n";
	return m_host->send_data(data);
}

long int Client::receive_data_nonblocking(char* buffer, long int buffer_size)
{
	return m_host->receive_nonblocking(buffer, static_cast<int>(buffer_size));
}

long int Client::receive_data_blocking(char* buffer, long int buffer_size)
{
	return m_host->receive_blocking(buffer, static_cast<int>(buffer_size));
}

// RETURNS: total received bytes on success; -1 on error
long int Client::receive_file(std::ostream& outfile)
{
	constexpr auto buffer_size = 4096U;
	char buffer [buffer_size];
	auto total_received_bytes = 0;
	while(true)
	{
		auto received_bytes = receive_data_nonblocking(buffer, buffer_size); // TODO: check if blocking works, because we don't want messages to be saved in a file
		if(received_bytes == -1)
		{
			std::cerr << "Error: failed to receive data!" << std::endl;
			return -1;
		}
		else if(received_bytes == 0)
		{
			std::cerr << "Server shut down." << std::endl;
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