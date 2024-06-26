#include "Config.hpp"

Config::Config(std::string filePath)
{
	this->_filePath = filePath;
	this->configString = Utility::readFile(this->_filePath);

	std::cout << TEXT_YELLOW;
	std::cout << "=== Config file read === " << std::endl;
	std::cout << this->configString << std::endl;
	std::cout << RESET;

	parse();

	printConfig();
}

void Config::printConfig()
{
	std::cout << TEXT_YELLOW;
	std::cout << "=== Printing parsed config ===" << std::endl;
	int i = 0;
	for (ServerConfig server : this->servers)
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
	std::vector<std::string> serverStrings = Utility::splitString(this->configString, "[server]");
	this->servers.resize(serverStrings.size());

	parseServers(serverStrings);

	std::cout << "=== Parsing done ===" << std::endl;
}

void Config::parseServers(std::vector<std::string> serverStrings)
{
	int i = 0;
	for (std::string server : serverStrings)
	{
		ServerConfig serverConfig;
		std::string generalConfig;
		
		std::vector<std::string> split = Utility::splitString(server, "[location]");

		// Parsing server general config
		generalConfig = split[0];
		std::cout << "generalConfig: " << generalConfig << std::endl;

		// Reading line by line
		std::istringstream stream(generalConfig);
		std::string line;
		while (std::getline(stream, line))
		{	
			if (line.empty()) continue;

			std::cout << "Line: " << line << std::endl;

			// Split line into keys and values
			std::vector<std::string> keyValue = Utility::splitString(line, " ");

			// Trime whitespace from both ends of a string
			for (std::string str : keyValue)
			{
				str = Utility::trim(str);
			}

			std::string key = Utility::trim(keyValue[0]);
			std::string value = Utility::trim(keyValue[1]);
			
			if (key == "ipAddress")
				this->servers[i].ipAddress = value;
			else if (key == "serverName")
				this->servers[i].serverName = value;
			else if (key == "port")
				this->servers[i].port = std::stoi(value);
			else if (key == "clientMaxBodySize")
				this->servers[i].clientMaxBodySize = value;
			else if (key == "error")
			{
				std::cout << "error: " << value << std::endl;
				std::vector<std::string> errorCodesString = Utility::splitString(value, ",");
				for (std::string code : errorCodesString)
				{
					this->servers[i].errorPages[std::stoi(code)] = Utility::trim(keyValue[2]);
				}
			}
			else if (key == "cgis")
			{
				std::vector<std::string> cgis = Utility::splitString(value, ",");
				for (std::string cgi : cgis)
					this->servers[i].cgis[cgi] = true;
			}
		}

		// Parsing server locations
		std::vector<std::string> locationStrings(split.begin() + 1, split.end());
		this->servers[i].locations.resize(locationStrings.size());
		parseLocations(this->servers[i], locationStrings);

		i++;
	}
}

void Config::parseLocations(ServerConfig& serverConfig, std::vector<std::string> locationStrings)
{
		serverConfig.locations.resize(locationStrings.size());
		int j = 0;
		for (std::string location : locationStrings)
		{
			std::cout << "location: " << location << std::endl;
			
			std::istringstream stream(location);
			std::string line;
			while (std::getline(stream, line))
			{	
				if (line.empty()) continue;

				std::cout << "Line: " << line << std::endl;

				// Split line into keys and values
				std::vector<std::string> keyValue = Utility::splitString(line, " ");
				std::string key = Utility::trim(keyValue[0]);
				std::string value = Utility::trim(keyValue[1]);

				if (key == "path")
				{
					std::cout << "path: " << value << std::endl;
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
	return this->servers;
}
