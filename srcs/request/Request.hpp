/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:40 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/25 15:21:11 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include "../utils/Utility.hpp"
#include "../utils/logUtils.hpp"
#include "../config/Config.hpp"
#include "../utils/UrlEncoder.hpp"

class Request
{
	typedef std::map<std::string, std::string> QueryStringParameters;
	private:
		QueryStringParameters	_startLine;
		QueryStringParameters	_headers;
		std::string				_body;

		/* Unchunk request */
		size_t					hexStringToSizeT(const std::string &hexStr);
		std::string				unchunkBody(std::string& body);
		
	public:
		Request();
		Request(std::string request);

		void					parse(std::string request);

		/* Getters and setters */
		QueryStringParameters	getStartLine();
		QueryStringParameters	getHeaders();
		std::string				getBody();
		void					setHeader(std::string key, std::string value);

		void					printRequest();
};
