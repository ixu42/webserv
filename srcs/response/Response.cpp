/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:46 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/01 19:08:47 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
static const std::string serverName = "webserv";
static const std::string charset = "; charset=UTF-8";
static const std::map<std::string, std::string> statusMessages = {
	{"200", "OK"},
	{"400", "Bad Request"},
	{"403", "Fodbidden"},
	{"404", "Not Found"},
	{"405", "Method Not Allowed"}, // if the location does not allowes method in request. THen put "Allowed: GET, POST" in response header
	{"413,", "Request Entity Too Large"}, // if the request body size exceeds the clientMaxBodySize
	{"500", "Internal Server Error"}, // can be used when the server runs into unexpected issues processing the request, including memory allocation failures
	{"307",	"Temporary Redirect"}
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

Response::Response() {}

// Response::Response(std::string statusCode, std::string body, std::string fileFormat)
// {
// 	_startline["version"] = "HTTP/1.1";
// 	_startline["status"] = statusCode;
// 	_startline["message"] = statusMessages.at(statusCode);

// 	_headers["date"] = Utility::getDate();
// 	_headers["server"] = "webserv";
// 	_headers["content-length"] = std::to_string(body.length());
// 	_headers["connection"] = "close";
// 	_headers["content-type"] = mimeTypes.at(fileFormat);

// 	_body = body;

// }

std::string	Response::getBody()
{
	return _body;
}

void Response::appendToBody(char* data, std::size_t length)
{
    _body.append(data, length);
}