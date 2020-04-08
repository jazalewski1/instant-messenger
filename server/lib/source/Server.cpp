#include <Data.hpp>
#include <Listener.hpp>
#include <Server.hpp>
#include <iostream>
#include <string>
#include <string.h>

Server::Server(IListener* listener) :
	m_listener{listener}
{
}

int Server::connect(const std::string& port_number)
{
	auto create_listening_result = m_listener->create_listening_socket(port_number);
	if(create_listening_result == -1)
	{
		std::cerr << "Error: can't get address info!\n";
		return -1;
	}
	else if(create_listening_result == -2)
	{
		std::cerr << "Error: can't bind listening socket!\n";
		return -1;
	}

	auto listening_result = m_listener->start_listening();
	if(listening_result <= -1)
	{
		std::cerr << "Error: can't start listening!\n";
		return -1;
	}

	return 0;
}

int Server::add_client()
{
	auto accept_result = m_listener->accept_host();
	if(accept_result <= -1)
	{
		std::cerr << "Error: can't connect new client! Client #" << accept_result << "\n";
		return -1;
	}
	std::cout << "New client connected. Client #" << accept_result << "\n";
	return 0;
}

int Server::remove_client(int sockfd)
{
	if(m_listener->remove_socket(sockfd) == -1)
	{
		std::cerr << "Error: can't remove client! Client # " << sockfd << "\n";
		return -1;
	}
	std::cout << "Removed client # " << sockfd << "\n";
	return 0;
}

int Server::poll()
{
	auto poll_result = m_listener->poll();
	if(poll_result == -1)
	{
		std::cerr << "Error: can't select socket!\n";
		return -1;
	}
	return poll_result;
}

Data Server::receive(int source_fd)
{
	auto buffer_size = 4096u;
	char buffer [buffer_size];

	memset(buffer, 0, buffer_size);

	auto received_bytes = m_listener->receive(source_fd, buffer, buffer_size);
	if(received_bytes == -1)
	{
		std::cerr << "Error: can't receive data! From client #" << source_fd << "\n";
		return Data{-1, ""};
	}
	return Data{received_bytes, std::string{buffer}};
}

int Server::send_to(int destination_fd, const std::string& data)
{
	auto sent_bytes = m_listener->send_data(destination_fd, data);
	if(sent_bytes == -1)
	{
		std::cerr << "Error: can't send data! To client #" << destination_fd << '\n';
		return -1;
	}
	return 0;
}

int Server::send_to_except(int except_fd, const std::string& data)
{
	auto sent_bytes = m_listener->send_to_except(except_fd, data);
	if(sent_bytes == -1)
	{
		std::cerr << "Error: can't send data!\n";
		return -1;
	}
	return 0;
}


int Server::wait_for_accept_file(int sender_fd)
{
	while(true)
	{
		auto poll_result = poll();
		if(poll_result == -1)
		{
			std::cerr << "Error: select socket!\n";
			return -1;
		}
		else if(poll_result > 0)
		{
			if(poll_result != sender_fd)
			{
				auto received_data = receive(poll_result);
				if(received_data.bytes <= -1)
				{
					std::cerr << "Error: can't receive data! Socket #" << poll_result << "\n";
					return -1;
				}
				else if(received_data.bytes == 0)
				{
					std::cout << "Client disconnected. Socket #" << poll_result << "\n";
					remove_client(poll_result);
					return -1;
				}
				else
				{
					if(received_data.data.find("/acceptfile") == 0)
						return 0;
					else
						return -1;
				}
			}
		}
	}
	return -1;
}

int Server::transfer_file(int source_fd)
{
	while(true)
	{
		auto received_data = receive(source_fd);
		if(received_data.bytes <= -1)
		{
			std::cerr << "Error: can't receive file!\n";
			return -1;
		}
		else if(received_data.bytes == 0)
		{
			std::cerr << "Error: client disconnected!\n";
			remove_client(source_fd);
			return -1;
		}
		else
		{
			m_listener->send_to_except(source_fd, received_data.data);

			if(received_data.data.find("/endfile") == 0)
				break;
		}
	}
	return 0;
}