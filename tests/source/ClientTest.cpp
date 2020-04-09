#include <Client.hpp>
#include <MockHost.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>

class ClientTest : public testing::Test
{
protected:
	testing::NiceMock<MockHost> host;
	std::string ipAddress;
	int portNumber;
	Client client;

protected:
	ClientTest() :
		host{}, ipAddress{"0.0.0.0"}, portNumber{9999}, client{&host}
	{
	}
};

TEST_F(ClientTest, ConenctionTest)
{
	EXPECT_CALL(host, create_socket());

	EXPECT_CALL(host, conn(ipAddress, portNumber));

	client.connect(ipAddress, portNumber);
}