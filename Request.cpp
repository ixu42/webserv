#include "Request.hpp"

#include <string>
#include <vector>
#include <sstream>

// std::string request = "GET / HTTP/1.1\nHost: localhost:8080\nConnection: keep-alive\nCache-Control: max-age=0";

// std::vector<std::string> splitString(std::string request, std::string delimiter = "\n")
// {
// 	std::stringstream test(request);
// 	std::string segment;
// 	std::vector<std::string> seglist;

// 	while(std::getline(test, segment, delimiter[0]))
// 	{
// 		seglist.push_back(segment);
// 	}

// 	return seglist;
// }

// Function to trim whitespace from both ends of a string
std::string trim(const std::string& str) {
	std::string::const_iterator start = std::find_if_not(str.begin(), str.end(), [](int c) {
		return std::isspace(c);
	});
	std::string::const_iterator end = std::find_if_not(str.rbegin(), str.rend(), [](int c) {
		return std::isspace(c);
	}).base();
	return (start < end ? std::string(start, end) : "");
}

std::vector<std::string> splitString(const std::string &request, const std::string &delimiter = "\n") {
	std::vector<std::string> seglist;
	std::string segment;
	size_t start = 0, end = 0;

	while ((end = request.find(delimiter, start)) != std::string::npos)
	{
		segment = request.substr(start, end - start);
		seglist.push_back(segment);
		start = end + delimiter.length();
	}
	// Add the last segment
	seglist.push_back(request.substr(start));

	return seglist;
}


Request::Request(std::string request)
{
	parse(request);
}

void Request::parse(std::string request)
{
	// std::cout << "Request: " << request << "||||||||" << std::endl;

	// Split start line, headers, and body:
	// substr until empty line

	// int emptyLinePosition = request.find("\n\n");
	// std::cout << "Empty line position: " << emptyLinePosition << std::endl;

	int emptyLinePosition = request.find("\r\n\r\n");
	std::cout << "Empty line position: " << emptyLinePosition << std::endl;

	if (emptyLinePosition != -1)
	{
		std::string headers = request.substr(0, emptyLinePosition);
		std::string body = request.substr(emptyLinePosition + 2);

		// Split headers into lines
		std::vector<std::string> headerLines = splitString(request);

		// Parse the start line
		std::vector<std::string> startLineSplit = splitString(headerLines[0], " ");

		this->startLine["method"] = trim(startLineSplit[0]);
		this->startLine["target"] = trim(startLineSplit[1]);
		this->startLine["version"] = trim(startLineSplit[2]);

		// Parse the headers
		for (unsigned int i = 1; i < headerLines.size(); i++)
		{
			std::vector<std::string> headerSplit = splitString(headerLines[i], ": ");
			this->headers[trim(headerSplit[0])] = trim(headerSplit[1]);
		}

		// Parse the body
		this->body = trim(body);
	}
}

Request::QueryStringParameters Request::getStartLine()
{
	return this->startLine;
}
Request::QueryStringParameters Request::getHeaders()
{
	return this->headers;
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

