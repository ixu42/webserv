/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 00:51:19 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/09 16:02:37 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerException.hpp"

ServerException::ServerException(const std::string message) : _message(message) {}

const char* ServerException::what() const noexcept
{
	return _message.c_str();
}

SignalException::SignalException(const std::string& message) : _message(message) {}

const char* SignalException::what() const noexcept
{
	return _message.c_str();
}

ResponseError::ResponseError(int code, std::map<std::string, std::string> optionalHeaders, std::string message) : ServerException("Response error")
{
	_code = code;
	_headers = optionalHeaders;
	_message = message;
}

int ResponseError::getCode() const
{
	return _code;
}

std::map<std::string, std::string> ResponseError::getHeaders() const
{
	return _headers;
}