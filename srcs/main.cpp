/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:04:36 by ixu               #+#    #+#             */
/*   Updated: 2024/07/19 15:47:00 by dnikifor         ###   ########.fr       */
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
const size_t g_bufferSize = 10240;

static void signalHandler(int signal)
{
	LOG_DEBUG("Signal ", signal, " received");
	g_signalReceived.store(true);
	for (auto pid : g_childPids)
	{
		if (pid > 0)
		{
			std::cout << TEXT_MAGENTA << "\n[INFO] Terminating the child process with pid [" << pid << "]" << RESET;
			kill(pid, SIGTERM);
		}
	}
	std::cout << TEXT_MAGENTA << "\n[INFO] Shutting down the server(s)..." << RESET << std::endl;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, signalHandler); /* ctrl + c */
	signal(SIGTSTP, signalHandler); /* ctrl + z */
	signal(SIGQUIT, signalHandler); /* ctrl + \ */
	signal(SIGTERM, signalHandler); /* kill -15 pid */
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
		return EXIT_FAILURE;
	}

	// bool serverManagerInstanceCreated = false; // wtf? where is it used?

	try
	{
		ServersManager::initConfig(configFile.c_str());
		ServersManager* manager = ServersManager::getInstance();
		// serverManagerInstanceCreated = true; // wtf? where is it used?
		manager->run();
		delete manager->getInstance();
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
