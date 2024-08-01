/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 00:51:19 by vshchuki          #+#    #+#             */
/*   Updated: 2024/08/01 14:01:23 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerException.hpp"

ServerException::ServerException(const std::string message) : _message(message) {
	_errno = errno;
}

const char* ServerException::what() const noexcept
{
	return _message.c_str();
}

int ServerException::getErrno() const
{
	return _errno;
}

ProcessingError::ProcessingError(int code, std::map<std::string, std::string> optionalHeaders, std::string message) : ServerException("Response error")
{
	_code = code;
	_headers = optionalHeaders;
	_message = message;
}

int ProcessingError::getCode() const
{
	return _code;
}

std::map<std::string, std::string> ProcessingError::getHeaders() const
{
	return _headers;
}