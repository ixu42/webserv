/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 00:51:19 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/06 15:06:02 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerException.hpp"

ServerException::ServerException(const std::string message) : _message(message) {}

const char* ServerException::what() const noexcept
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