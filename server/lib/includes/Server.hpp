#pragma once

#include <Data.hpp>
#include <Listener.hpp>
#include <iostream>
#include <string>
#include <thread>

class Server
{
private:
	IListener* m_listener;

public:
	Server(IListener* listener);

	int connect(const std::string& port_number);

	int add_client();

	int remove_client(int sockfd);

	int poll();

	Data receive(int source_fd);

	int send_to(int source_fd, const std::string& data);

	int send_to_except(int except_fd, const std::string& data);

	int wait_for_accept_file(int source_fd);

	int transfer_file(int source_fd);
};