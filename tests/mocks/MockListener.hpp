#include <IListener.hpp>
#include <gmock/gmock.h>

class MockListener : public IListener
{
public:
	MOCK_METHOD(int, createListeningSocket, (const std::string& portNumber), (override));
	
	MOCK_METHOD(int, startListening, (), (override));

	MOCK_METHOD(int, acceptHost, (), (override));

	MOCK_METHOD(int, removeSocket, (int sockfd), (override));

	MOCK_METHOD(int, poll, (), (override));

	MOCK_METHOD(long, receive, (int sockfd, char* buffer, unsigned int bufferSize), (override));

	MOCK_METHOD(long int, sendData, (int receiverSockfd, const std::string& data), (override));

	MOCK_METHOD(long int, sendAll, (const std::string& data), (override));

	MOCK_METHOD(long int, sendAllExcept, (int exceptSockfd, const std::string& data), (override));
};