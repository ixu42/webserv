/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:04:36 by ixu               #+#    #+#             */
/*   Updated: 2024/07/08 13:45:46 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/Server.hpp"
#include "network/ServersManager.hpp"
#include "config/Config.hpp"
#include "utils/ServerException.hpp"
#include "utils/Colors.hpp"
#include "utils/logUtils.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
	std::string configFile = DEFAULT_CONFIG;

	if (argc == 1)
	{
		LOG_INFO("No file provided. Default config will be used from default/config.conf");
	}
	else if (argc == 2)
		configFile = argv[1];
	else
	{
		LOG_ERROR("Too many arguments");
		LOG_INFO("Usage: ./webserv <config>");
		return EXIT_FAILURE;
	}

	try
	{
		ServersManager::initConfig(configFile.c_str());
		ServersManager* manager = ServersManager::getInstance();
		manager->run();
	}
	catch(const ServerException& e)
	{
		LOG_ERROR("Server close with error: ", e.what());
		return EXIT_FAILURE;
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("Server close with exception: ", e.what());
		return EXIT_FAILURE;
	}
}
