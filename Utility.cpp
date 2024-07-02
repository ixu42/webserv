/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utility.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:11:23 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/02 14:42:52 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utility.hpp"

std::string Utility::replaceWhiteSpaces(std::string str)
{
	for (char& c: str)
	{
		if (std::isspace(c))
			c = ' ';
	}
	return str;
}


// Function to trim whitespace from both ends of a string
std::string Utility::trim(std::string str)
{
	std::string::const_iterator start = std::find_if_not(str.begin(), str.end(), [](int c) {
		return std::isspace(c);
	});
	std::string::const_iterator end = std::find_if_not(str.rbegin(), str.rend(), [](int c) {
		return std::isspace(c);
	}).base();
	return (start < end ? std::string(start, end) : "");
}

// Splits string with string delimiter
std::vector<std::string> Utility::splitString(const std::string &str, const std::string &delimiter)
{
	std::vector<std::string> seglist;
	std::string segment;
	size_t start = 0, end = 0;

	if (str.empty())
		return seglist;

	while ((end = str.find(delimiter, start)) != std::string::npos)
	{
		segment = str.substr(start, end - start);
		if (segment.length() > 0)
			seglist.push_back(segment);
		start = end + delimiter.length();
	}
	// Add the last segment
	seglist.push_back(str.substr(start));

	return seglist;
}


std::string Utility::strToLower(std::string str)
{
	for (char& c: str)
		c = std::tolower(c);

	return str;
}

std::string Utility::readFile(std::string filePath)
{
	std::string result;
	std::ifstream file(filePath);

	if (!file.is_open())
		throw ServerException("Can not open file: " + filePath);

	// Create a string stream to hold the file contents
	std::ostringstream ss;

	// Read the file contents into the string stream
	ss << file.rdbuf();

	file.close();

	return ss.str();
}


// Date: Sun, 18 Oct 2012 10:36:20 GMT
std::string Utility::getDate()
{
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = gmtime(&rawtime);

	strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);

	return std::string(buffer);
}

std::string Utility::readLine(std::istream &stream)
{
	std::string line;
	std::getline(stream, line);
	if (!line.empty() && line.back() == '\r')
	{
		line.pop_back();
	}
	return line;
}