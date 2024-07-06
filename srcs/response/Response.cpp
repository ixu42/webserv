/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:46 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/06 03:10:07 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
static const std::string serverName = "webserv";
static const std::string charset = "; charset=UTF-8";
static const std::map<int, std::string> statusMessages = {
	{200, "OK"},
	{307, "Temporary Redirect"},
	{400, "Bad Request"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"}, // if the location does not allowes method in request. THen put "Allowed: GET, POST" in response header
	{413, "Request Entity Too Large"}, // if the request body size exceeds the clientMaxBodySize
	{500, "Internal Server Error"} // can be used when the server runs into unexpected issues processing the request, including memory allocation failures
};

static const std::map<std::string, std::string> mimeTypes = {
	{"html", "text/html" + charset},
	{"htm", "text/html" + charset},
	{"css", "text/css" + charset},
	{"js", "application/javascript" + charset},

	{"ttf", "font/ttf"},
	{"woff", "font/woff"},
	{"woff2", "font/woff2"},

	{"jpg", "image/jpeg"},
	{"jpeg", "image/jpeg"},
	{"png", "image/png"},
	{"gif", "image/gif"},
	{"svg", "image/svg+xml"},
	{"ico",	"image/x-icon"},
	{"bmp", "image/bmp"},
	{"tiff", "image/tiff"},
	{"webp", "image/webp"},

	{"mp3", "audio/mpeg"},
	{"wav", "audio/wav"},
	{"ogg", "audio/ogg"},

	{"mp4", "video/mp4"},
	{"webm", "video/webm"},
	{"ogg", "video/ogg"},

	{"txt", "text/plain" + charset},
	{"sitemap", "application/xml" + charset},
	{"json", "application/json" + charset},
	{"xml", "application/xml" + charset},
	{"csv", "text/csv" + charset},
	{"markdown", "text/markdown" + charset},
	{"pdf", "application/pdf"},
	{"zip", "application/zip"},
	{"gzip", "application/gzip"},

	// https://stackoverflow.com/questions/20508788/do-i-need-content-type-application-octet-stream-for-file-download
	{"default", "application/octet-stream"}
};

// HTTP/1.1 404 Not Found
// Date: Sun, 18 Oct 2012 10:36:20 GMT
// Server: Apache/2.2.14 (Win32)
// Content-Length: 230
// Connection: Closed
// Content-Type: text/html; charset=iso-8859-1

// body
// 200, 400, 403, 404, 405, 413, 500, 307

Response::Response() {}

Response::Response(int code, std::string filePath)
{
	auto [file, size] = Utility::readBinaryFile(filePath);
	setStatus(std::to_string(code) + statusMessages.at(code));

	setContentLength(size);

    std::string fileContent(file.begin(), file.end());

	size_t dotPos = filePath.find_last_of(".");

	if (dotPos != std::string::npos && mimeTypes.find(filePath.substr(filePath.find_last_of(".") + 1)) != mimeTypes.end())
	{
		setType(mimeTypes.at(filePath.substr(filePath.find_last_of(".") + 1)));
		setBody(fileContent);
	}
	else
	{
		setType(mimeTypes.at("default"));
		setBody(fileContent);
	}
}

std::string& Response::getBody()
{
	return _body;
}

std::string& Response::getStatus()
{
	return _status;
}

std::string& Response::getType()
{
	return _type;
}

bool& Response::getCGIflag()
{
	return _CGIflag;
}


int Response::getContentLength() const 
{
	return _contentLength;
}

void Response::setBody(std::string body)
{
	_body = body;
}

void Response::setStatus(std::string status)
{
	_status = status;
}

void Response::setType(std::string type)
{
	_type = type;
}

void Response::setCGIflag(bool CGIflag)
{
	_CGIflag = CGIflag;
}

void Response::setContentLength(int contentLength)
{
	_contentLength = contentLength;
}

void Response::appendToBody(char* data, std::size_t length)
{
	_body.append(data, length);
}

std::string Response::buildResponse(Response& response)
{
	std::stringstream responseNew;

	if (response.getStatus().empty())
	{
		response._status = "200 OK";
	}
	responseNew << "HTTP/1.1 " << response.getStatus() << "\r\n";
	responseNew << Utility::getDate() << "\r\n";
	responseNew << "Server: webserv" << "\r\n";
	// if (response.getContentLength() == 0)
	// 	response.setContentLength(response.getBody().size());
	responseNew << "Content-Length: " << response.getBody().size() << "\r\n";
	responseNew << "Content-Type: " << response.getType() << "\r\n";
	responseNew << "\r\n";
	if (response.getBody().size() != 0)
		responseNew << response.getBody() << "\r\n";
	return responseNew.str();
}
