/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/19 12:29:37 by dnikifor          #+#    #+#             */
/*   Updated: 2024/08/01 20:32:31 by vshchuki         ###   ########.fr       */
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
		_emptyLinePos(-1),
		_emptyLinesSize(0),
		_contentLengthNum(std::string::npos),
		_isHeadersRead(false),
		_isBodyRead(false),
		_maxClientBodyBytes(std::numeric_limits<size_t>::max()),
		_totalBytesWritten(0),
		_cgiStart(std::chrono::system_clock::now()) {}

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

std::string& Client::getRespBody()
{
	return _respBody;
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

bool Client::getIsHeadersRead()
{
	return _isHeadersRead;
}

bool Client::getIsBodyRead()
{
	return _isBodyRead;
}

int Client::getEmptyLinePos()
{
	return _emptyLinePos;
}
int Client::getEmptyLinesSize()
{
	return _emptyLinesSize;
}

size_t Client::getContentLengthNum()
{
	return _contentLengthNum;
}

size_t Client::getMaxClientBodyBytes()
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

std::chrono::_V2::system_clock::time_point Client::getCgiStart()
{
	return _cgiStart;
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

void Client::setIsHeadersRead(bool isHeadersRead)
{
	_isHeadersRead = isHeadersRead;
}

void Client::setIsBodyRead(bool isBodyRead)
{
	_isBodyRead = isBodyRead;
}

void		Client::setEmptyLinePos(int emptyLinePos)
{
	_emptyLinePos = emptyLinePos;
}

void		Client::setEmptyLinesSize(int emptyLinesSize)
{
	_emptyLinesSize = emptyLinesSize;
}

void		Client::setContentLengthNum(size_t contentLengthNum)
{
	_contentLengthNum = contentLengthNum;
}

void Client::setMaxClientBodyBytes(size_t maxClientBodyBytes)
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

void Client::setCgiStart(std::chrono::_V2::system_clock::time_point cgiStart)
{
	_cgiStart = cgiStart;
}
