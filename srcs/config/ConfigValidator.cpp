/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:11 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/23 18:29:02 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigValidator.hpp"

// Check if the serverName per ip:port is unique
int ConfigValidator::validateServerNamePerIpPort(std::vector<std::string> serverStrings, size_t i, std::map<std::string, std::regex> patterns)
{
	std::string ipAddress = "";
	std::string port = "";
	std::string serverName = "";

	std::istringstream stream1(serverStrings[i]); 
	std::string line1;
	while (std::getline(stream1, line1))
	{
		line1 = Utility::replaceWhiteSpaces(line1, ' ');
		if (ipAddress.empty() && std::regex_match(line1, patterns["ipAddress"]))
			ipAddress = Utility::splitStr(line1, " ")[1];
		else if (port.empty() && std::regex_match(line1, patterns["port"]))
			port = Utility::splitStr(line1, " ")[1];
		else if (serverName.empty() && std::regex_match(line1, patterns["serverName"]))
			serverName = Utility::splitStr(line1, " ")[1];
	}

	for (size_t j = 0; j < i; j++)
	{
		std::string currIpAddress = "";
		std::string currPort = "";
		std::string currServerName = "";

		stream1.clear();
		stream1.str(serverStrings[j]);
		while (std::getline(stream1, line1))
		{
			line1 = Utility::replaceWhiteSpaces(line1, ' ');
			if (currIpAddress.empty() && std::regex_match(line1, patterns["ipAddress"]))
				currIpAddress = Utility::splitStr(line1, " ")[1];
			else if (currPort.empty() && std::regex_match(line1, patterns["port"]))
				currPort = Utility::splitStr(line1, " ")[1];
			else if (currServerName.empty() && std::regex_match(line1, patterns["serverName"]))
				currServerName = Utility::splitStr(line1, " ")[1];
		}
		if (serverName == currServerName && port == currPort && ipAddress == currIpAddress)
		{
			LOG_DEBUG("Config not valid: ", TEXT_RED, "Server name ", serverName, " is not unique for ", currIpAddress, ":", currPort, RESET);
			return 1;
		}
	}
	return 0;
}

/**
 * Checks second parameter with comma separation for unique values.
 * For example, cgis, error fields
 */
int ConfigValidator::checkUnique(std::string line)
{
	std::vector<std::string> lineSplit = Utility::splitStr(Utility::replaceWhiteSpaces(line, ' '), " ");
	std::vector<std::string> errorCodesStrings = Utility::splitStr(lineSplit[1], ",");
	std::set<std::string> uniqueErrorCodesStrings(errorCodesStrings.begin(), errorCodesStrings.end());
	if (errorCodesStrings.size() != uniqueErrorCodesStrings.size())
	{
		LOG_DEBUG("Line not valid: ", TEXT_RED, line, RESET);
		return 1;
	}
	return 0;
}

/**
 * pattern1 matches the line and pattern2 is to check if line is valid
 * Returns 1 if line is not valid
*/
int ConfigValidator::matchLinePattern(std::string& line, std::string field, std::regex pattern2)
{
	field = "^\\s*" + field + "\\b.*";
	std::regex pattern1(field);
	if (std::regex_match(line, pattern1) && !std::regex_match(line, pattern2))
	{
		LOG_DEBUG("Line not valid: ", TEXT_RED, line, RESET);
		return 1;
	}
	return 0;
}

int ConfigValidator::countMatchInRegex(std::string str, std::regex pattern)
{
	auto words_begin = std::sregex_iterator(
		str.begin(), str.end(), pattern);
	auto words_end = std::sregex_iterator();

	return std::distance(words_begin, words_end);
}
int ConfigValidator::validateMandatoryFields(std::string str,
											std::vector<std::string> mandatoryFields,
											std::map<std::string, std::regex> patterns)
{
	int stringErrorsCount = 0;

	for (std::string& field : mandatoryFields)
	{
		if (countMatchInRegex(str, patterns[field]) == 0)
		{
			LOG_DEBUG("Line not valid: ", TEXT_RED, "Location should have at least 1 ", field, RESET);
			stringErrorsCount++;
		}
	}
	return stringErrorsCount;
}

int ConfigValidator::validateMainConfig(std::string mainConfig)
{
	std::regex cgiPattern(R"(\s*[a-z]+\s+(\.\.\/|\/)*([a-zA-Z0-9-_~.]+(\/[a-zA-Z0-9-_~.]+))*\s*)");
	int errorsCount = 0;
	int cgisCount = 0;
	int lineCount = 0;
	mainConfig = Utility::trim(mainConfig);
	std::istringstream stream(mainConfig);
	std::string line;
	while (std::getline(stream, line))
	{
		line = Utility::trim(line);
		if (line.empty()) continue;

		if (lineCount == 0)
		{
			if (line == "[main]")
			{
				lineCount++;
				continue;
			}
			else 
			{
				errorsCount++;
				LOG_DEBUG("Config error: ", TEXT_RED, "[main] is missing in the first line", RESET);
				lineCount++;
			}
		}
		if (!std::regex_match(line, cgiPattern))
		{
			errorsCount++;
			LOG_DEBUG("Line not valid: ", TEXT_RED, line, RESET);
			continue ;
		}
		LOG_DEBUG("Line validated: ", TEXT_GREEN, line, RESET);
		cgisCount++;
	}
	if (errorsCount != 0 || cgisCount == 0)
	{
		LOG_WARNING("Main config is invalid and will be ignored");
		return errorsCount;
	}
	return errorsCount;
}

/**
 * Returns number of invalid lines
 */
