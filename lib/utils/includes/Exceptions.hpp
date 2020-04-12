#pragma once

#include <exception>

namespace Util
{

class exception {};

// Socket exceptions
class poll_error : public exception {};

class create_socket_error : public exception {};

class connect_error : public exception {};

class listen_error : public exception {};

class accept_error : public exception {};

class remove_error : public exception {};

class disconnected_exception : public exception
{
private:
	int m_sock_fd;
public:
	disconnected_exception(int fd) : m_sock_fd{fd} {}
	int sock_fd() const { return m_sock_fd; }
};

class timeout_exception : public exception {};

// Data exceptions
class receive_error : public exception {};

class send_error : public exception {};

// App exceptions
class close_exception : public exception {};

}