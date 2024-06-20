/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:56 by ixu               #+#    #+#             */
/*   Updated: 2024/06/20 14:01:21 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "debug.hpp"
#include <iostream>
#include <cstring> // memset()
#include <arpa/inet.h> // htonl(), htons()
#include <unistd.h> // read(), write(), close()
#include <string>

Server::Server() : _serverSocket(Socket()), _port(8080), _backlog(10)
{
	DEBUG("Server constructor called");
	std::memset((char *)&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = htonl(INADDR_ANY);
	_address.sin_port = htons(_port);
}

Server::~Server()
{
	DEBUG("Server destructor called");
}

bool	Server::launch()
{
	DEBUG("Server::launch() called");
	if (!_serverSocket.create() ||
		!_serverSocket.bindAddress(_address) ||
		!_serverSocket.listenForConnections(10))
	{
		std::cerr << "Failed to launch server" << std::endl;
		return false;
	}

	const std::string hello = "Hello from server!";

	while(true)
	{
		std::cout << "\n--- Waiting for new connection ---\n\n";
		int clientSocket = _serverSocket.acceptConnection(_address);
		if (clientSocket == -1)
		{
			std::cerr << "Failed to launch server" << std::endl;
			return false;
		}
		char buffer[30000] = {0};
		read(clientSocket , buffer, 30000);
		std::cout << buffer << std::endl;
		write(clientSocket, hello.c_str(), hello.length());
		std::cout << "--- Hello message sent ---\n";
		close(clientSocket);
	}

	return true;
}
