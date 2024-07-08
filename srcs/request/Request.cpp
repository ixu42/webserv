/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:37 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/08 10:55:10 by ixu              ###   ########.fr       */
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

Request::Request(std::string request)
{
	parse(request);
}

void Request::parse(std::string request)
{
	int emptyLinePosition = request.find("\r\n\r\n");
	if (emptyLinePosition == -1)
		emptyLinePosition = request.find("\n\n");
	// std::cout << "Empty line position: " << emptyLinePosition << std::endl;

	if (emptyLinePosition != -1)
	{
		std::string headers = Utility::trim(request.substr(0, emptyLinePosition));
		std::string body = Utility::trim(request.substr(emptyLinePosition + 2));

		// Split headers into lines
		std::vector<std::string> headerLines = Utility::splitString(headers, "\n");

		// Parse the start line
		std::vector<std::string> startLineSplit = Utility::splitString(headerLines[0], " ");
		std::vector<std::string> querySplit = Utility::splitString(startLineSplit[1], "?");

 		_startLine["method"] = Utility::trim(startLineSplit[0]);
		_startLine["path"] = Utility::trim(querySplit[0]);
		_startLine["version"] = Utility::trim(startLineSplit[2]);
		if (querySplit.size() == 2)
			_startLine["query"] = Utility::trim(querySplit[1]);
		else
			_startLine["query"] = "";

		// Parse the headers and convert to lower case
		for (unsigned int i = 1; i < headerLines.size(); i++)
		{
			// if (headerLines[i].empty())
			// 	continue;
			std::vector<std::string> headerSplit = Utility::splitString(headerLines[i], ": ");
			std::string key = Utility::strToLower(Utility::trim(headerSplit[0]));
			std::string value = Utility::trim(headerSplit[1]);
			_headers[key] = value;
			LOG_DEBUG("Header: ", key, " Value: ", value);
		}

		// Parse the body
		_body = Utility::trim(body);
		// Unchunk the body if necessary
		if (_headers.find("transfer-encoding") != _headers.end() && Utility::strToLower(_headers["transfer-encoding"]) == "chunked")
		{
			_body = unchunkBody(body);
		}
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
	std::string unchunkedData;
	std::istringstream stream(string);

	while (true)
	{
		std::string chunkSizeStr = Utility::readLine(stream);
		size_t chunkSize = hexStringToSizeT(chunkSizeStr);
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

void	Request::printRequest()
{
	LOG_DEBUG("Request::printRequest() called");
	for (auto& kv : getStartLine())
		LOG_DEBUG("Start Line: ", kv.first, " = ", kv.second);
	for (auto& [key, value] : getHeaders())
		LOG_DEBUG("Header: ", key, " = ", value);
	LOG_DEBUG_RAW("[DEBUG] Body: ", "\n");
	LOG_DEBUG_RAW(getBody());
}

// Request should be at least start line, Host, Connection

//echo -e "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n" | nc google.com 80


// GET / HTTP/1.1
// Host: localhost:8080
// Connection: keep-alive
// Cache-Control: max-age=0
// sec-ch-ua: "Not/A)Brand";v="8", "Chromium";v="126", "Google Chrome";v="126"
// sec-ch-ua-mobile: ?0
// sec-ch-ua-platform: "macOS"
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
// Sec-Fetch-Site: none
// Sec-Fetch-Mode: navigate
// Sec-Fetch-User: ?1
// Sec-Fetch-Dest: document
// Accept-Encoding: gzip, deflate, br, zstd
// Accept-Language: en-GB,en-US;q=0.9,en;q=0.8,ru;q=0.7
// Cookie: wp-settings-1=libraryContent%3Dbrowse%26posts_list_mode%3Dlist; wp-settings-time-1=1697667275; adminer_permanent=c2VydmVy--cm9vdA%3D%3D-bG9jYWw%3D%3AWdDaEmjuEAY%3D

