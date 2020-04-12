#pragma once

#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <netdb.h>

namespace Util
{

// Utility functions
std::pair<std::string, std::string> display_info(sockaddr_in* sock_addr_ptr)
{
	char ip [NI_MAXHOST];
	memset(ip, 0, NI_MAXHOST);

	getnameinfo((sockaddr*)&sock_addr_ptr, sizeof(sockaddr_in), ip, NI_MAXHOST, nullptr, 0, 0);

	inet_ntop(AF_INET, &sock_addr_ptr->sin_addr, ip, NI_MAXHOST);
	return {ip, std::to_string(ntohs(sock_addr_ptr->sin_port))};
}

}