/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:24 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/22 17:21:44 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config(std::string filePath, const char* argv0)
{
	_argv0 = argv0;
	_configString = Utility::readFile(normalizeFilePath(filePath, false));
	_configString = filterOutComments(_configString);

	// std::cout << TEXT_YELLOW;
	// std::cout << "=== Config file read === " << std::endl;
	// std::cout << _configString << std::endl;
	// std::cout << RESET;

	parse();

	printConfig();
}

std::string Config::filterOutComments(std::string configString)
{
	std::stringstream filteredConfig;
	std::istringstream stream(configString);
	std::string line;

	std::regex linePattern(R"(\s*#+.*)");
	while (std::getline(stream, line))
	{
		if (std::regex_match(line, linePattern))
			continue;
		filteredConfig << line << "\n";
	}
	return filteredConfig.str();
}

void Config::printConfig()
{
	LOG_DEBUG(TEXT_YELLOW, "=== Printing parsed config ===", RESET);
	int i = 0;
	for (auto& key : _serversConfigsMapKeys) // also should go through the map of configs
	{
		std::vector<ServerConfig> serversConfigs = _serversConfigsMap[key];
		int j = 0;
		LOG_DEBUG(BG_YELLOW, TEXT_BLACK, TEXT_BOLD, "Server #", i, RESET);
		for (ServerConfig server : serversConfigs)
		{
			LOG_DEBUG(TEXT_BOLD, TEXT_UNDERLINE, TEXT_YELLOW,"Named Server #", j, RESET);
			LOG_DEBUG(TEXT_YELLOW, "\tipAddress: ", server.ipAddress, RESET);
			LOG_DEBUG(TEXT_YELLOW, "\tport: ", server.port, RESET);
			LOG_DEBUG(TEXT_YELLOW, "\tserverName: ", server.serverName, RESET);
			LOG_DEBUG(TEXT_YELLOW, "\tclientMaxBodySize: ", server.clientMaxBodySize, RESET);
			for (auto& error : server.defaultPages)
				LOG_DEBUG(TEXT_YELLOW, "\tdefaultError: ", error.first, " ", error.second, RESET);
			for (auto& error : server.errorPages)
				LOG_DEBUG(TEXT_YELLOW, "\terror: ", error.first, " ", error.second, RESET);
			// for (auto& cgi : server.cgis)
			// 	LOG_DEBUG(TEXT_YELLOW, "\tcgi: ", cgi.first, " ", std::boolalpha, cgi.second, RESET);
			for (auto& [cgi, path] : _cgis)
				LOG_DEBUG(TEXT_YELLOW, "\t" + cgi + ": " + path, RESET);
			for (auto& location : server.locations)
			{
				LOG_DEBUG(TEXT_YELLOW, TEXT_UNDERLINE, "\tLocation: ", location.path, RESET_UNDERLINE, RESET);
				LOG_DEBUG(TEXT_YELLOW, "\t\tredirect: ", location.redirect, RESET);
				LOG_DEBUG(TEXT_YELLOW, "\t\troot: ", location.root, RESET);
				// LOG_DEBUG(TEXT_YELLOW, "\t\tuploadPath: ", location.uploadPath, RESET); // remove, now handled with upload
				LOG_DEBUG(TEXT_YELLOW, "\t\tupload: ", location.upload, RESET);
				LOG_DEBUG(TEXT_YELLOW, "\t\tautoindex: ", std::boolalpha, location.autoindex, RESET);
				LOG_DEBUG(TEXT_YELLOW, "\t\tindex: ", location.index, RESET);
				for (auto& method : location.methods)
				{
					if (method.second)
						LOG_DEBUG(TEXT_YELLOW, "\t\tmethod: ", method.first, RESET);
				}
			}
			j++;
		}
		i++;
	}
}

