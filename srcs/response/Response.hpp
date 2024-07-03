/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:51 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/03 16:13:14 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../utils/Utility.hpp"
#include <map>
#include <string>

class Response
{
	private:
		static const std::string						serverName;
		static const std::map<std::string, std::string>	statusMessages;
		static const std::map<std::string, std::string>	mimeTypes;

		std::map<std::string, std::string>				_startline;
		std::map<std::string, std::string>				_headers;
		std::string										_body;

	public:
		Response();
		// Response(std::string statusCode, std::string body, std::string fileFormat);
		std::string	getBody();
		void appendToBody(char* data, std::size_t length);
};
