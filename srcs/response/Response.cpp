/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:46 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/29 12:57:26 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "../config/Config.hpp"

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status

static const std::string serverName = "webserv";
static const std::string charset = "; charset=UTF-8";
static const std::map<int, std::string> statusMessages = {
	{200, "OK"},
	{201, "Created"},
	{204, "No Content"},
	{307, "Temporary Redirect"},
	{400, "Bad Request"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"}, // if the location does not allowes method in request. Then put "Allowed: GET, POST" in response header
	{411, "Length Required"}, // Content-Length not provided
	{413, "Payload Too Large"}, // if the request body size exceeds the clientMaxBodySize
	{500, "Internal Server Error"}, // can be used when the server runs into unexpected issues processing the request, including memory allocation failures
	{505, "HTTP Version Not Supported"}
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

Response::Response() {}

Response::Response(int code, ServerConfig* serverConfig, std::map<std::string, std::string> optionalHeaders)
{
	std::string errorPagePath = serverConfig->defaultPages[404]; // fallback for not legit error codes
	if (code >= 300 && code <= 399)
		errorPagePath = "";
	auto errorIt = serverConfig->errorPages.find(code);
	auto defaultIt = serverConfig->defaultPages.find(code);
	if (errorIt != serverConfig->errorPages.end() && access(errorIt->second.c_str(), R_OK) == 0)
	{
		errorPagePath = serverConfig->errorPages[code];
	}
	else if (defaultIt != serverConfig->defaultPages.end())
	{
		LOG_DEBUG(TEXT_GREEN, "default page found", RESET);
		errorPagePath = serverConfig->defaultPages[code];
	}
	*this = Response(code, errorPagePath, optionalHeaders);
}

Response::Response(int code, std::string filePath, std::map<std::string, std::string> optionalHeaders)
{
	if (optionalHeaders.size() > 0)
		_headers.insert(optionalHeaders.begin(), optionalHeaders.end());

	std::string fileContent;
	size_t size = 0;
	// filePath can be empty for 3XX Status request as it does not require body
	setStatusFromCode(code);
	if (!filePath.empty())
	{
		if (access(filePath.c_str(), F_OK) == -1) // Fallback, in case default pages/404.html file is missing
		{
			setStatusFromCode(404);
			fileContent = "<!DOCTYPE html><html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1><hr /><p>Last resort.</p><p>Nothing found.</p></body></html>"; 
		}
		else if (access(filePath.c_str(), R_OK) == -1) // Fallback, in case default pages/404.html file has no permission to read
		{
			setStatusFromCode(403);
			fileContent = "<!DOCTYPE html><html><head><title>403 Forbidden</title></head><body><h1>403 Forbidden</h1><hr /><p>Last resort.</p><p>No permission.</p></body></html>"; 
		}
		size = fileContent.size();


		if (access(filePath.c_str(), R_OK) == 0)
		{
			LOG_DEBUG("Utility::readBinaryFile() called");
			auto [binaryFile, binarySize] = Utility::readBinaryFile(filePath);
			LOG_DEBUG("Utility::readBinaryFile() finished");
			size = binarySize;
			fileContent = std::string(binaryFile.begin(), binaryFile.end());

			size_t dotPos = filePath.find_last_of(".");

			if (dotPos != std::string::npos && mimeTypes.find(filePath.substr(filePath.find_last_of(".") + 1)) != mimeTypes.end())
				setType(mimeTypes.at(filePath.substr(filePath.find_last_of(".") + 1)));
			else
				setType(mimeTypes.at("default"));
		}
	}
	setBody(fileContent);
	setContentLength(size);
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

int Response::getContentLength() const
{
	return _contentLength;
}

std::string Response::getHeader(const std::string& key)
{
	return _headers[key];
}

void Response::setBody(std::string body)
{
	_body = body;
}

void Response::setStatus(std::string status)
{
	_status = status;
}

void Response::setStatusFromCode(int code)
{
	_status = std::to_string(code) + " " + statusMessages.at(code);
}

void Response::setType(std::string type)
{
	_type = type;
}

void Response::setTypeFromFormat(std::string format)
{
	try
	{
		_type = mimeTypes.at(format);
	}
	catch (const std::exception& e)
	{
		_type = mimeTypes.at("default");
	}
}

void Response::setContentLength(int contentLength)
{
	_contentLength = contentLength;
}

void Response::setHeader(const std::string& key, std::string& value)
{
	_headers[key] = value;
}

void Response::appendToBody(char* data, size_t length)
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
	responseNew << "Date: " << Utility::getDate() << "\r\n";
	responseNew << "Server: webserv" << "\r\n";
	responseNew << "Connection: close" << "\r\n";
	responseNew << "Content-Length: " << response.getBody().size() << "\r\n";

	if (!response.getType().empty())
		responseNew << "Content-Type: " << response.getType() << "\r\n";

	/* Add optional headers*/
	for (auto& [headerKey, headerValue] : response._headers)
	{

		responseNew << headerKey << ": " << headerValue << "\r\n";
		LOG_DEBUG("headerKey: ", headerKey, ", headerValue: ", headerValue);
	}
	LOG_DEBUG("response so far: ", responseNew.str());
	responseNew << "\r\n";

	/* Not adding extra line if body is empty*/
	if (response.getBody().size() != 0)
		responseNew << response.getBody() << "\r\n";
	return responseNew.str();
}
