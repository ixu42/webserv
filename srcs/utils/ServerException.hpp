/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:10:43 by vshchuki          #+#    #+#             */
/*   Updated: 2024/08/05 13:26:26 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <exception>
#include <string>
#include <map>

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
