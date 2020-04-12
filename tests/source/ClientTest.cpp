#include <Client.hpp>
#include <Exceptions.hpp>
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

TEST_F(ClientTest, ConenctionTestSuccess)
{
	EXPECT_CALL(host, create_socket());

	EXPECT_CALL(host, conn(ipAddress, portNumber));

	EXPECT_NO_THROW(client.connect(ipAddress, portNumber));
}

TEST_F(ClientTest, ConenctionTestFailureCreateSocket)
{
	EXPECT_CALL(host, create_socket())
	.WillOnce(testing::Return(-1));

	EXPECT_CALL(host, conn(ipAddress, portNumber))
	.Times(0);

	EXPECT_THROW(client.connect(ipAddress, portNumber), Util::create_socket_error);
}

TEST_F(ClientTest, ConenctionTestFailureConnectError)
{
	EXPECT_CALL(host, create_socket());

	EXPECT_CALL(host, conn(ipAddress, portNumber))
	.WillOnce(testing::Return(-1));

	EXPECT_THROW(client.connect(ipAddress, portNumber), Util::connect_error);
}

TEST_F(ClientTest, SendDataSuccess)
{
	std::string data {"this is data"};
	
	EXPECT_CALL(host, send_data(data))
	.WillOnce(testing::Return(data.length()));

	EXPECT_NO_THROW(client.send_data(data));
}

TEST_F(ClientTest, SendDataFailure)
{
	std::string data {"this is data"};

	EXPECT_CALL(host, send_data(data))
	.WillOnce(testing::Return(-1));

	EXPECT_THROW(client.send_data(data), Util::send_error);
}

TEST_F(ClientTest, ReceiveNonblockingSuccess)
{
	std::string data {"this is data"};

	EXPECT_CALL(host, receive_nonblocking(testing::_, testing::_))
	.WillOnce(testing::Return(data.length()));

	EXPECT_NO_THROW(client.receive_data_nonblocking());
}

TEST_F(ClientTest, ReceiveNonblockingTimeout)
{
	std::string data {"this is data"};

	EXPECT_CALL(host, receive_nonblocking(testing::_, testing::_))
	.WillOnce(testing::Return(-2));

	EXPECT_THROW(client.receive_data_nonblocking(), Util::timeout_exception);
}

TEST_F(ClientTest, ReceiveNonblockingFailureError)
{
	std::string data {"this is data"};

	EXPECT_CALL(host, receive_nonblocking(testing::_, testing::_))
	.WillOnce(testing::Return(-1));

	EXPECT_THROW(client.receive_data_nonblocking(), Util::receive_error);
}

TEST_F(ClientTest, ReceiveNonblockingFailureDisconnected)
{
	std::string data {"this is data"};

	EXPECT_CALL(host, receive_nonblocking(testing::_, testing::_))
	.WillOnce(testing::Return(0));

	EXPECT_THROW(client.receive_data_nonblocking(), Util::disconnected_exception);
}

TEST_F(ClientTest, ReceiveBlockingSuccess)
{
	std::string data {"this is data"};

	EXPECT_CALL(host, receive_blocking(testing::_, testing::_))
	.WillOnce(testing::Return(data.length()));

	EXPECT_NO_THROW(client.receive_data_blocking());
}

TEST_F(ClientTest, ReceiveBlockingFailureError)
{
	std::string data {"this is data"};

	EXPECT_CALL(host, receive_blocking(testing::_, testing::_))
	.WillOnce(testing::Return(-1));

	EXPECT_THROW(client.receive_data_blocking(), Util::receive_error);
}

TEST_F(ClientTest, ReceiveBlockingFailureDisconnected)
{
	std::string data {"this is data"};

	EXPECT_CALL(host, receive_blocking(testing::_, testing::_))
	.WillOnce(testing::Return(0));

	EXPECT_THROW(client.receive_data_blocking(), Util::disconnected_exception);
}