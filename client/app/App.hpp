#pragma once

#include <Client.hpp>
#include <Host.hpp>
#include <iostream>
#include <string>

class App
{
private:
	std::string m_ip_address;
	int m_port_number;
	Host m_host;
	Client m_client;


public:
	App(std::string ip_address, int port_number) :
		m_ip_address{ip_address}, m_port_number{port_number},
		m_host{}, m_client{&m_host}
	{
	}

	~App()
	{
	}

	int run()
	{
		auto connect_result = m_client.connect(m_ip_address, m_port_number);
		if(connect_result <= -1)
		{
			std::cerr << "Error: can't connect to server!" << std::endl;
			return -1;
		}
		std::cout << "Connected to server succesfully!\n" << std::endl;

		m_client.start();

		std::cout << "(type \"/close\" to disconnect)" << std::endl;
		std::cout << "(type \"/sendfile <absolutePathToFile>\" to send a file)\n" << std::endl;

		std::string input;
		do
		{
			std::cout << "YOU> ";
			std::getline(std::cin, input);
		} while(m_client.update(input));
	}
};