/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:40 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/02 15:39:55 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <sstream>
#include <iostream>

#include "Utility.hpp"
#include "Config.hpp"

class Request
{
	typedef std::map<std::string, std::string> QueryStringParameters;
	private:
		QueryStringParameters	_startLine;
		QueryStringParameters	_headers;
		std::string				_body; // ???

		Location* location;

	public:
		Request();
		Request(std::string request);

		void					parse(std::string request);

		/* Unchunk request */
		size_t					hexStringToSizeT(const std::string &hexStr);
		std::string				unchunkBody(std::istream& stream);

		/* Getters and setters */
		QueryStringParameters	getStartLine();
		QueryStringParameters	getHeaders();
		std::string				getBody();

		void					setLocation(Location* location);
};
