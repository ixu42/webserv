/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:51 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/08 17:39:02 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

	#pragma once

	#include "../utils/Utility.hpp"
	#include <map>
	#include <string>
	#include <sstream>
	#include <iostream>

	class Response
	{
		private:
			bool								_CGIflag = false; // wtf??
			std::string							_body;
			std::string							_status;
			std::string							_type;
			std::map<std::string, std::string>	_headers;
			int									_contentLength = 0;

		public:
			Response();
			Response(int code, std::map<std::string, std::string> optionalHeaders = {});
			Response(int code, std::string filePath);

			std::string& getBody();
			std::string& getStatus();
			std::string& getType();
			bool& getCGIflag();
			int getContentLength() const;

			void setBody(std::string body);
			void setStatus(std::string status);
			void setStatusFromCode(int code);
			void setType(std::string type);
			void setTypeFromFormat(std::string format);
			void setCGIflag(bool CGIflag);
			void setContentLength(int contentLength);
			
			void appendToBody(char* data, std::size_t length);
			static std::string buildResponse(Response& response);
	};
