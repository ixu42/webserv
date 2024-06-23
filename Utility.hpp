#pragma once


#include <map>
#include <string>
// #include <sstream>
// #include <iostream>
#include <vector>
#include <algorithm>

class Utility
{
	public:
		static std::string trim(const std::string& str);
		static std::vector<std::string> splitString(const std::string &request, const std::string &delimiter);
};
