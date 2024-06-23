#include "Config.hpp"

Config::Config(std::string filePath)
{
	this->filePath = filePath;
	readFile();
	std::cout << TEXT_YELLOW;
	std::cout << "=== Config file read === " << std::endl;
	std::cout << this->configString << std::endl;
	std::cout << RESET;

	parse();

	printConfig();
}

void Config::readFile()
{
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
		this->configString.append(buffer, bytesRead);
	}

	// std::cout << this->configString << std::endl;

	close(fd);
}

void Config::printConfig()
{
	std::cout << TEXT_YELLOW;
	std::cout << "=== Printing config ===" << std::endl;
	int index = 0;
	for (ServerConfig server : this->config)
	{
		std::cout << BG_YELLOW << TEXT_BLACK << TEXT_BOLD;
		std::cout << "Server #" << index << RESET << std::endl;
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
			for (std::string index : location.index)
				std::cout << "\tindex: " << index << std::endl;
			for (std::string method : location.methods)
				std::cout << "\tmethod: " << method << std::endl;
		}
		index++;
	}
	std::cout << RESET;
}

void Config::parse()
{
	std::cout << "=== Parsing the config ===" << std::endl;
	std::vector<std::string> servers;
	servers = Utility::splitString(this->configString, "[server]");
	this->config.resize(servers.size());
	// std::cout << "server: " << servers[0] << std::endl;
	// std::cout << "server: " << servers[1] << std::endl;

	int i = 0;
	for (std::string server : servers)
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
				this->config[i].ipAddress = value;
			else if (key == "serverName")
				this->config[i].serverName = value;
			else if (key == "port")
				this->config[i].port = std::stoi(value);
			else if (key == "clientMaxBodySize")
				this->config[i].clientMaxBodySize = value;
			else if (key == "error")
			{
				std::cout << "error: " << value << std::endl;
				std::vector<std::string> errorCodesString = Utility::splitString(value, ",");
				for (std::string code : errorCodesString)
				{
					this->config[i].errorPages[std::stoi(code)] = Utility::trim(keyValue[2]);
				}
			}
			else if (key == "cgis")
			{
				std::vector<std::string> cgis = Utility::splitString(value, ",");
				for (std::string cgi : cgis)
					this->config[i].cgis[cgi] = true;
			}
		}

		// Parsing server locations
		std::vector<std::string> locations(split.begin() + 1, split.end());
		this->config[i].locations.resize(locations.size());
		int j = 0;
		for (std::string location : locations)
		{
			std::cout << "location: " << location << std::endl;
			
			stream.clear();
			stream.str(location);
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
					this->config[i].locations[j].path = value;
				}
				else if (key == "redirect")
					this->config[i].locations[j].redirect = value;
				else if (key == "root")
					this->config[i].locations[j].root = value;
				else if (key == "uploadPath")
					this->config[i].locations[j].uploadPath = value;
				else if (key == "directoryListing" && value == "on")
						this->config[i].locations[j].directoryListing = true;
				else if (key == "index")
				{
					std::vector<std::string> indexes = Utility::splitString(value, ",");
					for (std::string index : indexes)
						this->config[i].locations[j].index.push_back(index);
				}
				else if (key == "methods")
				{
					std::vector<std::string> methods = Utility::splitString(value, ",");
					for (std::string method : methods)
						this->config[i].locations[j].methods.push_back(method);
				}
			}
			j++;
		}

		i++;
	}

	std::cout << "=== Parsing done ===" << std::endl;
}

// void Config::parseLocations(ServerConfig& serverConfig, std::vector<std::string> locations)
// {
// 		serverCOnfig.locations.resize(locations.size());
// 		int j = 0;
// 		for (std::string location : locations)
// 		{
// 			std::cout << "location: " << location << std::endl;
			
// 			std::istringstream stream(location);
// 			std::string line;
// 			while (std::getline(stream, line))
// 			{	
// 				if (line.empty()) continue;

// 				std::cout << "Line: " << line << std::endl;

// 				// Split line into keys and values
// 				std::vector<std::string> keyValue = Utility::splitString(line, " ");
// 				std::string key = Utility::trim(keyValue[0]);
// 				std::string value = Utility::trim(keyValue[1]);

// 				if (key == "path")
// 				{
// 					std::cout << "path: " << value << std::endl;
// 					serverCOnfig.locations[j].path = value;
// 				}
// 				else if (key == "redirect")
// 					serverCOnfig.locations[j].redirect = value;
// 				else if (key == "root")
// 					serverCOnfig.locations[j].root = value;
// 				else if (key == "uploadPath")
// 					serverCOnfig.locations[j].uploadPath = value;
// 				else if (key == "directoryListing" && value == "on")
// 						serverCOnfig.locations[j].directoryListing = true;
// 				else if (key == "index")
// 				{
// 					std::vector<std::string> indexes = Utility::splitString(value, ",");
// 					for (std::string index : indexes)
// 						serverCOnfig.locations[j].index.push_back(index);
// 				}
// 				else if (key == "methods")
// 				{
// 					std::vector<std::string> methods = Utility::splitString(value, ",");
// 					for (std::string method : methods)
// 						serverCOnfig.locations[j].methods.push_back(method);
// 				}
// 			}
// 			j++;
// 		}
// }