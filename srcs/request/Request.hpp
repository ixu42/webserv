/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:40 by vshchuki          #+#    #+#             */
/*   Updated: 2024/08/05 13:25:42 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include "../network/Client.hpp"
#include "../utils/Utility.hpp"
#include "../utils/logUtils.hpp"
#include "../config/Config.hpp"
#include "../utils/UrlEncoder.hpp"

// Forward declaration of the Client class
class Client;

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
		Request(Client& client);

		void					parse(Client& client);

		/* Getters and setters */
		QueryStringParameters	getStartLine();
		QueryStringParameters	getHeaders();
		std::string				getBody();
		void					setHeader(std::string key, std::string value);

		void					printRequest();
};
