#pragma once

#include <string>

struct Data
{
	long int bytes;
	const std::string data;

	Data(long int bytes_, const std::string& data_) :
		bytes{bytes_}, data{data_}
	{
	}
};