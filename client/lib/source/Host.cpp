#include <Host.hpp>
#include <Utils.hpp>
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Host::Host() :
	m_sockfd{-1}
{
}

Host::~Host()
{
	close(m_sockfd);
}

// RETURNS: sockfd on success; -1 on error
int Host::create_socket()
{
	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	return m_sockfd;
}

// RETURNS: 0 on success; -1 on error
int Host::conn(const std::string& ip_address, int port_number)
{
	sockaddr_in hints;
	hints.sin_family = AF_INET;
	hints.sin_port = htons(port_number);
	inet_pton(AF_INET, ip_address.c_str(), &hints.sin_addr);

	auto info = Util::display_info(&hints);

	std::cout << "Server's IP: " << info.first << ", port: " << info.second << "\n";

	return connect(m_sockfd, (sockaddr*)&hints, sizeof(hints));
}

// RETURNS: -2 on timeout; -1 on error; 0 on disconnection; else bytes received
long int Host::receive_nonblocking(char* buffer, unsigned long int buffer_size)
{
	pollfd pfd;
	pfd.fd = m_sockfd;
	pfd.events = POLLIN;
	int poll_result {poll(&pfd, 1, 100)};
	if(poll_result == -1)
		return -1;
	else if(poll_result == 0)
		return -2;

	memset(buffer, 0, buffer_size);
	return recv(m_sockfd, buffer, buffer_size, 0);
}

// RETURNS: -1 on error; 0 on disconnection; else bytes received
long int Host::receive_blocking(char* buffer, unsigned long int buffer_size)
{
	memset(buffer, 0, buffer_size);
	return recv(m_sockfd, buffer, buffer_size, 0);
}


// RETURNS: -1 on error; else bytes sent
long int Host::send_data(const std::string& data)
{
	return send(m_sockfd, data.c_str(), static_cast<int>(data.length()), 0);
}

int Host::get_sock_fd() const { return m_sockfd; }