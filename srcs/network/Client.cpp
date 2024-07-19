/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/19 12:29:37 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/19 14:55:51 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() :
		_fd(-1),
		_pid(-1),
		_parentPipe{-1, -1},
		_childPipe{-1, -1},
		_request(nullptr),
		_response(nullptr),
		_state(ClientState::READING),
		_stateCGI(CGIState::INIT),
		_maxClientBodyBytes(std::numeric_limits<std::size_t>::max()),
		_totalBytesWritten(0) {}

Client::~Client() {}

/**
 * Getters
 */

int Client::getFd()
{
	return _fd;
}

pid_t Client::getPid()
{
	return _pid;
}

int Client::getParentPipe(int index)
{
	if (index >= 0 && index < 2)
		return _parentPipe[index];
	throw std::out_of_range("Index out of range for parentPipe");
}

int* Client::getParentPipeWhole()
{
	return _parentPipe;
}

int Client::getChildPipe(int index)
{
	if (index >= 0 && index < 2)
		return _childPipe[index];
	throw std::out_of_range("Index out of range for childPipe");
}

int* Client::getChildPipeWhole()
{
	return _childPipe;
}

std::string Client::getCGIString()
{
	return _CGIString;
}

Request* Client::getRequest()
{
	return _request;
}

Response* Client::getResponse()
{
	return _response;
}

Client::ClientState Client::getState()
{
	return _state;
}

Client::CGIState Client::getCGIState()
{
	return _stateCGI;
}

std::string Client::getRequestString()
{
	return _requestString;
}

std::size_t Client::getMaxClientBodyBytes()
{
	return _maxClientBodyBytes;
}

std::string Client::getResponseString()
{
	return _responseString;
}

size_t Client::getTotalBytesWritten()
{
	return _totalBytesWritten;
}

/**
 * Setters
 */

void Client::setFd(int fd)
{
	_fd = fd;
}

void Client::setPid(pid_t pid)
{
	_pid = pid;
}

void Client::setParentPipe(int index, int fd)
{
	if (index >= 0 && index < 2)
		_parentPipe[index] = fd;
	else
		throw std::out_of_range("Index out of range for parentPipe");
}

void Client::setChildPipe(int index, int fd)
{
	if (index >= 0 && index < 2)
		_childPipe[index] = fd;
	else
		throw std::out_of_range("Index out of range for childPipe");
}

void Client::setCGIString(const std::string& cgiString)
{
	_CGIString = cgiString;
}

void Client::setRequest(Request* request)
{
	_request = request;
}

void Client::setResponse(Response* response)
{
	_response = response;
}

void Client::setState(ClientState state)
{
	_state = state;
}

void Client::setCGIState(CGIState stateCGI)
{
	_stateCGI = stateCGI;
}

void Client::setRequestString(const std::string& requestString)
{
	_requestString = requestString;
}

void Client::setMaxClientBodyBytes(std::size_t maxClientBodyBytes)
{
	_maxClientBodyBytes = maxClientBodyBytes;
}

void Client::setResponseString(const std::string& responseString)
{
	_responseString = responseString;
}

void Client::setTotalBytesWritten(size_t totalBytesWritten)
{
	_totalBytesWritten = totalBytesWritten;
}
