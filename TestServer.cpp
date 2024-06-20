/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:56 by ixu               #+#    #+#             */
/*   Updated: 2024/06/20 17:22:40 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TestServer.hpp"
#include "debug.hpp"
#include <iostream>
#include <unistd.h> // read(), write(), close()

const std::string TestServer::_hello = "Hello from server!";

TestServer::TestServer() : Server(), _clientSocketFd(-1), _buffer{}
{
	DEBUG("TestServer constructor called");
}

TestServer::~TestServer()
{
	DEBUG("TestServer destructor called");
}

bool	TestServer::launch()
{
	DEBUG("TestServer::launch() called");
	if (!_serverSocket.create() ||
		!_serverSocket.bindAddress(_address) ||
		!_serverSocket.listenForConnections(_backlog))
	{
		std::cerr << "Failed to launch server\n";
		return false;
	}

	while(true)
	{
		std::cout << "\n=== WAITING FOR A NEW CONNECTION ===\n\n";
		if (!accepter())
			return false;
		handler();
		responder();
		std::cout << "\n=== DONE ===\n\n";
	}
	return true;
}

int	TestServer::accepter()
{
	DEBUG("TestServer::accepter() called");

	_clientSocketFd = _serverSocket.acceptConnection(_address);
	if (_clientSocketFd == -1)
	{
		std::cerr << "Failed to launch server\n";
		return false;
	}
	read(_clientSocketFd , _buffer, 30000);
	return true;
}

void	TestServer::handler()
{
	DEBUG("TestServer::handler() called");
	std::cout << _buffer << std::endl;
}

void	TestServer::responder()
{
	DEBUG("TestServer::responder() called");

	write(_clientSocketFd, _hello.c_str(), _hello.length());
	close(_clientSocketFd);	
}
