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
	EXPECT_CALL(listener, createListeningSocket("9999"))
	.WillOnce(Return(0));

	EXPECT_CALL(listener, startListening)
	.WillOnce(Return(0));

	EXPECT_EQ(server.connect("9999"), 0);
}

TEST_F(ServerTest, ConnectFailure)
{
	EXPECT_CALL(listener, createListeningSocket("9999"))
	.WillOnce(Return(-1));

	EXPECT_CALL(listener, startListening)
	.Times(0);

	EXPECT_EQ(server.connect("9999"), -1);
}

TEST_F(ServerTest, ConnectFailure2)
{
	EXPECT_CALL(listener, createListeningSocket("9999"))
	.WillOnce(Return(0));

	EXPECT_CALL(listener, startListening)
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

	EXPECT_CALL(listener, receive)
	.WillOnce(Return(msg.length()));

	Data expected {static_cast<long int>(msg.length()), msg};

	EXPECT_EQ(server.receive(5).bytes, expected.bytes);
}

TEST_F(ServerTest, ReceiveFailure)
{
	EXPECT_CALL(listener, receive)
	.WillOnce(Return(-1));

	Data expected {-1, ""};

	EXPECT_EQ(server.receive(5).bytes, expected.bytes);
}

TEST_F(ServerTest, SendToSuccess)
{
	const std::string msg {"test message"};

	EXPECT_CALL(listener, sendData(5, msg))
	.WillOnce(Return(msg.length()));

	EXPECT_EQ(server.sendTo(5, msg), 0);
}

TEST_F(ServerTest, SendToFailure)
{
	const std::string msg {"test message"};

	EXPECT_CALL(listener, sendData(5, msg))
	.WillOnce(Return(-1));

	EXPECT_EQ(server.sendTo(5, msg), -1);
}

TEST_F(ServerTest, AcceptFileSuccess1)
{
	EXPECT_CALL(listener, poll)
	.WillOnce(Return(3));

	const std::string acceptMsg {"/accept"};

	EXPECT_CALL(listener, receive)
	.WillOnce(Return(acceptMsg.length()));

	server.waitForAcceptFile(5);
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

	EXPECT_CALL(listener, receive)
	.WillOnce(Return(acceptMsg.length()));

	server.waitForAcceptFile(5);
}

TEST_F(ServerTest, AcceptFileFailure1)
{
	EXPECT_CALL(listener, poll)
	.WillOnce(Return(-1));

	EXPECT_CALL(listener, receive)
	.Times(0);

	server.waitForAcceptFile(5);
}

TEST_F(ServerTest, AcceptFileFailure2)
{
	EXPECT_CALL(listener, poll)
	.WillOnce(Return(3));

	EXPECT_CALL(listener, receive)
	.WillOnce(Return(-1));

	EXPECT_EQ(server.waitForAcceptFile(5), -1);
}

TEST_F(ServerTest, TransferFileFailure1)
{
	const std::string msg {"test file"};

	InSequence seq;

	EXPECT_CALL(listener, receive)
	.WillOnce(Return(msg.length()));

	EXPECT_CALL(listener, sendAllExcept)
	.Times(1);

	EXPECT_CALL(listener, receive)
	.WillOnce(Return(-1));

	EXPECT_CALL(listener, sendAllExcept)
	.Times(0);

	server.transferFile(5);
}