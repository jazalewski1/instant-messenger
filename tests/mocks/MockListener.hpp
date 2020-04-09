#include <IListener.hpp>
#include <gmock/gmock.h>

class MockListener : public IListener
{
public:
	MOCK_METHOD(int, create_listening_socket, (const std::string& port_number), (override));
	
	MOCK_METHOD(int, start_listening, (), (override));

	MOCK_METHOD(int, accept_host, (), (override));

	MOCK_METHOD(int, remove_socket, (int sockfd), (override));

	MOCK_METHOD(int, poll, (), (override));

	MOCK_METHOD(long int, receive_data, (int sockfd, char* buffer, unsigned int buffer_size), (override));

	MOCK_METHOD(long int, send_data, (int receiver_fd, const std::string& data), (override));

	MOCK_METHOD(long int, send_all, (const std::string& data), (override));

	MOCK_METHOD(long int, send_to_except, (int except_fd, const std::string& data), (override));
};