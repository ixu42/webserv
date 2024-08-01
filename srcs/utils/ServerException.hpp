/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:10:43 by vshchuki          #+#    #+#             */
/*   Updated: 2024/08/01 14:01:23 by vshchuki         ###   ########.fr       */
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
	protected:
		std::string _message;
		int			_errno;

	public:
		explicit ServerException(const std::string message);

		const char*	what() const noexcept override;
		int	getErrno() const;
};

class ProcessingError: public ServerException
{
	private:
		int									_code;
		std::map<std::string, std::string>	_headers;

	public:
		ProcessingError(int code, std::map<std::string, std::string> optionalHeaders = {}, std::string message = "");
		int 								getCode() const;
		std::map<std::string, std::string>	getHeaders() const;
};
