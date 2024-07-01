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

	printConfig();
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
			std::cout << "\tindex: " << location.index << std::endl;
			for (auto& method : location.methods)
			{
				if (method.second)
					std::cout << "\tmethod: " << method.first << std::endl;
			}
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
		int configErrorsFound = ConfigValidator::validateGeneralConfig(generalConfig, getServers());
		// Validate locations
		for (std::string& locationString : locationStrings)
		{
			configErrorsFound += ConfigValidator::validateLocationConfig(locationString);
		}

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
				str = Utility::replaceWhiteSpaces(str);
			}
			if (keyValue.size() < 2)
				throw ServerException("Invalid config file format, missing value for key: " + keyValue[0]);
			std::string key = keyValue[0];
			std::string value = keyValue[1];
			
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
					_servers[i].errorPages[std::stoi(code)] = keyValue[2];
				}
			}
			else if (key == "cgis")
			{
				std::vector<std::string> cgis = Utility::splitString(value, ",");
				for (std::string cgi : cgis)
					_servers[i].cgis[cgi] = true;
			}
		}

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
						serverConfig.locations[j].index = value;
				else if (key == "methods")
				{
					std::vector<std::string> methods = Utility::splitString(value, ",");
					for (std::string method : methods)
						serverConfig.locations[j].methods[method] = true;
						// serverConfig.locations[j].methods.push_back(method);
				}
			}
			j++;
		}
}

std::vector<ServerConfig>& Config::getServers()
{
	return _servers;
}
