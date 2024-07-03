/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:04:36 by ixu               #+#    #+#             */
/*   Updated: 2024/07/03 17:01:59 by ixu              ###   ########.fr       */
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
		catch(const std::exception& e)
		{
			std::cerr << BG_RED << TEXT_WHITE;
			std::cerr << "Server close with exception: " << e.what() << '\n';
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

//CGI tests

// int main() {
// 	Request request("POST cgi-bin/script.py?name=alex&surname=johnson HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nsec-ch-ua: \"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google Chrome\";v=\"126\"\r\nsec-ch-ua-mobile: ?0\r\nsec-ch-ua-platform: \"macOS\"\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br, zstd\r\nAccept-Language: en-GB,en-US;q=0.9,en;q=0.8,ru;q=0.7\r\nCookie: wp-settings-1=libraryContent%3Dbrowse%26posts_list_mode%3Dlist; wp-settings-time-1=1697667275; adminer_permanent=c2VydmVy--cm9vdA%3D%3D-bG9jYWw%3D%3AWdDaEmjuEAY%3D\r\n\r\n");
// 	std::string response = CGIServer::handleCGI(request);
// 	std::cout << "Response from CGI script:\n\n\n" << response << std::endl;

// 	return 0;
// }