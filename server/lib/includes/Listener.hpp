#pragma once

#include <IListener.hpp>
#include <iostream>
#include <netdb.h>
#include <string>

class Listener : public IListener
{
private:
	int m_listening_fd;
	int m_sockfd_count;
	fd_set m_master;

public:
	Listener();

	~Listener();

	int create_listening_socket(const std::string& port_number) override;

	int start_listening() override;

	int accept_host() override;

	int remove_socket(int sockfd) override;

	int poll() override;

	long int receive(int sockfd, char* buffer, unsigned int boffer_size) override;

	long int send_data(int receiver_fd, const std::string& data) override;

	long int send_all(const std::string& data) override;

	long int send_to_except(int except_fd, const std::string& data) override;
};