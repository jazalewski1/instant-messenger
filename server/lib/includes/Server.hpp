#pragma once

#include <Listener.hpp>
#include <string>

class Server
{
private:
	IListener* m_listener;

public:
	Server(IListener* listener);

	void connect(const std::string& port_number);

	void add_client();

	void remove_client(int sockfd);

	int poll();

	std::string receive_from(int source_fd);

	void send_to(int source_fd, const std::string& data);

	void send_to_except(int except_fd, const std::string& data);
};