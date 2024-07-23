/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utility.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:11:23 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/23 17:35:20 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utility.hpp"

std::string Utility::replaceWhiteSpaces(std::string str, char newChar)
{
	for (char& c: str)
	{
		if (std::isspace(c))
			c = newChar;
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

std::string Utility::trimChars(std::string str, std::string chars)
{
	std::string result = str;
	for (char& symbol : chars)
	{
		std::string::const_iterator start = std::find_if_not(result.begin(), result.end(), [symbol](char c) {
			return symbol == c;
		});
		std::string::const_iterator end = std::find_if_not(result.rbegin(), result.rend(), [symbol](char c) {
			return symbol == c;
		}).base();
		result = start < end ? std::string(start, end) : "";
	}
	return result;
}



// Splits string with string delimiter
std::vector<std::string> Utility::splitStr(const std::string &str, const std::string &delimiter)
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

std::string Utility::strToUpper(std::string str)
{
	for (char& c: str)
		c = std::toupper(c);

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

std::string	Utility::replaceStrInStr(std::string dest, const std::string& str1, const std::string& str2)
{
	return std::regex_replace(dest, std::regex(str1), str2);
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

std::pair<std::vector<uint8_t>, size_t> Utility::readBinaryFile(const std::string& filePath)
{
	// Open the file in binary mode at the end to get the file size
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (!file) {
		throw ResponseError(404, {}, "Exception (no file) has been thrown in readBinaryFile() "
			"method of Utility class");
	}

	// Get the size of the file
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	// Read the contents of the file into a vector
	std::vector<uint8_t> buffer(size);
	if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
		throw ResponseError(500, {}, "Exception (reading error) has been thrown in readBinaryFile() "
			"method of Utility class");
	}

	return {buffer, static_cast<size_t>(size)};
}
void Utility::createFile(std::string filename, std::string content)
{
	std::ofstream outFile(filename);
	if (!outFile) {
		throw ServerException("Error: Could not create the file!");
	}
	outFile << content;
	outFile.close();
}
