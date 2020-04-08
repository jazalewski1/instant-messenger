#pragma once

#include <string>

class IListener
{
public:
	virtual ~IListener() {};
	
	virtual int create_listening_socket(const std::string& port_number) = 0;

	virtual int start_listening() = 0;

	virtual int accept_host() = 0;

	virtual int remove_socket(int sockfd) = 0;

	virtual int poll() = 0;

	virtual long int receive(int sockfd, char* buffer, unsigned int buffer_size) = 0;

	virtual long int send_data(int receiver_fd, const std::string& data) = 0;

	virtual long int send_all(const std::string& data) = 0;

	virtual long int send_to_except(int except_fd, const std::string& data) = 0;
};