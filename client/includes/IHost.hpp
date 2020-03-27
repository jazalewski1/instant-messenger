#pragma once

#include <string>

class IHost
{
public:
	virtual ~IHost() {};

	virtual int createSocket() = 0;
	
	virtual int conn(const std::string& ipAddress, int portNumber) = 0;
	
	virtual long int receiveNonblocking(char* buffer, int bufferSize) = 0;
	
	virtual long int receiveBlocking(char* buffer, int bufferSize) = 0;
	
	virtual long int sendData(const std::string& data) = 0;
};