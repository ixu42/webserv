#pragma once

#include "ServerException.hpp"
#include "Colors.hpp"
#include "Utility.hpp"

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <string>

#include <fcntl.h>
#include <unistd.h>
// #include <string.h>
#include <cstring>

#include <sstream>

struct Location
{
	std::string					path;
	std::string					redirect;
	std::string					root;
	std::string					uploadPath;
	bool						directoryListing = false;
	std::vector<std::string>	index = {"index.html"};
	std::vector<std::string>	methods;
};

struct ServerConfig
{

		std::string					ipAddress; // = "127.0.0.1";
		int							port; // = 8080;
		std::string					serverName; // = "localhost";
		std::string					clientMaxBodySize = "100M";

		std::map<int, std::string>	errorPages = {{404, "default/404.html"}, {500, "default/500.html"}};
		std::map<std::string, bool>	cgis = {{"php", false}, {"py", false}};

		std::vector<Location>		locations;
};

class Config
{
	private:
		std::string					_filePath;
		std::string					_configString;
		std::vector<ServerConfig>	_servers;
		// Config() = delete;

	public:
		Config(std::string filePath);
		void						parse();
		void						parseServers(std::vector<std::string> serverStrings);
		void						parseLocations(ServerConfig& serverConfig, std::vector<std::string> locations);
		void 						printConfig();

		std::vector<ServerConfig>&	getServers();
};
