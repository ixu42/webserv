/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:20 by vshchuki          #+#    #+#             */
/*   Updated: 2024/08/08 13:46:36 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../utils/ServerException.hpp"
#include "../utils/colors.hpp"
#include "../utils/Utility.hpp"
#include "../utils/logUtils.hpp"
#include "ConfigValidator.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>

#include <sstream>

#include <filesystem>  // getExecutablePath

#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

struct Location
{
	std::string												path;
	std::string												redirect;
	std::string												root;
	bool													upload = false;
	bool													autoindex = false;
	std::string												defaultListingTemplate = "pages/listing-template.html";
	std::string												index = "index.html";
	std::map<std::string, bool>								methods = {{"get", true}, {"post", true}, {"delete", true}};
};

struct ServerConfig
{

	std::string												ipAddress; // = "127.0.0.1";
	int														port; // = 8080;
	std::string												serverName; // = "localhost";
	std::string												clientMaxBodySize = "100M";

	std::map<int, std::string>								defaultPages = {
																		{201, "pages/201.html"},
																		{204, "pages/204.html"},
																		{400, "pages/400.html"},
																		{403, "pages/403.html"},
																		{404, "pages/404.html"},
																		{405, "pages/405.html"},
																		{411, "pages/411.html"},
																		{413, "pages/413.html"},
																		{500, "pages/500.html"},
																		{502, "pages/502.html"},
																		{504, "pages/504.html"},
																		{505, "pages/505.html"}
																	};
	std::map<int, std::string>								errorPages;
	std::map<std::string, std::string>*						cgis;

	std::vector<Location>									locations;
};

class Config
{
	private:
		std::string											_configString;
		std::list<std::string>								_serversConfigsMapKeys;
		std::map<std::string, std::vector<ServerConfig>>	_serversConfigsMap; // map element example: {"127.0.0.1:8000", serverConfigs}
		const char*											_argv0;
		std::map<std::string, std::string>					_cgis;

		Config() = delete;

		void												parse();
		void												parseMainConfig(std::string mainConfig);
		void												parseServers(std::vector<std::string> serverStringsVec);
		void												parseLocations(ServerConfig& serverConfig, std::vector<std::string> locations);
		void 												printConfig();
		std::vector<std::string>							filterOutInvalidServerStrings(std::vector<std::string> serverStringsVec);
		fs::path											getExecutablePath();
		std::string											filterOutComments(std::string configString);

	public:
		Config(std::string filePath, const char*argv0);

		std::string											normalizeFilePath(std::string rootStr, bool closePath);
		std::map<std::string, std::vector<ServerConfig>>&	getServersConfigsMap();
		std::list<std::string>&								getServersConfigsMapKeys();
};
