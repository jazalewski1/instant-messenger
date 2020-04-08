#pragma once

#include <IHost.hpp>
#include <arpa/inet.h>
#include <string>

class Host : public IHost
{
private:
	int m_sockfd;

public:
	Host();

	~Host();

	int create_socket() override;

	int conn(const std::string& ip_address, int port_number) override;

	long int receive_nonblocking(char* buffer, int buffer_size) override;

	long int receive_blocking(char* buffer, int buffer_size) override;

	long int send_data(const std::string& data) override;

	static void display_info(const std::string& name, sockaddr_in* sock_addr_ptr);
};