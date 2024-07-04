/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:20 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/04 17:25:31 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../utils/ServerException.hpp"
#include "../utils/Colors.hpp"
#include "../utils/Utility.hpp"
#include "../utils/debug.hpp"
#include "ConfigValidator.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <sstream>

struct Location
{
	std::string							path;
	std::string							redirect;
	std::string							root;
	std::string							uploadPath;
	bool								directoryListing = false;
	std::string							index = "index.html";
	std::map<std::string, bool>	methods;
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
		std::string					_configString;
		std::vector<ServerConfig>	_servers;
		Config() = delete;

		void						parse();
		void						parseServers(std::vector<std::string> serverStrings);
		void						parseLocations(ServerConfig& serverConfig, std::vector<std::string> locations);
		void 						printConfig();

	public:
		Config(std::string filePath);

		std::vector<ServerConfig>&	getServers();
};
