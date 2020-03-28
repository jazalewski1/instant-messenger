#include <Server.hpp>
#include <MockListener.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class ServerTest : public testing::Test
{
protected:
	testing::NiceMock<MockListener> listener;
	std::string portNumber;
	Server server;

protected:
	ServerTest() :
		listener{}, portNumber{"9999"}, server{&listener}
	{
	}
};

TEST_F(ServerTest, Connect)
{
	EXPECT_CALL(listener, createListeningSocket(portNumber))
	.Times(1);

	EXPECT_CALL(listener, startListening)
	.Times(1);

	server.connect(portNumber);
}

TEST_F(ServerTest, ReceiveMessage)
{
	std::string receivedData {"this is a message"};
	int sockfd {1};

	EXPECT_CALL(listener, sendAllExcept(sockfd, receivedData))
	.Times(1);

	server.receive(sockfd, receivedData);
}

TEST_F(ServerTest, ReceiveSendfileFailure)
{
	std::string command {"/sendfile "};
	std::string fileName {"hello.txt"};
	int sockfd {1};

	testing::InSequence s;

	EXPECT_CALL(listener, sendAllExcept(sockfd, "/receivefile " + fileName));

	EXPECT_CALL(listener, poll)
	.WillOnce(testing::Return(-1));

	EXPECT_CALL(listener, sendData(sockfd, "/rejectfile"));

	server.receive(sockfd, command + fileName);
}