/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:46 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/03 19:23:00 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

// HTTP/1.1 404 Not Found
// Date: Sun, 18 Oct 2012 10:36:20 GMT
// Server: Apache/2.2.14 (Win32)
// Content-Length: 230
// Connection: Closed
// Content-Type: text/html; charset=iso-8859-1

// body

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

void Response::appendToBody(char* data, std::size_t length)
{
	_body.append(data, length);
}

std::string Response::buildResponse(Response& response)
{
	std::stringstream responseNew;

	responseNew << "HTTP/1.1 " << response.getStatus() << "\r\n";
	responseNew << Utility::getDate() << "\r\n";
	responseNew << "Server: webserv" << "\r\n";
	responseNew << "Content-Length: " << response.getBody().size() << "\r\n";
	responseNew << "Content-Type: " << response.getType() << "\r\n";
	responseNew << "\r\n";
	responseNew << response.getBody() << "\r\n";
	return responseNew.str();
}