std::vector<std::string> Config::filterOutInvalidServerStrings(std::vector<std::string> serverStringsVec)
{
	int j = 0;
	for (size_t i = 0; i < serverStringsVec.size();)
	{
		LOG_DEBUG("Filtering server #", i);
		std::string generalConfig;
		if (serverStringsVec[i].empty())
		{
			serverStringsVec.erase(serverStringsVec.begin() + i);
			continue;
		}
		
		std::vector<std::string> split = Utility::splitStr(serverStringsVec[i], "[location]");

		// Get server general config string
		generalConfig = split[0];
		// std::cout << "generalConfig: " << generalConfig << std::endl;

		// Get server locations string
		std::vector<std::string> locationStrings(split.begin() + 1, split.end());

		// Validate general config
		int configErrorsFound = ConfigValidator::validateGeneralConfig(generalConfig, serverStringsVec, i);
		// Validate locations
		for (std::string& locationString : locationStrings)
		{
			configErrorsFound += ConfigValidator::validateLocationConfig(locationString);
		}

		if (configErrorsFound != 0)
		{
			// decrease servers vector because config is faulty
			LOG_WARNING("Server config (server #", j, ") has ", configErrorsFound, " config errors and will be ignored");
			// _servers.resize(_servers.size() - 1);
			serverStringsVec.erase(serverStringsVec.begin() + i);
			j++;
			continue;
		}
		j++;
		i++;
	}
	return serverStringsVec;
}

std::string findIpPortKey(std::string generalConfig)
{
	std::regex ipAddressPattern(R"(\s*ipAddress\s+((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}\s*)");
	std::regex portPattern(R"(\s*port\s+[0-9]+\s*)");
	std::string ipAddress;
	std::string port;

	std::istringstream stream(generalConfig);
	std::string line;
	while (std::getline(stream, line))
	{
		if (std::regex_match(line, ipAddressPattern))
		{
			line = Utility::replaceWhiteSpaces(line, ' ');
			std::vector<std::string> splitLine = Utility::splitStr(line, " ");
			ipAddress = splitLine[1];
		}
		else if (std::regex_match(line, portPattern))
		{
			line = Utility::replaceWhiteSpaces(line, ' ');
			std::vector<std::string> splitLine = Utility::splitStr(line, " ");
			port = splitLine[1];
		}
	}
	return ipAddress + ":" + port;
}

void Config::parse()
{
	LOG_DEBUG("=== Parsing the config ===");

	/* Check if config has something above the first [server] */
	_configString = Utility::trim(_configString);
	size_t pos = _configString.find("[server]");
	std::string mainConfig;
	std::string serversString;

	if (pos == std::string::npos)
		throw ServerException("Invalid config file format, missing [server] section");
	if (pos != 0)
	{
		mainConfig = _configString.substr(0, pos);
		// throw ServerException("Invalid config file format, [server] section should be at the beginning of the file");
	}
	serversString = _configString.substr(pos);

	parseMainConfig(mainConfig);

	std::vector<std::string> serverStringsVec = Utility::splitStr(serversString, "[server]");
	/* Filter out invalid server configs */
	serverStringsVec = filterOutInvalidServerStrings(serverStringsVec);
	parseServers(serverStringsVec);

	LOG_INFO("Config file parsed");
}

void Config::parseMainConfig(std::string mainConfig)
{
	if (!ConfigValidator::validateMainConfig(mainConfig))
	{
		std::istringstream stream(mainConfig);
		std::string line;
		std::getline(stream, line); // skip the first line [main]
		while (std::getline(stream, line))
		{
			line = Utility::replaceWhiteSpaces(line, ' ');
			line = Utility::trim(line);
			if (line.empty()) continue;
			std::vector<std::string> lineSplit = Utility::splitStr(line, " ");
			if (lineSplit.size() == 2)
				_cgis[lineSplit[0]] = normalizeFilePath(lineSplit[1], false);
		}
	}
}

