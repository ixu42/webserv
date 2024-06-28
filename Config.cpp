#include "Config.hpp"

Config::Config(std::string filePath)
{
	_configString = Utility::readFile(filePath);
	// validate();

	// std::cout << TEXT_YELLOW;
	// std::cout << "=== Config file read === " << std::endl;
	// std::cout << _configString << std::endl;
	// std::cout << RESET;

	parse();

	// printConfig();
}

void Config::printConfig()
{
	std::cout << TEXT_YELLOW;
	std::cout << "=== Printing parsed config ===" << std::endl;
	int i = 0;
	for (ServerConfig server : _servers)
	{
		std::cout << BG_YELLOW << TEXT_BLACK << TEXT_BOLD;
		std::cout << "Server #" << i << RESET << std::endl;
		std::cout << TEXT_YELLOW;
		std::cout << "ipAddress: " << server.ipAddress << std::endl;
		std::cout << "port: " << server.port << std::endl;
		std::cout << "serverName: " << server.serverName << std::endl;
		std::cout << "clientMaxBodySize: " << server.clientMaxBodySize << std::endl;
		for (auto error : server.errorPages)
			std::cout << "error: " << error.first << " " << error.second << std::endl;
		for (auto cgi : server.cgis)
			std::cout << "cgi: " << cgi.first << " " << std::boolalpha << cgi.second << std::endl;
		for (auto location : server.locations)
		{
			std::cout << TEXT_UNDERLINE;
			std::cout << "\tLocation: " << location.path << std::endl;
			std::cout << RESET_UNDERLINE;
			std::cout << "\tredirect: " << location.redirect << std::endl;
			std::cout << "\troot: " << location.root << std::endl;
			std::cout << "\tuploadPath: " << location.uploadPath << std::endl;
			std::cout << "\tdirectoryListing: " << std::boolalpha << location.directoryListing << std::endl;
			for (std::string ind : location.index)
				std::cout << "\tind: " << ind << std::endl;
			for (std::string method : location.methods)
				std::cout << "\tmethod: " << method << std::endl;
		}
		i++;
	}
	std::cout << RESET;
}

void Config::parse()
{
	std::cout << "=== Parsing the config ===" << std::endl;
	std::vector<std::string> serverStrings = Utility::splitString(_configString, "[server]");
	_servers.resize(serverStrings.size());

	parseServers(serverStrings);

	std::cout << "=== Parsing done ===" << std::endl;
}

// void Config::validate()
// {
// 	std::istringstream stream(_configString);
// 	std::string line;

// 	while (std::getline(stream, line))
// 	{
// 		if (line.empty()) continue;
// 		std::cout << "Validate line " << line << std::endl;
// 	}
// }

/**
 * Returns number of invalid lines
 */
