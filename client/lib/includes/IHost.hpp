#pragma once

#include <string>

class IHost
{
public:
	virtual ~IHost() {};

	virtual int create_socket() = 0;
	
	virtual int conn(const std::string& ip_address, int port_number) = 0;
	
	virtual long int receive_nonblocking(char* buffer, int buffer_size) = 0;
	
	virtual long int receive_blocking(char* buffer, int buffer_size) = 0;
	
	virtual long int send_data(const std::string& data) = 0;
};