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

TEST_F(ServerTest, PollSuccess)
{
	EXPECT_CALL(listener, poll)
	.WillOnce(Return(5));

	EXPECT_EQ(server.poll(), 5);
}

TEST_F(ServerTest, ReceiveSuccess)
{
	const std::string msg {"test message"};

	EXPECT_CALL(listener, receive_data)
	.WillOnce(Return(msg.length()));

	EXPECT_NO_THROW(server.receive_from(5));
}

TEST_F(ServerTest, ReceiveFailure)
{
	EXPECT_CALL(listener, receive_data)
	.WillOnce(Return(-1));

	EXPECT_THROW(server.receive_from(5), Util::receive_error);
}

TEST_F(ServerTest, SendToSuccess)
{
	const std::string msg {"test message"};

	EXPECT_CALL(listener, send_data(5, msg))
	.WillOnce(Return(msg.length()));

	EXPECT_NO_THROW(server.send_to(5, msg));
}

TEST_F(ServerTest, SendToFailure)
{
	const std::string msg {"test message"};

	EXPECT_CALL(listener, send_data(5, msg))
	.WillOnce(Return(-1));

	EXPECT_THROW(server.send_to(5, msg), Util::send_error);
}