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
	std::cout << "=== Printing server config ===" << std::endl;
	int index = 0;
	for (ServerConfig server : this->config)
	{
		std::cout << TEXT_UNDERLINE;
		std::cout << "Server #" << index << std::endl;
		std::cout << RESET_UNDERLINE;
		std::cout << "ipAddress: " << server.ipAddress << std::endl;
		std::cout << "port: " << server.port << std::endl;
		std::cout << "serverName: " << server.serverName << std::endl;
		std::cout << "clientMaxBodySize: " << server.clientMaxBodySize << std::endl;
		for (auto error : server.errorPages)
			std::cout << "error: " << error.first[0] << " " << error.second << std::endl;
		for (auto cgi : server.cgi)
			std::cout << "cgi: " << cgi.first << " " << cgi.second << std::endl;
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

	int index = 0;
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
			if (!line.empty())
				std::cout << "Line: " << line << std::endl;

			// Split line into keys and values
			std::vector<std::string> keyValue = Utility::splitString(line, " ");
			// if (keyValue.size() == 2)
			//{
				std::string key = Utility::trim(keyValue[0]);
				std::string value = Utility::trim(keyValue[1]);
				std::cout << "key: " << key << " value: " << value << std::endl;
			// }
			
			if (key == "ipAddress")
				this->config[index].ipAddress = value;
			else if (key == "serverName")
				this->config[index].serverName = value;
			else if (key == "port")
				this->config[index].port = std::stoi(value);
			else if (key == "clientMaxBodySize")
				this->config[index].clientMaxBodySize = value;
			else if (key == "error")
			{
				std::cout << "error: " << value << std::endl;
				std::vector<std::string> errorCodesString = Utility::splitString(value, ",");
				std::vector<int> errorCodes;
				for (std::string code : errorCodesString)
					errorCodes.push_back(std::stoi(code));
				this->config[index].errorPages[errorCodes] = Utility::trim(keyValue[2]);
			}
			else if (key == "cgi")
			{
				std::vector<std::string> cgi = Utility::splitString(value, " ");
				for (std::string ext : cgi)
					this->config[index].cgi[ext] = true;
			}
		}

		// Parsing server locations
		std::vector<std::string> locations(split.begin() + 1, split.end());
		for (std::string location : locations)
			std::cout << "location: " << location << std::endl;

		index++;
	}
}
