#include <IHost.hpp>
#include <gmock/gmock.h>

class MockHost : public IHost
{
public:
	MOCK_METHOD(int, createSocket, (), (override));
	
	MOCK_METHOD(int, conn, (const std::string& ipAddress, int portNumber), (override));

	MOCK_METHOD(long int, receiveNonblocking, (char* buffer, int bufferSize), (override));

	MOCK_METHOD(long int, receiveBlocking, (char* buffer, int bufferSize), (override));

	MOCK_METHOD(long int, sendData, (const std::string& data), (override));
};