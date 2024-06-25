/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:04:36 by ixu               #+#    #+#             */
/*   Updated: 2024/06/25 15:44:48 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ServersManager.hpp"
#include "Config.hpp"
#include "ServerException.hpp"
#include "Colors.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	// Server server0;
	// if (!server0.run())
	// 	return 1;

	// Server server1(nullptr, 8091);
	// Server server1("0.0.0.0", 8091);
	// if (!server1.run())
	// 	return 1;

	// return 0;

	if (argc != 2)
	{
		std::cout << "Usage: ./webserv <config>" << std::endl;
		return 1;
	}
	{
		try
		{			
			ServersManager::initConfig(argv[1]);

			ServersManager* manager = ServersManager::getInstance();
			manager->run();
		}
		catch(const ServerException& e)
		{
			std::cerr << BG_RED << TEXT_WHITE;
			std::cerr << "Server close with error: " << e.what() << '\n';
			std::cerr << RESET;
		}
	}
}


/*int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage: ./webserv <config>" << std::endl;
		return 1;
	}
	{
		try
		{			
			ServersManager::initConfig(argv[1]);

			ServersManager* manager = ServersManager::getInstance();
			manager->run();
		}
		catch(const ServerException& e)
		{
			std::cerr << BG_RED << TEXT_WHITE;
			std::cerr << "Server close with error: " << e.what() << '\n';
			std::cerr << RESET;
		}
	}
 */


/* 	try
	{
		// throw ServerException("dummy exception from main"); // throws exception
		Server testServer;
		// Server testServer("10.11.5.17", 6500);
		// Server testServer("127.127.3.1", 80);
		// Server testServer("123.123.123.123", 6500); // throws exception
		// Server testServer("", 6500); // throws exception
	}
	catch (const ServerException& e)
	{
		std::cout << "Message: " << e.what() << std::endl;
	}

	return 0; */
