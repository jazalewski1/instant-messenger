#pragma once

#include <Host.hpp>
#include <string>
#include <thread>

class Client
{
private:
	IHost* m_host;
	bool m_is_input_thread_running;
	bool m_is_receive_thread_running;
	std::thread m_receive_thread;
	bool m_receive_thread_wants_input;
	std::string m_receive_input;

private:
	void start_receiving();

	int wait_for_accept_file();

public:
	Client(IHost* host);

	~Client();

	int connect(const std::string& ip_address, int port_number);

	void start();

	int update(const std::string& input);

	int accept_file(const std::string& filename, std::ofstream& out_file);

	long int receive_file(std::ostream& out_file);

	int request_send_file(const std::string& filepath, std::ifstream& in_file);

	int send_file(std::ifstream& in_file);
};