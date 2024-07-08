/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:10:43 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/08 16:20:36 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
// #include <stdexcept>
#include <exception>
#include <string>
#include <map>

// class ServerException : public std::runtime_error
// {
// 	public:
// 		explicit ServerException(const std::string &message)
// 		: std::runtime_error(message) {}
// };

class ServerException : public std::exception
{
	private:
		std::string _message;

	public:
		explicit ServerException(const std::string message);

		const char* what() const noexcept override;
};

class SignalException : public std::exception
{
	private:
		std::string _message;

	public:
		explicit SignalException(const std::string& message);

		const char* what() const noexcept override;
};

class ResponseError: public ServerException
{
	private:
		int									_code;
		std::map<std::string, std::string>	_headers;

	public:
		ResponseError(int code, std::map<std::string, std::string> optionalHeaders = {});
		int 								getCode() const;
		std::map<std::string, std::string>	getHeaders() const;
};