int ConfigValidator::validateGeneralConfig(std::string generalConfig, std::vector<std::string> serverStrings, size_t i)
{
	int generalConfigErrorsCount = 0;

	std::regex linePattern(R"(\s*(ipAddress|port|serverName|clientMaxBodySize|error|cgis|)\s+[a-zA-Z0-9~\-_.,]+\s*[a-zA-Z0-9~\-_.,\/"' ]*\s*)");
	std::map<std::string, std::regex> patterns = {
		{"ipAddress", std::regex(R"(\s*ipAddress\s+((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}\s*)")},
		{"port", std::regex(R"(\s*port\s+[0-9]+\s*)")},
		{"serverName", std::regex(R"(\s*serverName\s+(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])\s*)")},
		{"clientMaxBodySize", std::regex(R"(\s*clientMaxBodySize\s+[1-9]+[0-9]*(G|M|K|B))")},
		{"error", std::regex(R"(\s*error\s+[4-5][0-9]{2}(?:,[4-5][0-9]{2})*\s+((["'])*[^,]+(?:\.html|\.htm)(\2)*)\s*)")}
	};

	std::vector<std::string> oneAllowed = {"ipAddress", "port", "serverName", "clientMaxBodySize"};
	std::vector<std::string> mandatoryFields = {"port"};


	generalConfigErrorsCount += validateMandatoryFields(generalConfig, mandatoryFields, patterns);
	generalConfigErrorsCount += validateServerNamePerIpPort(serverStrings, i, patterns);

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
				// Check for repeating only one time allowed fields
				auto it = std::find(oneAllowed.begin(), oneAllowed.end(), pattern.first);
				// std::cout << "count matches for " << pattern.first << ": " << countMatchInRegex(generalConfig, patterns[pattern.first]) << std::endl;
				if (it != oneAllowed.end() && countMatchInRegex(generalConfig, patterns[pattern.first]) > 1)
				{
					// std::cout << "Match found for repeating field" << std::endl;
					LOG_DEBUG("Line not valid: ", TEXT_RED, line, RESET);
					errorCaught = 1;
					generalConfigErrorsCount++;
					break;
				}

				if ((errorCaught = matchLinePattern(line, pattern.first, pattern.second)) == 1)
				{
					generalConfigErrorsCount++;
					break;
				}
				else if (pattern.first == "port" && std::regex_match(line, pattern.second))
				{
					int port = std::stoi(Utility::trim(Utility::splitStr(line, " ")[1]));
					if (port < 1 || port > 65535)
					{
						LOG_DEBUG("Line not valid: ", TEXT_RED, line, RESET);
						generalConfigErrorsCount++;
						errorCaught = 1;
						break;
					}
				}
				else if (pattern.first == "error" && (errorCaught = checkUnique(line)) == 1)
				{
					generalConfigErrorsCount++;
					break;
				}
			}
			if (errorCaught != 1)
				LOG_DEBUG("Line validated: ", TEXT_GREEN, line, RESET);
		}
		else
		{
			LOG_DEBUG("Line not valid: ", TEXT_RED, line, RESET);
			generalConfigErrorsCount++;
		}
	}
	return generalConfigErrorsCount;
}


/**
 * Validates: path, redirect index, root, methods, uploadPath, autoindex
*/
int ConfigValidator::validateLocationConfig(std::string locationString)
{
	// linePattern is more broad and should have characters from more specific cases
	std::regex linePattern(R"(\s*(path|redirect|index|root|methods|upload|autoindex)\s+[a-zA-Z0-9~\-_./,:$"' ]+\s*)");
	std::map<std::string, std::regex> patterns = {
		{"path", std::regex(R"(\s*path\s+\/([a-zA-Z0-9_\-~.]+\/?)*([a-zA-Z0-9_\-~.]+\.[a-zA-Z0-9_\-~.]+)?\s*)")},
		{"index", std::regex(R"(\s*index\s+([^,\s]+(?:\.html|\.htm))\s*)")},
		{"redirect", std::regex(R"(\s*redirect\s+((\w+:(\/\/[^\/\s]+)?[^\s]*)|(\/([a-zA-Z0-9-_~.]*\/)))\s*)")},
		// {"root", std::regex(R"(\s*root\s+(\.\.\/|\/)*([a-zA-Z0-9-_~. ]+\/)+\s*)")},
		// {"root", std::regex(R"(\s*root\s+(?:(\.\.\/|\/)*([a-zA-Z0-9-_~. ]*\/)*\s*|(['"])((?:\.\.\/|\/)*([a-zA-Z0-9-_~. ]*\/)*\3)\s*))")},

		{"root", std::regex(R"(\s*root\s+(['"]*)((?:\.\.\/|\/)*([a-zA-Z0-9-_~. ]+\/)+\1)\s*)")},

		{"upload", std::regex(R"(\s*upload\s+(on|off)\s*)")},
		{"methods", std::regex(R"(\s*methods\s+(get|post|delete)(,(get|post|delete)){0,2}\s*)")},
		{"autoindex", std::regex(R"(\s*autoindex\s+(on|off)\s*)")},
	};

	int locationStringErrorsCount = 0;
	std::vector<std::string> mandatoryFields = {"path"};


	LOG_DEBUG("Let's validate location...");
	validateMandatoryFields(locationString, mandatoryFields, patterns);

	std::istringstream stream(locationString); 
	std::string line;
	
	while (std::getline(stream, line))
	{
		int errorCaught = 0;
		line = Utility::trim(line);
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
				LOG_DEBUG("Line validated: ", TEXT_GREEN, line, RESET);
		}
		else
		{
			LOG_DEBUG("Line not valid: ", TEXT_RED, line, RESET);
			locationStringErrorsCount++;
		}
	}
	return locationStringErrorsCount;
}
