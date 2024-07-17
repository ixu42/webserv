/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:04:36 by ixu               #+#    #+#             */
/*   Updated: 2024/07/17 15:02:01 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/Server.hpp"
#include "network/ServersManager.hpp"
#include "config/Config.hpp"
#include "utils/ServerException.hpp"
#include "utils/Colors.hpp"
#include "utils/logUtils.hpp"
#include <iostream>
#include "utils/signal.hpp"
// #include "utils/logUtils.hpp"

std::atomic<bool> g_signalReceived(false);

static void signalHandler(int signal)
{
	LOG_DEBUG("Signal ", signal, " received");
	g_signalReceived.store(true);
	std::cout << TEXT_MAGENTA << "\n[INFO] Shutting down the server(s)..." << RESET << std::endl;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, signalHandler); /* ctrl + c */
	signal(SIGTSTP, signalHandler); /* ctrl + z */
	signal(SIGQUIT, signalHandler); /* ctrl + \ */
	// signal(SIGPIPE, SIG_IGN);

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
		LOG_INFO("<config> - absolute path or relative path to the executable directory");
		return EXIT_FAILURE;
	}

	try
	{
		// std::cout << TEXT_MAGENTA << getExecutablePath(argv[0]) << RESET;
		ServersManager::initConfig(configFile.c_str(), argv[0]);
		ServersManager* manager = ServersManager::getInstance(argv[0]);
		manager->run();
		delete manager->getInstance(argv[0]);
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
