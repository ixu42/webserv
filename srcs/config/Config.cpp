/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:24 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/07 02:06:36 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config(std::string filePath)
{
	_configString = Utility::readFile(filePath);

	// std::cout << TEXT_YELLOW;
	// std::cout << "=== Config file read === " << std::endl;
	// std::cout << _configString << std::endl;
	// std::cout << RESET;

	parse();

	printConfig();
}

void Config::printConfig()
{
	std::cout << TEXT_YELLOW;
	std::cout << "=== Printing parsed config ===" << std::endl;
	int i = 0;
	for (auto& serversConfigs : _serversConfigsMap) // also should go through the map of configs
	{
		int j = 0;
		std::cout << BG_YELLOW << TEXT_BLACK << TEXT_BOLD << "Server #" << i << " " << serversConfigs.first << RESET << std::endl;
		for (ServerConfig server : serversConfigs.second)
		{
			std::cout << TEXT_BOLD << TEXT_UNDERLINE << TEXT_YELLOW;;
			std::cout << "Named Server #" << j << RESET << std::endl;
			std::cout << TEXT_YELLOW;
			std::cout << "\tipAddress: " << server.ipAddress << std::endl;
			std::cout << "\tport: " << server.port << std::endl;
			std::cout << "\tserverName: " << server.serverName << std::endl;
			std::cout << "\tclientMaxBodySize: " << server.clientMaxBodySize << std::endl;
			for (auto error : server.errorPages)
				std::cout << "\tdefaultError: " << error.first << " " << error.second << std::endl;
			for (auto error : server.errorPages)
				std::cout << "\terror: " << error.first << " " << error.second << std::endl;
			for (auto cgi : server.cgis)
				std::cout << "\tcgi: " << cgi.first << " " << std::boolalpha << cgi.second << std::endl;
			for (auto location : server.locations)
			{
				std::cout << TEXT_UNDERLINE;
				std::cout << "\tLocation: " << location.path << std::endl;
				std::cout << RESET_UNDERLINE;
				std::cout << "\t\tredirect: " << location.redirect << std::endl;
				std::cout << "\t\troot: " << location.root << std::endl;
				std::cout << "\t\tuploadPath: " << location.uploadPath << std::endl;
				std::cout << "\t\tdirectoryListing: " << std::boolalpha << location.directoryListing << std::endl;
				std::cout << "\t\tindex: " << location.index << std::endl;
				for (auto& method : location.methods)
				{
					if (method.second)
						std::cout << "\tmethod: " << method.first << std::endl;
				}
			}
			j++;
		}
		i++;
	}
	std::cout << RESET;
}

std::vector<std::string> Config::filterOutInvalidServerStrings(std::vector<std::string> serverStrings)
{
	// for (std::string server : serverStrings)
	for (size_t i = 0; i < serverStrings.size();)
	{
		std::cout << "Filtering server #" << i << std::endl;
		std::string generalConfig;
		if (serverStrings[i].empty())
		{
			serverStrings.erase(serverStrings.begin() + i);
			continue;
		}
		
		std::vector<std::string> split = Utility::splitString(serverStrings[i], "[location]");

		// Get server general config string
		generalConfig = split[0];
		// std::cout << "generalConfig: " << generalConfig << std::endl;

		// Get server locations string
		std::vector<std::string> locationStrings(split.begin() + 1, split.end());

		// Validate general config
		int configErrorsFound = ConfigValidator::validateGeneralConfig(generalConfig, serverStrings, i);
		// Validate locations
		for (std::string& locationString : locationStrings)
		{
			configErrorsFound += ConfigValidator::validateLocationConfig(locationString);
		}

		if (configErrorsFound != 0)
		{
			// decrease servers vector because config is faulty
			std::cout << "This server config has " << configErrorsFound << " config errors and will be ignored" << std::endl;
			// _servers.resize(_servers.size() - 1);
			serverStrings.erase(serverStrings.begin() + i);
			continue;
		}
		i++;
	}
	return serverStrings;
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
			std::vector<std::string> splitLine = Utility::splitString(line, " ");
			ipAddress = splitLine[1];
		}
		else if (std::regex_match(line, portPattern))
		{
			line = Utility::replaceWhiteSpaces(line, ' ');
			std::vector<std::string> splitLine = Utility::splitString(line, " ");
			port = splitLine[1];
		}
	}
	return ipAddress + ":" + port;
}

void Config::parse()
{
	std::cout << "=== Parsing the config ===" << std::endl;

	/* Check if config has something above the first [server]*/
	_configString = Utility::trim(_configString);
	size_t pos = _configString.find("[server]");

	if (pos == std::string::npos)
		throw ServerException("Invalid config file format, missing [server] section");
	if (pos != 0)
		throw ServerException("Invalid config file format, [server] section should be at the beginning of the file");

	std::vector<std::string> serverStrings = Utility::splitString(_configString, "[server]");

	// Filter out invalid server configs
	serverStrings = filterOutInvalidServerStrings(serverStrings);

	parseServers(serverStrings);

	std::cout << "=== Parsing done ===" << std::endl;
}

void Config::parseServers(std::vector<std::string> serverStrings)
{

	int i = 0;
	for (std::string server : serverStrings)
	{
		std::cout << "Parsing server #" << i << std::endl;
		ServerConfig serverConfig;
		std::string generalConfig;
		
		std::vector<std::string> split = Utility::splitString(server, "[location]");

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
			std::string ipPort = findIpPortKey(generalConfig);

			if (line.empty()) continue;

			// std::cout << "Line: " << line << std::endl;

			// Split line into keys and values
			line = Utility::replaceWhiteSpaces(line, ' ');
			std::vector<std::string> keyValue = Utility::splitString(line, " ");

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
				std::vector<std::string> errorCodesString = Utility::splitString(value, ",");
				for (std::string code : errorCodesString)
				{
					serverConfig.errorPages[std::stoi(code)] = keyValue[2];
				}
			}
			else if (key == "cgis")
			{
				std::vector<std::string> cgis = Utility::splitString(value, ",");
				for (std::string cgi : cgis)
					serverConfig.cgis[cgi] = true;
			}
		}

		parseLocations(serverConfig, locationStrings);
		_serversConfigsMap[findIpPortKey(generalConfig)].push_back(serverConfig);
		i++;
	}
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
				if (line.empty()) continue;

				// std::cout << "Line: " << line << std::endl;

				// Split line into keys and values
				std::vector<std::string> keyValue = Utility::splitString(line, " ");
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
					serverConfig.locations[j].root = value;
				else if (key == "uploadPath")
					serverConfig.locations[j].uploadPath = value;
				else if (key == "directoryListing" && value == "on")
						serverConfig.locations[j].directoryListing = true;
				else if (key == "index")
						serverConfig.locations[j].index = value;
				else if (key == "methods")
				{
					std::vector<std::string> methods = Utility::splitString(value, ",");
					for (std::string method : methods)
						serverConfig.locations[j].methods[method] = true;
				}
			}
			j++;
		}
}

std::map<std::string, std::vector<ServerConfig>>& Config::getServersConfigsMap()
{
	return _serversConfigsMap;
}
