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

	EXPECT_EQ(server.connect("9999"), 0);
}

TEST_F(ServerTest, ConnectFailure)
{
	EXPECT_CALL(listener, create_listening_socket("9999"))
	.WillOnce(Return(-1));

	EXPECT_CALL(listener, start_listening)
	.Times(0);

	EXPECT_EQ(server.connect("9999"), -1);
}

TEST_F(ServerTest, ConnectFailure2)
{
	EXPECT_CALL(listener, create_listening_socket("9999"))
	.WillOnce(Return(0));

	EXPECT_CALL(listener, start_listening)
	.WillOnce(Return(-1));

	EXPECT_EQ(server.connect("9999"), -1);
}

TEST_F(ServerTest, PollSuccess)
{
	EXPECT_CALL(listener, poll)
	.WillOnce(Return(1));

	EXPECT_EQ(server.poll(), 1);
}

TEST_F(ServerTest, PollFailure)
{
	EXPECT_CALL(listener, poll)
	.WillOnce(Return(-1));

	EXPECT_EQ(server.poll(), -1);
}

TEST_F(ServerTest, ReceiveSuccess)
{
	const std::string msg {"test message"};

	EXPECT_CALL(listener, receive_data)
	.WillOnce(Return(msg.length()));

	Data expected {static_cast<long int>(msg.length()), msg};

	EXPECT_EQ(server.receive_from(5).bytes, expected.bytes);
}

TEST_F(ServerTest, ReceiveFailure)
{
	EXPECT_CALL(listener, receive_data)
	.WillOnce(Return(-1));

	Data expected {-1, ""};

	EXPECT_EQ(server.receive_from(5).bytes, expected.bytes);
}

TEST_F(ServerTest, send_toSuccess)
{
	const std::string msg {"test message"};

	EXPECT_CALL(listener, send_data(5, msg))
	.WillOnce(Return(msg.length()));

	EXPECT_EQ(server.send_to(5, msg), 0);
}

TEST_F(ServerTest, send_toFailure)
{
	const std::string msg {"test message"};

	EXPECT_CALL(listener, send_data(5, msg))
	.WillOnce(Return(-1));

	EXPECT_EQ(server.send_to(5, msg), -1);
}

TEST_F(ServerTest, AcceptFileSuccess1)
{
	EXPECT_CALL(listener, poll)
	.WillOnce(Return(3));

	const std::string acceptMsg {"/accept"};

	EXPECT_CALL(listener, receive_data)
	.WillOnce(Return(acceptMsg.length()));

	server.wait_for_accept_file(5);
}

TEST_F(ServerTest, AcceptFileSuccess2)
{
	InSequence seq;

	EXPECT_CALL(listener, poll)
	.Times(3)
	.WillRepeatedly(Return(-2));

	EXPECT_CALL(listener, poll)
	.WillOnce(Return(3));

	const std::string acceptMsg {"/accept"};

	EXPECT_CALL(listener, receive_data)
	.WillOnce(Return(acceptMsg.length()));

	server.wait_for_accept_file(5);
}

TEST_F(ServerTest, AcceptFileFailure1)
{
	EXPECT_CALL(listener, poll)
	.WillOnce(Return(-1));

	EXPECT_CALL(listener, receive_data)
	.Times(0);

	server.wait_for_accept_file(5);
}

TEST_F(ServerTest, AcceptFileFailure2)
{
	EXPECT_CALL(listener, poll)
	.WillOnce(Return(3));

	EXPECT_CALL(listener, receive_data)
	.WillOnce(Return(-1));

	EXPECT_EQ(server.wait_for_accept_file(5), -1);
}

TEST_F(ServerTest, transfer_fileFailure1)
{
	const std::string msg {"test file"};

	InSequence seq;

	EXPECT_CALL(listener, receive_data)
	.WillOnce(Return(msg.length()));

	EXPECT_CALL(listener, send_to_except)
	.Times(1);

	EXPECT_CALL(listener, receive_data)
	.WillOnce(Return(-1));

	EXPECT_CALL(listener, send_to_except)
	.Times(0);

	server.transfer_file(5);
}