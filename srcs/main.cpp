/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:04:36 by ixu               #+#    #+#             */
/*   Updated: 2024/07/08 18:19:15 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/Server.hpp"
#include "network/ServersManager.hpp"
#include "config/Config.hpp"
#include "utils/ServerException.hpp"
#include "utils/Colors.hpp"
#include "utils/logUtils.hpp"
#include <iostream>

static void signalHandler(int signal)
{
	LOG_DEBUG("Signal ", signal, " received");
	throw SignalException(std::to_string(signal));
}

int main(int argc, char *argv[])
{
	signal(SIGINT, signalHandler); /* ctrl + c */
	signal(SIGTSTP, signalHandler); /* ctrl + z */
	signal(SIGQUIT, signalHandler); /* ctrl + \ */

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

	bool serverManagerInstanceCreated = false;

	try
	{
		ServersManager::initConfig(configFile.c_str());
		ServersManager* manager = ServersManager::getInstance();
		serverManagerInstanceCreated = true;
		manager->run();
	}
	catch(const SignalException& e)
	{
		std::cout << TEXT_MAGENTA << "\n[INFO] Server shutdown by signal " << e.what() << RESET << std::endl;
		if (serverManagerInstanceCreated)
			delete ServersManager::getInstance();
		return std::stoi(e.what(), 0, 10);
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
