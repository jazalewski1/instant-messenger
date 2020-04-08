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

TEST_F(ClientTest, CloseTest)
{
	EXPECT_EQ(client.update("/close"), 0);
}

TEST_F(ClientTest, MessageTest)
{
	std::string input {"this is a message"};

	EXPECT_CALL(host, send_data(input))
	.Times(1);

	client.update(input);
}

TEST_F(ClientTest, SendfileTest)
{
	std::string input {"/sendfile CMakeLists.txt"};

	EXPECT_CALL(host, send_data(input))
	.Times(1);

	client.update(input);
}

TEST_F(ClientTest, SendfileWrongCommandTest)
{
	std::string input {"/sendfile"};

	EXPECT_CALL(host, send_data(input))
	.Times(0);

	client.update(input);
}