int Config::validateGeneralConfig(std::string generalConfig)
{
	int generalConfigErrorsCount = 0;

	std::istringstream stream(generalConfig); 
	std::string line;

	// std::regex linePattern(R"([a-zA-Z0-9]+\s+[a-zA-Z0-9,.]+\s*[a-zA-Z0-9,.]*\s*)");
	std::regex linePattern(R"((ipAddress|port|serverName|clientMaxBodySize|error|cgis)\s+[a-zA-Z0-9,.]+\s*[a-zA-Z0-9,.]*\s*)");
	std::regex portPattern(R"(\s*port\s+[0-9]+\s*)");
	std::regex ipAddressPattern(R"(\s*ipAddress\s+((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4})");
	// RFCC 1123 Standard
	std::regex serverNamePattern(R"(\s*serverName\s+(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9]))");
	std::regex clientMaxBodySizePattern(R"(\s*clientMaxBodySize\s+[1-9]+[0-9]*(T|G||M|K|B))");
	std::regex errorPattern(R"(\s*error\s+[1-5][0-9]{2}(?:,[1-5][0-9]{2})*\s+([^,\s]+(?:\.html|\.htm))\s*)");

	// Cgi pattern is constructed from cgis map default keys
	std::string cgisString;
	size_t i = 0;
	for (auto& cgi : getServers()[0].cgis)
	{
		cgisString += cgi.first;
		if (i != getServers()[0].cgis.size() - 1)
			cgisString += "|";
		i++;
	}
    std::string patternStr = "\\s*cgis\\s+\\b(" + cgisString + ")(?:,(" + cgisString + "))?\\b";
    std::regex cgisPattern(patternStr);

	// std::regex cgisPattern(R"(\s*cgis\s+\b(" + cgisString + ")(?:,(" + cgisString + "))?\b)");
	// std::regex cgisPattern(R"(\s*cgis\s+\b(php|py)(?:,(php|py))?\b)");

	while (std::getline(stream, line))
	{
		if (line.empty()) continue;

		// line = Utility::trim(line);
		// std::cout << "Validating the line... " << line << std::endl;
		if (std::regex_match(line, linePattern))
		{
			// Validate address
			if (std::regex_match(line, std::regex(R"(^\s*ipAddress\b.*)")) && !std::regex_match(line, ipAddressPattern))
			{
				std::cout << "Line not valid: " << TEXT_RED << line << RESET << std::endl;
				generalConfigErrorsCount++;
				continue;
			}
			// Validate port
			if (std::regex_match(line, std::regex(R"(^\s*port\b.*)")) && !std::regex_match(line, portPattern))
			{
				std::cout << "Line not valid: " << TEXT_RED << line << RESET << std::endl;
				generalConfigErrorsCount++;
				continue;
			}
			else if (std::regex_match(line, portPattern))
			{
				int port = std::stoi(Utility::trim(Utility::splitString(line, " ")[1]));
				if (port < 1023 || port > 65535)
				{
					std::cout << "Line not valid: " << TEXT_RED << line << RESET << std::endl;
					// std::cout << "Invalid port number: " + std::to_string(port) << std::endl;
					// throw ServerException("Invalid port number: " + std::to_string(port));
					generalConfigErrorsCount++;
					continue;
				}
			}
			// Validate server name
			if (std::regex_match(line, std::regex(R"(^\s*serverName\b.*)")) && !std::regex_match(line, serverNamePattern))
			{
				std::cout << "Line not valid: " << TEXT_RED << line << RESET << std::endl;
				generalConfigErrorsCount++;
				continue;
			}
			// Validate client max body size
			if (std::regex_match(line, std::regex(R"(^\s*clientMaxBodySize\b.*)")) && !std::regex_match(line, clientMaxBodySizePattern))
			{
				std::cout << "Line not valid: " << TEXT_RED << line << RESET << std::endl;
				generalConfigErrorsCount++;
				continue;
			}
			// Validate error
			if (std::regex_match(line, std::regex(R"(^\s*error\b.*)")) && !std::regex_match(line, errorPattern))
			{
				std::cout << "Line not valid: " << TEXT_RED << line << RESET << std::endl;
				generalConfigErrorsCount++;
				continue;
			}
			// Validate cgis
			if (std::regex_match(line, std::regex(R"(^\s*cgis\b.*)")) && !std::regex_match(line, cgisPattern))
			{
				std::cout << "Line not valid: " << TEXT_RED << line << RESET << std::endl;
				generalConfigErrorsCount++;
				continue;
			}
			// Validate 
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

		// Validate general config
		int configErrorsFound = validateGeneralConfig(generalConfig);

		if (configErrorsFound != 0)
		{
			// decrease servers vector because config is faulty
			std::cout << "This server config has " << configErrorsFound << " invalid lines and will be ignored" << std::endl;
			_servers.resize(_servers.size() - 1);
			continue;
		}
		
		// Reading line by line
		std::istringstream stream(generalConfig);
		std::string line;
		while (std::getline(stream, line))
		{	
			if (line.empty()) continue;

			// std::cout << "Line: " << line << std::endl;

			// Split line into keys and values
			std::vector<std::string> keyValue = Utility::splitString(line, " ");

			// Trime whitespace from both ends of a string
			for (std::string str : keyValue)
			{
				str = Utility::trim(str);
			}
			if (keyValue.size() < 2)
				throw ServerException("Invalid config file format, missing value for key: " + keyValue[0]);
			std::string key = Utility::trim(keyValue[0]);
			std::string value = Utility::trim(keyValue[1]);
			
			if (key == "ipAddress")
				_servers[i].ipAddress = value;
			else if (key == "serverName")
				_servers[i].serverName = value;
			else if (key == "port")
			{
				_servers[i].port = std::stoi(value);
			}
			else if (key == "clientMaxBodySize")
				_servers[i].clientMaxBodySize = value;
			else if (key == "error")
			{
				// std::cout << "error: " << value << std::endl;
				std::vector<std::string> errorCodesString = Utility::splitString(value, ",");
				for (std::string code : errorCodesString)
				{
					_servers[i].errorPages[std::stoi(code)] = Utility::trim(keyValue[2]);
				}
			}
			else if (key == "cgis")
			{
				std::vector<std::string> cgis = Utility::splitString(value, ",");
				for (std::string cgi : cgis)
					_servers[i].cgis[cgi] = true;
			}
		}

		// _servers[i].locations.resize(locationStrings.size());
		parseLocations(_servers[i], locationStrings);

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
				{
					std::vector<std::string> indexes = Utility::splitString(value, ",");
					for (std::string index : indexes)
						serverConfig.locations[j].index.push_back(index);
				}
				else if (key == "methods")
				{
					std::vector<std::string> methods = Utility::splitString(value, ",");
					for (std::string method : methods)
						serverConfig.locations[j].methods.push_back(method);
				}
			}
			j++;
		}
}

std::vector<ServerConfig>& Config::getServers()
{
	return _servers;
}
