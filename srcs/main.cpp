/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:04:36 by ixu               #+#    #+#             */
/*   Updated: 2024/07/30 13:26:21 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/Server.hpp"
#include "network/ServersManager.hpp"
#include "config/Config.hpp"
#include "utils/ServerException.hpp"
#include "utils/Signals.hpp"
#include "utils/colors.hpp"
#include "utils/logUtils.hpp"
#include "utils/globals.hpp"

std::atomic<bool>	g_signalReceived(false);
std::vector<pid_t>	g_childPids;
const size_t		g_bufferSize = 102400;

int main(int argc, char *argv[])
{
	Signals::trackSignals();

	std::string configFile = DEFAULT_CONFIG;

	if (Utility::argvCheck(argc, argv, configFile))
	{
		return EXIT_FAILURE;
	}
	
	try
	{
		ServersManager::initConfig(configFile.c_str(), argv[0]);
		ServersManager* manager = ServersManager::getInstance(argv[0]);
		manager->run();
		delete manager->getInstance(argv[0]);
	}
	catch (const ServerException& e)
	{
		LOG_ERROR("Server close with error: ", e.what());
		return EXIT_FAILURE;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Server close with exception: ", e.what());
		return EXIT_FAILURE;
	}
}
