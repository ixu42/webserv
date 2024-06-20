/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:09:46 by ixu               #+#    #+#             */
/*   Updated: 2024/06/20 12:41:47 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include "debug.hpp" // DEBUG()
# include <sys/socket.h> // socket(), bind(), listen()
#include <cstring> // memset()
#include <arpa/inet.h> // htonl(), htons()
#include <errno.h> // errno
#include <string.h> // strerror()

Socket::Socket() : _socketFd(-1)
{
	DEBUG("Socket default constructor called");
	std::memset((char *)&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_addr.sin_port = htons(8080);
	_backlog = 10;
}

Socket::Socket(int domain, int port, int backlog) : _socketFd(-1)
{
	DEBUG("Socket parameterized constructor called");
	std::memset((char *)&_addr, 0, sizeof(_addr));
	_addr.sin_family = domain;
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_addr.sin_port = htons(port);
	_backlog = backlog;
}

Socket::~Socket()
{
	DEBUG("Socket default destructor called");
}

bool	Socket::create()
{
	DEBUG("Socket::create() called");
	_socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (!isValid(_socketFd))
	{
		printError("socket() error: ");
		return false;
	}
	return true;
}

bool	Socket::bindToSocket()
{
	DEBUG("Socket::bindToSocket() called");
	socklen_t	addrlen = sizeof(_addr);
	int	ret = bind(_socketFd, (struct sockaddr*)&_addr, addrlen);
	if (!isValid(ret))
	{
		printError("bind() error: ");
		return false;
	}
	return true;		
}

bool	Socket::listenOnSocket()
{
	DEBUG("Socket::listenOnSocket() called");
	int	ret = listen(_socketFd, _backlog);
	if (!isValid(ret))
	{
		printError("listen() error: ");
		return false;
	}
	return true;
}

int	Socket::getSocketFd()
{
	DEBUG("Socket::getSocketFd() called");
	return _socketFd;
}

bool	Socket::isValid(int funcReturn)
{
	if (funcReturn < 0)
		return false;
	return true;
}

void	Socket::printError(const std::string& msg)
{
	std::cerr << msg << strerror(errno) << std::endl;
}
