#include <Listener.hpp>
#include <Server.hpp>
#include <Exceptions.hpp>
#include <iostream>
#include <string>
#include <string.h>

Server::Server(IListener* listener) :
	m_listener{listener}
{
}

void Server::connect(const std::string& port_number)
{
	if(m_listener->create_listening_socket(port_number) < 0)
		throw Util::create_socket_error{};

	if(m_listener->start_listening() < 0)
		throw Util::listen_error{};
}

void Server::add_client()
{
	auto accept_result = m_listener->accept_host();
	if(accept_result < 0)
		throw Util::accept_error{};
	
	std::cout << "Connected client #" << accept_result << "\n";
}

void Server::remove_client(int sockfd)
{
	if(m_listener->remove_socket(sockfd) < 0)
		throw Util::remove_error{};
	
	std::cout << "Removed client # " << sockfd << "\n";
}

int Server::poll()
{
	auto poll_result = m_listener->poll();

	if(poll_result == -1)
		throw Util::poll_error{};

	if(poll_result == -2)
		throw Util::timeout_exception{};
		
	return poll_result;
}

std::string Server::receive_from(int source_fd)
{
	constexpr auto buffer_size = 4096u;
	char buffer [buffer_size];

	auto received_bytes = m_listener->receive_data(source_fd, buffer, buffer_size);
	if(received_bytes < 0)
		throw Util::receive_error{};

	if(received_bytes == 0)
		throw Util::disconnected_exception{source_fd};
	
	return buffer;
}

void Server::send_to(int destination_fd, const std::string& data)
{
	if(m_listener->send_data(destination_fd, data) < 0)
		throw Util::send_error{};
}

void Server::send_to_except(int except_fd, const std::string& data)
{
	if(m_listener->send_to_except(except_fd, data) < 0)
		throw Util::send_error{};
}