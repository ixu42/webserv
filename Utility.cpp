#include "Utility.hpp"

// Function to trim whitespace from both ends of a string
std::string Utility::trim(const std::string& str)
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
std::vector<std::string> Utility::splitString(const std::string &request, const std::string &delimiter)
{
	std::vector<std::string> seglist;
	std::string segment;
	size_t start = 0, end = 0;

	while ((end = request.find(delimiter, start)) != std::string::npos)
	{
		segment = request.substr(start, end - start);
		if (segment.length() > 0)
			seglist.push_back(segment);
		start = end + delimiter.length();
	}
	// Add the last segment
	seglist.push_back(request.substr(start));

	return seglist;
}