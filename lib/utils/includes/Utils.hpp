#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <string.h>
#include <netdb.h>

namespace Utils
{

void display_info(const std::string& name, sockaddr_in* sock_addr_ptr) // TODO: could return std::pair of strings with ip and port
{
	char ip [NI_MAXHOST];
	memset(ip, 0, NI_MAXHOST);
	char port [NI_MAXSERV];
	memset(port, 0, NI_MAXSERV);

	getnameinfo((sockaddr*)&sock_addr_ptr, sizeof(sockaddr_in), ip, NI_MAXHOST, nullptr, 0, 0);

	inet_ntop(AF_INET, &sock_addr_ptr->sin_addr, ip, NI_MAXHOST);
	std::cout << name << "IP: " << ip << ", port: " << ntohs(sock_addr_ptr->sin_port) << std::endl;
}

}