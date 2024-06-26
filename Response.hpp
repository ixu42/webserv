#pragma once

#include "Utility.hpp"
#include <map>
#include <string>

class Response
{
	private:
		static const std::string						serverName;
		static const std::map<std::string, std::string>	statusMessages;
		static const std::map<std::string, std::string>	mimeTypes;

		std::map<std::string, std::string>				_startline;
		std::map<std::string, std::string>				_headers;
		std::string										_body;
		Response() = delete;

	public:
		Response(std::string statusCode, std::string body, std::string fileFormat);
};
