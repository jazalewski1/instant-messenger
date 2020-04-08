#include <IHost.hpp>
#include <gmock/gmock.h>
#include <string>

class MockHost : public IHost
{
public:
	MOCK_METHOD(int, create_socket, (), (override));
	
	MOCK_METHOD(int, conn, (const std::string& ip_address, int port_number), (override));

	MOCK_METHOD(long int, receive_nonblocking, (char* buffer, int buffer_size), (override));

	MOCK_METHOD(long int, receive_blocking, (char* buffer, int buffer_size), (override));

	MOCK_METHOD(long int, send_data, (const std::string& data), (override));
};