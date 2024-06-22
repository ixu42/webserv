#pragma once

#include <map>
#include <string>
#include <sstream>
#include <algorithm>

#include <iostream>

class Request
{
	typedef std::map<std::string, std::string> QueryStringParameters;
	private:
		QueryStringParameters startLine;
		QueryStringParameters headers;
		std::string body; // ???

	public:
		Request();
		Request(std::string request);

		void parse(std::string request);
		
		QueryStringParameters getStartLine();
		QueryStringParameters getHeaders();
};
