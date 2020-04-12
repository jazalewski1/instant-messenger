#include <Exceptions.hpp>
#include <Server.hpp>
#include <MockListener.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

struct ServerTest : public testing::Test
{
public:
	MockListener listener;
	Server server;

	ServerTest() : 
		listener{}, server{&listener}
	{
	}
};

using namespace testing;

TEST_F(ServerTest, ConnectSuccess)
{
	EXPECT_CALL(listener, create_listening_socket("9999"))
	.WillOnce(Return(0));

	EXPECT_CALL(listener, start_listening)
	.WillOnce(Return(0));

	EXPECT_NO_THROW(server.connect("9999"));
}

TEST_F(ServerTest, ConnectFailureCreating)
{
	EXPECT_CALL(listener, create_listening_socket("9999"))
	.WillOnce(Return(-1));

	EXPECT_CALL(listener, start_listening)
	.Times(0);

	EXPECT_THROW(server.connect("9999"), Util::create_socket_error);
}

TEST_F(ServerTest, ConnectFailureListening)
{
	EXPECT_CALL(listener, create_listening_socket("9999"))
	.WillOnce(Return(0));

	EXPECT_CALL(listener, start_listening)
	.WillOnce(Return(-1));

	EXPECT_THROW(server.connect("9999"), Util::listen_error);
}

TEST_F(ServerTest, AddClientSuccess)
{
	EXPECT_CALL(listener, accept_host)
	.WillOnce(Return(5));

	EXPECT_NO_THROW(server.add_client());
}

TEST_F(ServerTest, AddClientFailure)
{
	EXPECT_CALL(listener, accept_host)
	.WillOnce(Return(-1));

	EXPECT_THROW(server.add_client(), Util::accept_error);
}

TEST_F(ServerTest, PollSuccess)
{
	EXPECT_CALL(listener, poll)
	.WillOnce(Return(5));

	EXPECT_EQ(server.poll(), 5);
}

TEST_F(ServerTest, PollFailureError)
{
	EXPECT_CALL(listener, poll)
	.WillOnce(Return(-1));

	EXPECT_THROW(server.poll(), Util::poll_error);
}

TEST_F(ServerTest, PollTimeout)
{
	EXPECT_CALL(listener, poll)
	.WillOnce(Return(-2));

	EXPECT_THROW(server.poll(), Util::timeout_exception);
}

TEST_F(ServerTest, ReceiveSuccess)
{
	const std::string data {"this is some data"};

	EXPECT_CALL(listener, receive_data(5, _, _))
	.WillOnce(Return(data.length()));

	EXPECT_NO_THROW(server.receive_from(5));
}

TEST_F(ServerTest, ReceiveFailureError)
{
	EXPECT_CALL(listener, receive_data(5, _, _))
	.WillOnce(Return(-1));

	EXPECT_THROW(server.receive_from(5), Util::receive_error);
}

TEST_F(ServerTest, ReceiveDisconnected)
{
	EXPECT_CALL(listener, receive_data(5, _, _))
	.WillOnce(Return(0));

	EXPECT_THROW(server.receive_from(5), Util::disconnected_exception);
}

TEST_F(ServerTest, SendToSuccess)
{
	const std::string data {"this is some data"};

	EXPECT_CALL(listener, send_data(5, data))
	.WillOnce(Return(data.length()));

	EXPECT_NO_THROW(server.send_to(5, data));
}

TEST_F(ServerTest, SendToFailure)
{
	const std::string data {"this is some data"};

	EXPECT_CALL(listener, send_data(5, data))
	.WillOnce(Return(-1));

	EXPECT_THROW(server.send_to(5, data), Util::send_error);
}