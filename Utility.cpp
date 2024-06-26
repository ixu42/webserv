#include "Utility.hpp"

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

	int fd = open(filePath.c_str(), O_RDONLY);
	if (fd == -1)
	{
		// std::cerr << "Error: " << strerror(errno) << std::endl;
		throw ServerException("Error: " + std::string(strerror(errno)));
	}

	int stringSize = 1024;
	char buffer[stringSize];
	int bytesRead;

	while (1)
	{
		bytesRead = read(fd, buffer, stringSize);
		if (bytesRead <= 0)
			break;
		result.append(buffer, bytesRead);
	}

	close(fd);

	return result;
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