/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UrlEncoder.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/19 14:31:12 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/19 18:10:49 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "UrlEncoder.hpp"

static const std::map<char, std::string> encodeMap =
{
	{' ', "%20"},
	{'!', "%21"},
	{'#', "%23"},
	{'$', "%24"},
	{'%', "%25"},
	{'\'', "%27"},
	{'(', "%28"},
	{')', "%29"},
	{'*', "%2A"},
	{'+', "%2B"},
	{',', "%2C"},
	{':', "%3A"},
	{';', "%3B"},
	{'=', "%3D"},
	{'?', "%3F"},
	{'[', "%5B"},
	{'@', "%40"},
	{'\\', "%5C"}, // Include backslash
	{']', "%5D"}
};

static const std::map<std::string, char> decodeMap = {
	{"%20", ' '},
	{"%21", '!'},
	{"%23", '#'},
	{"%24", '$'},
	{"%27", '\''},
	{"%28", '('},
	{"%29", ')'},
	{"%2A", '*'},
	{"%2B", '+'},
	{"%2C", ','},
	{"%3A", ':'},
	{"%3B", ';'},
	{"%3D", '='},
	{"%3F", '?'},
	{"%5B", '['},
	{"%40", '@'},
	{"%5C", '\\'},
	{"%5D", ']'},
	{"%25", '%'},
};

std::string UrlEncoder::encode(const std::string& str)
{
	std::ostringstream encoded;
	for (char c : str) {
		auto it = encodeMap.find(c);
		if (it != encodeMap.end()) {
			encoded << it->second;
		} else {
			encoded << c;
		}
	}
	return encoded.str();
}

std::string UrlEncoder::decode(const std::string& str) {
	std::string decoded;
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '%' && i + 2 < str.length()) {
			std::string code = str.substr(i, 3);
			auto it = decodeMap.find(code);
			if (it != decodeMap.end()) {
				decoded += it->second;
				i += 2; // Skip the next two characters
			} else {
				decoded += str[i];
			}
		} else {
			decoded += str[i];
		}
	}
	return decoded;
}