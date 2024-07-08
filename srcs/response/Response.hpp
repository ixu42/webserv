/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:51 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/08 19:38:26 by dnikifor         ###   ########.fr       */
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
			std::string	_body;
			std::string	_status;
			std::string	_type;
			std::map<std::string, std::string> _headers;
			int			_contentLength = 0;

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
