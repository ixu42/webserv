#include "ConfigValidator.hpp"

/**
 * Checks second parameter for unique values
 */
int ConfigValidator::checkUnique(std::string line)
{
	std::vector<std::string> lineSplit = Utility::splitString(Utility::replaceWhiteSpaces(line), " ");
	std::vector<std::string> errorCodesStrings = Utility::splitString(lineSplit[1], ",");
	std::set<std::string> uniqueErrorCodesStrings(errorCodesStrings.begin(), errorCodesStrings.end());
	if (errorCodesStrings.size() != uniqueErrorCodesStrings.size())
	{
		std::cout << "Line not valid: " << TEXT_RED << line << RESET << std::endl;
		return 1;
	}
	return 0;
}

/**
 * pattern1 matches the line and pattern2
 * Returns 1 if line is not valid
*/
int ConfigValidator::matchLinePattern(std::string& line, std::string field, std::regex pattern2)
{
	field = "^\\s*" + field + "\\b.*";
	std::regex pattern1(field);
	if (std::regex_match(line, pattern1) && !std::regex_match(line, pattern2))
	{
		std::cout << "Line not valid: " << TEXT_RED << line << RESET << std::endl;
		return 1;
	}
	return 0;
}

/**
 * Returns number of invalid lines
 */
int ConfigValidator::validateGeneralConfig(std::string generalConfig, std::vector<ServerConfig>& servers)
{
	int generalConfigErrorsCount = 0;

	// Cgi pattern is constructed from cgis map default keys
	std::string cgisString;
	size_t i = 0;
	for (auto& cgi : servers[0].cgis)
	{
		cgisString += cgi.first;
		if (i != servers[0].cgis.size() - 1)
			cgisString += "|";
		i++;
	}
	int cgisCount = servers[0].cgis.size();
	std::cout << "cgisString: " << cgisString << std::endl;


	std::string patternStr = "\\s*cgis\\s+\\b(" + cgisString + ")(?:,(" + cgisString + ")){0," + std::to_string(cgisCount - 1) +"}\\b\\s*";
	// std::string patternStr = "\\s*cgis\\s+\\b(" + cgisString + ")(?:,(" + cgisString + ")){0,2}\\b\\s*";
	// std::string patternStr = "\\s*cgis\\s+(" + cgisString +")(,(" + cgisString + "))?\\s*";	

	std::regex linePattern(R"((ipAddress|port|serverName|clientMaxBodySize|error|cgis)\s+[a-zA-Z0-9~\-_.,]+\s*[a-zA-Z0-9~\-_.,\/]*\s*)");
	std::map<std::string, std::regex> patterns = {
		{"ipAddress", std::regex(R"(\s*ipAddress\s+((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}\s*)")},
		{"port", std::regex(R"(\s*port\s+[0-9]+\s*)")},
		{"serverName", std::regex(R"(\s*serverName\s+(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])\s*)")},
		{"clientMaxBodySize", std::regex(R"(\s*clientMaxBodySize\s+[1-9]+[0-9]*(T|G||M|K|B))")},
		{"error", std::regex(R"(\s*error\s+[4-5][0-9]{2}(?:,[4-5][0-9]{2})*\s+([^,\s]+(?:\.html|\.htm))\s*)")},
		{"cgis",std::regex(patternStr)},
	};

	std::istringstream stream(generalConfig); 
	std::string line;
	while (std::getline(stream, line))
	{
		int errorCaught = 0;
		if (line.empty()) continue;

		if (std::regex_match(line, linePattern))
		{
			for (auto& pattern : patterns)
			{
				if ((errorCaught = matchLinePattern(line, pattern.first, pattern.second)) == 1)
				{
					generalConfigErrorsCount++;
					break;
				}
				else if (pattern.first == "port" && std::regex_match(line, pattern.second))
				{
					int port = std::stoi(Utility::trim(Utility::splitString(line, " ")[1]));
					if (port < 1023 || port > 65535)
					{
						std::cout << "Line not valid: " << TEXT_RED << line << RESET << std::endl;
						generalConfigErrorsCount++;
						errorCaught = 1;
						break;
					}
				}
				else if ((pattern.first == "error" || pattern.first == "cgis") && (errorCaught = checkUnique(line)) == 1)
				{
					generalConfigErrorsCount++;
					break;
				}
			}
			if (errorCaught != 1)
				std::cout << "Line validated: " << TEXT_GREEN << line << RESET<< std::endl;
		}
		else
		{
			std::cout << "Line not valid: " << TEXT_RED << line << RESET << std::endl;
			generalConfigErrorsCount++;
		}
	}
	return generalConfigErrorsCount;
}


/**
 * Validates: path, redirect index, root, methods, uploadPath, directoryListing
*/
int ConfigValidator::validateLocationConfig(std::string locationString)
{
	std::regex linePattern(R"((path|redirect|index|root|methods|uploadPath|directoryListing)\s+[a-zA-Z0-9~\-_./,:$]+\s*)");
	std::map<std::string, std::regex> patterns = {
		{"path", std::regex(R"(\s*path\s+\/([a-zA-Z0-9_\-~.]+\/?)*([a-zA-Z0-9_\-~.]+\.[a-zA-Z0-9_\-~.]+)?\s*)")},
		{"index", std::regex(R"(\s*index\s+([^,\s]+(?:\.html|\.htm))\s*)")},
		{"redirect", std::regex(R"(\s*redirect\s+((\w+:(\/\/[^\/\s]+)?[^\s]*)|(\/([a-zA-Z0-9-_~.]*\/)))\s*)")},
		{"root", std::regex(R"(\s*root\s+\/([a-zA-Z0-9-_~.]*\/)*\s*)")},
		{"uploadPath", std::regex(R"(\s*uploadPath\s+\/([a-zA-Z0-9-_~.]*\/)*\s*)")},
		{"methods", std::regex(R"(\s*methods\s+(get|post|delete)(,(get|post|delete)){0,2}\s*)")},
		{"directoryListing", std::regex(R"(\s*directoryListing\s+(on|off)\s*)")},
	};

	int locationStringErrorsCount = 0;

	std::istringstream stream(locationString); 
	std::string line;
	std::cout << "Let's validate location..." << std::endl;
	while (std::getline(stream, line))
	{
		int errorCaught = 0;
		if (line.empty()) continue;

		if (std::regex_match(line, linePattern))
		{
			for (auto& pattern : patterns)
			{
				if ((errorCaught = matchLinePattern(line, pattern.first, pattern.second)) == 1)
				{
					locationStringErrorsCount++;
					break;
				}
			}
			if (errorCaught != 1)
				std::cout << "Line validated: " << TEXT_GREEN << line << RESET<< std::endl;
		}
		else
		{
			std::cout << "Line not valid: " << TEXT_RED << line << RESET << std::endl;
			locationStringErrorsCount++;
		}
	}
	return locationStringErrorsCount;
}