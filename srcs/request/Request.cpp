/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:37 by vshchuki          #+#    #+#             */
/*   Updated: 2024/08/06 15:31:13 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

#include <string>
#include <vector>
#include <sstream>

Request::Request()
{
	LOG_DEBUG("Request default constructor called");
	_startLine["method"] = "";
	_startLine["path"] = "";
	_startLine["version"] = "";
	_startLine["query"] = "";
}

Request::Request(Client& client)
{
	LOG_DEBUG("Request constructor called");
	parse(client);
}

void Request::parseHeaders(std::vector<std::string> headerLines)
{
	for (unsigned int i = 1; i < headerLines.size(); i++)
	{
		size_t colonPos = headerLines[i].find(':');
		if (colonPos != std::string::npos) {
			std::string key = headerLines[i].substr(0, colonPos);
			std::string value = headerLines[i].substr(colonPos + 1);
			key = Utility::strToLower(Utility::trim(key));
			value = Utility::trim(value);
			_headers[key] = value;
		}
	}
}

void Request::parseBody(Client& client)
{

	if (client.getIsBodyRead())
	{
		std::string body = Utility::trim(client.getRequestString().substr(client.getEmptyLinePos() + 2));
		// Unchunk the body if necessary
		if (_headers.find("transfer-encoding") != _headers.end()
			&& Utility::strToLower(_headers["transfer-encoding"]) == "chunked")
		{
			_body = unchunkBody(body);
		}
		else
		{
			// Parse the body
			_body = Utility::trim(body);
		}
	}
}

void Request::parse(Client& client)
{
	LOG_DEBUG("Request::parse() called");

	if (client.getEmptyLinePos() != -1)
	{
		std::string headers = Utility::trim(client.getRequestString().substr(0, client.getEmptyLinePos()));
		// Split headers into lines
		std::vector<std::string> headerLines = Utility::splitStr(headers, "\n");
		LOG_DEBUG("Request::parse() splitting headerLines");
		// Check if headers are not empty
		if (headers.size() < 1)
			return;
		// Parse the start line
		std::vector<std::string> startLineSplit = Utility::splitStr(headerLines[0], " ");
		// Check start line after split
		if (startLineSplit.size() != 3)
			return;
		std::vector<std::string> querySplit = Utility::splitStr(startLineSplit[1], "?");

 		_startLine["method"] = Utility::trim(startLineSplit[0]);
		_startLine["path"] = UrlEncoder::decode(Utility::trim(querySplit[0]));
		_startLine["path_info"] = UrlEncoder::decode(Utility::trim(startLineSplit[1]));
		_startLine["version"] = Utility::trim(startLineSplit[2]);
		if (querySplit.size() == 2)
			_startLine["query"] = Utility::trim(querySplit[1]);
		else
			_startLine["query"] = "";

		parseHeaders(headerLines);
		parseBody(client);

	}
}

/*
In HTTP/1.1, chunked transfer encoding strictly requires the use of \r\n
(carriage return and line feed) to delimit the chunk size and the actual
data. This specification is defined in the HTTP/1.1 standard (RFC 7230,
Section 4.1). Using just \n (line feed) would not conform to the standard
and could result in improper parsing by HTTP clients and servers.
*/

size_t Request::hexStringToSizeT(const std::string &hexStr)
{
	std::istringstream iss(hexStr);
	size_t size;
	iss >> std::hex >> size;
	return size;
}

std::string Request::unchunkBody(std::string& string)
{
	LOG_DEBUG("Request::unchunkBody() called");
	std::string unchunkedData;
	std::istringstream stream(string);

	int count = 0;

	while (true)
	{
		count++;
		std::string chunkSizeStr = Utility::readLine(stream);
		size_t chunkSize = hexStringToSizeT(chunkSizeStr);

		LOG_DEBUG(TEXT_YELLOW, "chunkSize: ", chunkSize, RESET);

		if (chunkSize == 0) {
			break;
		}

		// Read the chunk data using substr
		std::string chunkData;
		chunkData.reserve(chunkSize);

		for (size_t i = 0; i < chunkSize; ++i) {
			char c;
			stream.get(c);
			chunkData += c;
		}
		unchunkedData += chunkData;

		// Read the trailing \r\n after chunk data
		std::string crlf;
		std::getline(stream, crlf);
		LOG_DEBUG(unchunkedData);
		if (count > 300)
			break;
	}

	return unchunkedData;
}


/**
 * Getters
 */

Request::QueryStringParameters Request::getStartLine()
{
	return _startLine;
}

Request::QueryStringParameters Request::getHeaders()
{
	return _headers;
}

std::string Request::getBody()
{
	return _body;
}


void	Request::setHeader(std::string key, std::string value)
{
	_headers[key] = value;
}

void	Request::printRequest()
{
	int limitRequestString = 2000;

	LOG_DEBUG("Request::printRequest() called");
	for (auto& kv : getStartLine())
		LOG_DEBUG("Start Line: ", kv.first, " = ", kv.second);
	for (auto& [key, value] : getHeaders())
		LOG_DEBUG("Header: ", key, " = ", value);
	LOG_DEBUG_RAW("[DEBUG] Body: ", "\n");
	LOG_DEBUG_RAW(getBody().substr(0, limitRequestString));
	LOG_DEBUG_RAW("\n...\n");
}
