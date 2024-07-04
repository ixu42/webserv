/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:04:36 by ixu               #+#    #+#             */
/*   Updated: 2024/07/04 19:40:38 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/Server.hpp"
#include "network/ServersManager.hpp"
#include "config/Config.hpp"
#include "utils/ServerException.hpp"
#include "utils/Colors.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
	std::string configFile = DEFAULT_CONFIG;

	if (argc == 1)
	{
		std::cout << TEXT_YELLOW << "No file provided." << std::endl;
		std::cout << "Default config will be used from default/config.conf" << RESET << std::endl;
	}
	else if (argc == 2)
		configFile = argv[1];
	else
	{
		std::cout << TEXT_YELLOW << "Too many arguments." << std::endl;
		std::cout << "Usage: ./webserv <config>" << RESET << std::endl;
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
		std::cerr << BG_RED << TEXT_WHITE;
		std::cerr << "Program close with error: " << e.what() << '\n';
		std::cerr << RESET;
		return EXIT_FAILURE;
	}
	catch(const std::exception& e)
	{
		std::cerr << BG_RED << TEXT_WHITE;
		std::cerr << "Program close with exception: " << e.what() << '\n';
		std::cerr << RESET;
		return EXIT_FAILURE;
	}
}

