#pragma once

#include <IHost.hpp>
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

	long int receive_nonblocking(char* buffer, unsigned long int buffer_size) override;

	long int receive_blocking(char* buffer, unsigned long int buffer_size) override;

	long int send_data(const std::string& data) override;

	int get_sock_fd() const override;
};