void Config::parseServers(std::vector<std::string> serverStringsVec)
{

	int i = 0;
	for (std::string server : serverStringsVec)
	{
		LOG_DEBUG("Parsing server #", i);
		ServerConfig serverConfig;
		std::string generalConfig;
		
		std::vector<std::string> split = Utility::splitStr(server, "[location]");

		// Get server general config string
		generalConfig = split[0];
		// std::cout << "generalConfig: " << generalConfig << std::endl;

		// Get  server locations string
		std::vector<std::string> locationStrings(split.begin() + 1, split.end());
		// Reading line by line
		std::istringstream stream(generalConfig);
		std::string line;
		while (std::getline(stream, line))
		{	
			line = Utility::trim(line);
			if (line.empty()) continue;
			// std::cout << "Line: " << line << std::endl;

			// Split line into keys and values
			line = Utility::replaceWhiteSpaces(line, ' ');
			std::vector<std::string> keyValue = Utility::splitStr(line, " ");

			if (keyValue.size() < 2)
				throw ServerException("Invalid config file format, missing value for key: " + keyValue[0]);
			std::string key = keyValue[0];
			std::string value = keyValue[1];
			
			if (key == "ipAddress")
				serverConfig.ipAddress = value;
			else if (key == "serverName")
				serverConfig.serverName = value;
			else if (key == "port")
			{
				serverConfig.port = std::stoi(value);
			}
			else if (key == "clientMaxBodySize")
				serverConfig.clientMaxBodySize = value;
			else if (key == "error")
			{
				// std::cout << "error: " << value << std::endl;
				std::vector<std::string> errorCodesString = Utility::splitStr(value, ",");
				for (std::string code : errorCodesString)
				{
					serverConfig.errorPages[std::stoi(code)] = normalizeFilePath(keyValue[2], false);
				}
			}
		}
		for (auto& page : serverConfig.defaultPages)
		{
			page.second = normalizeFilePath(page.second, false);
		}
		parseLocations(serverConfig, locationStrings);

		std::string ipPort = findIpPortKey(generalConfig);
		serverConfig.cgis = &_cgis; // assign cgi map to each server config
		_serversConfigsMap[ipPort].push_back(serverConfig);

		if (_serversConfigsMap[ipPort].size() == 1) // only unique keys will be saved
			_serversConfigsMapKeys.push_back(ipPort);

		i++;
	}
	LOG_DEBUG("=== Server in map ===");
	for (auto& key : _serversConfigsMapKeys)
	{
		auto& serverConfigs = _serversConfigsMap[key];
		LOG_DEBUG("Server: ", key);
		for (ServerConfig serverConfig : serverConfigs)
		{
			LOG_DEBUG("ServerName: ", serverConfig.serverName);
		}
	}
}


fs::path Config::getExecutablePath()
{
	fs::path executablePath = fs::current_path() / _argv0;
		
	if (fs::exists(executablePath)) {
		return fs::canonical(executablePath).parent_path();
	}

	// Handle the case where argv0 is an absolute path or relative path
	executablePath = fs::canonical(_argv0);
	return executablePath.parent_path();
}

std::string Config::normalizeFilePath(std::string filePathStr, bool closePath)
{
	fs::path filePath(filePathStr);
	fs::path executableDir = getExecutablePath();
	fs::path normalizedfilePath = filePath.is_absolute() ? filePath : executableDir / filePath;
	normalizedfilePath = fs::canonical(normalizedfilePath);

	return closePath ? normalizedfilePath.string() + "/" : normalizedfilePath.string();
}

void Config::parseLocations(ServerConfig& serverConfig, std::vector<std::string> locationStrings)
{
		serverConfig.locations.resize(locationStrings.size());
		int j = 0;
		for (std::string location : locationStrings)
		{
			// std::cout << "location: " << location << std::endl;
			
			std::istringstream stream(location);
			std::string line;
			while (std::getline(stream, line))
			{	
				line = Utility::trim(line);
				if (line.empty()) continue;

				// std::cout << "Line: " << line << std::endl;

				// Split line into keys and values
				std::vector<std::string> keyValue = Utility::splitStr(line, " ");
				std::string key = Utility::trim(keyValue[0]);
				std::string value = Utility::trim(keyValue[1]);

				if (key == "path")
				{
					// std::cout << "path: " << value << std::endl;
					serverConfig.locations[j].path = value;
				}
				else if (key == "redirect")
					serverConfig.locations[j].redirect = value;
				else if (key == "root")
					serverConfig.locations[j].root = normalizeFilePath(value, true); // normalize root to absolute
				else if (key == "upload" && value == "on")
					serverConfig.locations[j].upload = true;
				else if (key == "autoindex" && value == "on")
						serverConfig.locations[j].autoindex = true;
				else if (key == "index")
						serverConfig.locations[j].index = value;
				else if (key == "methods")
				{
					for (auto& methodPair : serverConfig.locations[j].methods)
						methodPair.second = false;
					std::vector<std::string> methods = Utility::splitStr(value, ",");
					for (std::string method : methods)
						serverConfig.locations[j].methods[method] = true;
				}
			}
			serverConfig.locations[j].defaultListingTemplate = normalizeFilePath(serverConfig.locations[j].defaultListingTemplate, false);
			j++;
		}
}

std::map<std::string, std::vector<ServerConfig>>& Config::getServersConfigsMap()
{
	return _serversConfigsMap;
}

std::list<std::string>& Config::getServersConfigsMapKeys()
{
	return _serversConfigsMapKeys;
}