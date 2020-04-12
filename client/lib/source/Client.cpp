#include <Client.hpp>
#include <Exceptions.hpp>
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

void Client::connect(const std::string& ip_address, int port_number)
{
	if(m_host->create_socket() < 0)
		throw Util::create_socket_error{};

	if(m_host->conn(ip_address, port_number) < 0)
		throw Util::connect_error{};
}

void Client::send_data(const std::string& data)
{
	if(m_host->send_data(data) < 0)
		throw Util::send_error{};
}

std::string Client::receive_data_nonblocking()
{
	constexpr auto buffer_size = 4096U;
	char buffer [buffer_size];

	auto received_bytes = m_host->receive_nonblocking(buffer, static_cast<unsigned long int>(buffer_size));

	if(received_bytes == -2)
		throw Util::timeout_exception{};
	if(received_bytes == -1)
		throw Util::receive_error{};
	if(received_bytes == 0)
		throw Util::disconnected_exception{m_host->get_sock_fd()};

	return std::string{buffer};
}

std::string Client::receive_data_blocking()
{
	constexpr auto buffer_size = 4096U;
	char buffer [buffer_size];

	auto received_bytes = m_host->receive_blocking(buffer, static_cast<unsigned long int>(buffer_size));

	if(received_bytes < 0)
		throw Util::receive_error{};
	if(received_bytes == 0)
		throw Util::disconnected_exception{m_host->get_sock_fd()};

	return std::string{buffer};
}