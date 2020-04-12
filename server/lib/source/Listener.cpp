#include <Listener.hpp>
#include <Utils.hpp>
#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <string>
#include <string.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>

Listener::Listener() :
	m_listening_fd{-1}, m_sockfd_count{0}
{
}

Listener::~Listener()
{
	for(int sockfd_itr {0}; sockfd_itr <= m_sockfd_count; ++sockfd_itr)
	{
		if(FD_ISSET(sockfd_itr, &m_master))
		{
			if(sockfd_itr != m_listening_fd)
				remove_socket(sockfd_itr);
		}
	}

	if(m_listening_fd != -1)
		remove_socket(m_listening_fd);
}

// RETURNS: 0 on success; -1 on getaddrinfo() error; -2 on binding error
int Listener::create_listening_socket(const std::string& port_number)
{
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	addrinfo* list_begin;
	auto addr_info_result = getaddrinfo(nullptr, port_number.c_str(), &hints, &list_begin);
	if(addr_info_result != 0)
		return -1;

	addrinfo* list_result;
	for(list_result = list_begin; list_result != nullptr; list_result = list_result->ai_next)
	{
		m_listening_fd = socket(list_result->ai_family, list_result->ai_socktype, list_result->ai_protocol);
		if(m_listening_fd == -1)
			continue;

		auto sockopt = 1;
		setsockopt(m_listening_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));

		auto bind_result = bind(m_listening_fd, list_result->ai_addr, list_result->ai_addrlen);
		if(bind_result == 0)
			break;

		close(m_listening_fd);
	}

	if(list_result == nullptr)
		return -2;

	freeaddrinfo(list_begin);

	std::cout << "Listening socket #" << m_listening_fd << std::endl;

	auto info = Util::display_info((sockaddr_in*)list_result->ai_addr);
	std::cout << "Server's IP: " << info.first << ", port: " << info.second << "\n";

	return 0;
}

// RETURNS: 0 on success; -1 on listen() error
int Listener::start_listening()
{
	auto listening_result = listen(m_listening_fd, 2);
	if(listening_result <= -1)
		return -1;

	m_sockfd_count = m_listening_fd;

	FD_ZERO(&m_master);
	FD_SET(m_listening_fd, &m_master);

	return 0;
}

// RETURNS: new host's sockfd on connection; -1 on accept() error
int Listener::accept_host()
{
	sockaddr_storage remote_addr;
	socklen_t addr_size {sizeof(remote_addr)};

	auto new_fd = accept(m_listening_fd, (sockaddr*)&remote_addr, &addr_size);
	if(new_fd <= -1)
		return -1;

	FD_SET(new_fd, &m_master);
	auto info = Util::display_info((sockaddr_in*)&remote_addr);
	std::cout << "New client's IP: " << info.first << ", port: " << info.second << "\n";

	if(new_fd > m_sockfd_count)
		m_sockfd_count = new_fd;

	return new_fd;
}

// RETURNS: 0 on success, -1 on close() error
int Listener::remove_socket(int sockfd)
{
	FD_CLR(sockfd, &m_master);
	return close(sockfd);
}

// RETURNS: 0 on connection request; sender's sockfd on success; -1 on select() error; -2 on timeout
int Listener::poll()
{
	fd_set master_copy {m_master};

	timeval timeout;
	timeout.tv_usec = 1000;
	timeout.tv_sec = 0;
	auto select_result = select(m_sockfd_count + 1, &master_copy, nullptr, nullptr, &timeout);
	if(select_result == 0)
		return -2;
	if(select_result == -1)
		return -1;

	for(int sockfd_itr {0}; sockfd_itr <= m_sockfd_count; ++sockfd_itr)
	{
		if(FD_ISSET(sockfd_itr, &master_copy))
		{
			if(sockfd_itr == m_listening_fd)
				return 0; // It's a connection request
			else
				return sockfd_itr; // It's a sending data request
		}
	}
	return -2;
}

long int Listener::receive_data(int sockfd, char* buffer, unsigned int buffer_size)
{
	memset(buffer, 0, buffer_size);

	return recv(sockfd, buffer, buffer_size, 0);
}

long int Listener::send_data(int receiver_fd, const std::string& data)
{
	return send(receiver_fd, data.c_str(), data.size(), 0);
}

long int Listener::send_all(const std::string& data)
{
	for(int sockfd_itr {0}; sockfd_itr <= m_sockfd_count; ++sockfd_itr)
	{
		if(FD_ISSET(sockfd_itr, &m_master))
		{
			if(sockfd_itr != m_listening_fd)
			{
				if(send_data(sockfd_itr, data) == -1)
					return -1;
			}
		}
	}
	return 0;
}

long int Listener::send_to_except(int except_fd, const std::string& data)
{
	for(int sockfd_itr {0}; sockfd_itr <= m_sockfd_count; ++sockfd_itr)
	{
		if(FD_ISSET(sockfd_itr, &m_master))
		{
			if(sockfd_itr != m_listening_fd && sockfd_itr != except_fd)
			{
				if(send_data(sockfd_itr, data) == -1)
					return -1;
			}
		}
	}
	return 0;
}