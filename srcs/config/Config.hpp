/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:20 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/08 13:17:11 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../utils/ServerException.hpp"
#include "../utils/Colors.hpp"
#include "../utils/Utility.hpp"
#include "ConfigValidator.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>

#include <sstream>

struct Location
{
	std::string							path;
	std::string							redirect;
	std::string							root;
	std::string							uploadPath;
	bool								autoindex = false;
	std::string							defaultListingTemplate = "pages/listing-template.html";
	// std::string						defaultIndex = "index.html";
	std::string							index = "index.html";
	std::map<std::string, bool>			methods = {{"get", true}, {"post", false}, {"delete", false}};
};

struct ServerConfig
{

		std::string					ipAddress; // = "127.0.0.1";
		int							port; // = 8080;
		std::string					serverName; // = "localhost";
		std::string					clientMaxBodySize = "100M";

		std::map<int, std::string>	defaultErrorPages = {
															{400, "pages/400.html"},
															{404, "pages/404.html"},
															{405, "pages/405.html"},
															{413, "pages/413.html"},
															{500, "pages/500.html"}
														};
		std::map<int, std::string>	errorPages;
		std::map<std::string, bool>	cgis = {{"php", false}, {"py", false}};

		std::vector<Location>		locations;
};

class Config
{
	private:
		std::string											_configString;
		std::list<std::string>								_serversConfigsMapKeys;
		std::map<std::string, std::vector<ServerConfig>>	_serversConfigsMap; // map element example: {"127.0.0.1:8000", serverConfigs}

		Config() = delete;

		void						parse();
		void						parseServers(std::vector<std::string> serverStrings);
		void						parseLocations(ServerConfig& serverConfig, std::vector<std::string> locations);
		void 						printConfig();
		std::vector<std::string>	filterOutInvalidServerStrings(std::vector<std::string> serverStrings);

	public:
		Config(std::string filePath);

		std::map<std::string, std::vector<ServerConfig>>& getServersConfigsMap();
		std::list<std::string>& getServersConfigsMapKeys();
};
