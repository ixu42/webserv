/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 00:51:19 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/08 16:21:20 by ixu              ###   ########.fr       */
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

ResponseError::ResponseError(int code, std::map<std::string, std::string> optionalHeaders) : ServerException("Response error")
{
	_code = code;
	_headers = optionalHeaders;
}

int ResponseError::getCode() const
{
	return _code;
}

std::map<std::string, std::string> ResponseError::getHeaders() const
{
	return _headers;
}