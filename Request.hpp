#pragma once

#include <map>
#include <string>
#include <sstream>
#include <algorithm>

class Request
{
	typedef std::map<std::string, std::string> QueryStringParameters;
	private:
		QueryStringParameters startLine;
		QueryStringParameters headers;
		std::string body; // ???

	public:
		Request();
		void parse(std::string request);
};
