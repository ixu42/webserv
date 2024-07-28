/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:04:36 by ixu               #+#    #+#             */
/*   Updated: 2024/07/28 19:26:05 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/Server.hpp"
#include "network/ServersManager.hpp"
#include "config/Config.hpp"
#include "utils/ServerException.hpp"
#include "utils/Colors.hpp"
#include "utils/logUtils.hpp"
#include "utils/globals.hpp"

#include <iostream>

std::atomic<bool> g_signalReceived(false);
std::vector<pid_t> g_childPids;
// const size_t g_bufferSize = 10;
const size_t g_bufferSize = 102400;

static void signalHandler(int signal)
{
	LOG_DEBUG("Signal ", signal, " received");
	g_signalReceived.store(true);
	for (auto pid : g_childPids)
	{
		if (pid > 0)
		{
			std::cout << TEXT_WHITE << "\n[" << getCurrentTime() << "] " << RESET;
			std::cout << TEXT_MAGENTA << "[INFO] " << RESET;
			std::cout << TEXT_MAGENTA << "Terminating the child process with pid [" << pid << "]" << RESET;
			kill(pid, SIGTERM);
		}
	}
	std::cout << TEXT_WHITE << "\n[" << getCurrentTime() << "] " << RESET;
	std::cout << TEXT_MAGENTA << "[INFO] " << RESET;
	std::cout << TEXT_MAGENTA << "Shutting down the server(s)..." << RESET << std::endl;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, signalHandler); /* ctrl + c */
	signal(SIGTSTP, signalHandler); /* ctrl + z */
	signal(SIGQUIT, signalHandler); /* ctrl + \ */
	signal(SIGTERM, signalHandler); /* kill -15 pid */
	signal(SIGPIPE, SIG_IGN); /* cancelling request */

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
