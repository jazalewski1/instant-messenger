#pragma once

#include <Host.hpp>
#include <fstream>
#include <string>
#include <thread>

class Client
{
private:
	IHost* m_host;

private:
	int wait_for_accept_file();

public:
	Client(IHost* host);

	~Client();

	int connect(const std::string& ip_address, int port_number);

	long int send_data(const std::string& data);

	long int receive_data_nonblocking(char* buffer, long int buffer_size);

	long int receive_data_blocking(char* buffer, long int buffer_size);

	long int receive_file(std::ostream& out_file);

	int send_file(std::ifstream& in_file);
};