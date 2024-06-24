/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:56 by ixu               #+#    #+#             */
/*   Updated: 2024/06/24 09:12:01 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "debug.hpp"
#include <cstring> // memset()
#include <arpa/inet.h> // htonl(), htons()
#include <signal.h> // signal()

bool Server::_running = true;

Server::Server() : _serverSocket(Socket()), _port(8080), _backlog(10)
{
	DEBUG("Server constructor called");
	std::memset((char *)&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = htonl(INADDR_ANY);
	_address.sin_port = htons(_port);
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
}

Server::~Server()
{
	DEBUG("Server destructor called");
}

void	Server::signal_handler(int signum)
{
	std::cout << "Signal (" << signum << ") received. Shutting down server...\n";
	_running = false;
}
