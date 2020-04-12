#pragma once

#include <Host.hpp>
#include <string>

class Client
{
private:
	IHost* m_host;

private:
	int wait_for_accept_file();

public:
	Client(IHost* host);

	~Client();

	void connect(const std::string& ip_address, int port_number);

	void send_data(const std::string& data);

	std::string receive_data_nonblocking();

	std::string receive_data_blocking();

	void receive_file(std::ostream& out_file);

	void send_file(std::ifstream& in_file);
};