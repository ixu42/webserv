/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utility.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:11:26 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/01 19:11:26 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <sstream>
#include <fstream>

#include <vector>
#include <algorithm>

#include <cstring>
#include <ctime>


#include "ServerException.hpp"

class Utility
{
	public:
		static std::string				replaceWhiteSpaces(std::string str);
		static std::string				trim(std::string str);
		static std::vector<std::string>	splitString(const std::string &request, const std::string &delimiter);
		static std::string				strToLower(std::string str);
		static std::string				readFile(std::string filePath);
		static std::string				getDate();
};
