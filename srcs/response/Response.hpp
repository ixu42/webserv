/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:51 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/13 13:47:22 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../utils/Utility.hpp"
#include "../config/Config.hpp"
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h> // for access()

class Response
{
	private:
		std::string	_body;
		std::string	_status;
		std::string	_type;
		std::map<std::string, std::string> _headers;
		int			_contentLength = 0;

	public:
		Response();
		Response(int code, ServerConfig* serverConfig, std::map<std::string, std::string> optionalHeaders = {});
		Response(int code, std::string filePath);

		std::string& getBody();
		std::string& getStatus();
		std::string& getType();
		std::string getHeader(const std::string& key);
		int getContentLength() const;

		void setBody(std::string body);
		void setStatus(std::string status);
		void setStatusFromCode(int code);
		void setType(std::string type);
		void setTypeFromFormat(std::string format);
		void setContentLength(int contentLength);
		void setHeader(const std::string& key, std::string& value);
		
		void appendToBody(char* data, std::size_t length);
		static std::string buildResponse(Response& response);
};
