/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:04:36 by ixu               #+#    #+#             */
/*   Updated: 2024/08/13 10:46:13 by dnikifor         ###   ########.fr       */
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
const float			g_timeout = 15.0;

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
		std::shared_ptr<ServersManager> manager = ServersManager::getInstance(argv[0]);
		manager->run();
	}
	catch (const ServerException& e)
	{
		LOG_ERROR("Server close with error: ", e.what(), ", errno: ", e.getErrno());
		Signals::killAllChildrenPids();
		return EXIT_FAILURE;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Server close with exception: ", e.what());
		Signals::killAllChildrenPids();
		return EXIT_FAILURE;
	}
}
