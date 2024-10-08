/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utility.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:11:26 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/28 20:33:58 by dnikifor         ###   ########.fr       */
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

#include <utility> // For std::pair
#include <stdint.h> // for uint8_t

#include <regex>

#include "ServerException.hpp"
#include "logUtils.hpp"

class Utility
{
	public:
		static std::string								replaceWhiteSpaces(std::string str, char newChar);
		static std::string								trim(std::string str);
		static std::string								trimChars(std::string str, std::string chars);
		static std::vector<std::string>					splitStr(const std::string &str, const std::string &delimiter);
		static std::string								strToLower(std::string str);
		static std::string								strToUpper(std::string str);
		static std::string								readFile(std::string filePath);
		static std::string								getDate();
		static std::string								replaceStrInStr(std::string dest, const std::string& str1, const std::string& str2);
		static std::string								readLine(std::istream &stream);
		static std::pair<std::vector<uint8_t>, size_t>	readBinaryFile(const std::string& filePath);
		static void										createFile(std::string filename, std::string content);
		static bool										argvCheck(int argc, char *argv[], std::string& configFile);
};
