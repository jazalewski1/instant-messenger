#pragma once

#include <exception>

namespace Util
{

struct exception {};

// Socket exceptions
struct poll_error : public exception {};

struct create_socket_error : public exception {};

struct listen_error : public exception {};

struct accept_error : public exception {};

struct remove_error : public exception {};

struct disconnected_exception : public exception {};

struct timeout_exception : public exception {};

// Data exceptions
struct receive_error : public exception {};

struct send_error : public exception {};

}