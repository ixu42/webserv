/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:09:46 by ixu               #+#    #+#             */
/*   Updated: 2024/06/20 15:29:27 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include "debug.hpp" // DEBUG()
# include <sys/socket.h> // socket(), bind(), listen()
#include <errno.h> // errno
#include <string.h> // strerror()
#include <unistd.h> // close()

Socket::Socket() : _socketFd(-1)
{
	DEBUG("Socket constructor called");
}

Socket::~Socket()
{
	DEBUG("Socket destructor called");

	if (isValidSocketFd())
		close(_socketFd);
}

bool	Socket::create()
{
	DEBUG("Socket::create() called");

	_socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (!isValidSocketFd())
	{
		printError("socket() error: ");
		return false;
	}
	return true;
}

bool	Socket::bindAddress(struct sockaddr_in addr)
{
	DEBUG("Socket::bindAddress() called");

	if (!isValidSocketFd())
		return false;

	socklen_t addrlen = sizeof(addr);
	int ret = bind(_socketFd, (struct sockaddr*)&addr, addrlen);
	if (ret < 0)
	{
		printError("bind() error: ");
		return false;
	}
	return true;	
}

bool	Socket::listenForConnections(int backlog)
{
	DEBUG("Socket::listenForConnections() called");

	if (!isValidSocketFd())
		return false;

	int ret = listen(_socketFd, backlog);
	if (ret < 0)
	{
		printError("listen() error: ");
		return false;
	}
	return true;
}

int	Socket::acceptConnection(struct sockaddr_in addr)
{
	DEBUG("Socket::acceptConnection() called");

	if (!isValidSocketFd())
		return false;

	socklen_t addrlen = sizeof(addr);
	int acceptedSocketFd = accept(_socketFd, (struct sockaddr*)&addr, &addrlen);
	if (acceptedSocketFd < 0)
	{
		printError("accept() error");
		return -1;
	}
	return acceptedSocketFd;
}

int	Socket::getSocketFd()
{
	DEBUG("Socket::getSocketFd() called");
	return _socketFd;
}

bool	Socket::isValidSocketFd()
{
	if (_socketFd < 0)
		return false;
	return true;
}

void	Socket::printError(const std::string& msg)
{
	std::cerr << msg << strerror(errno) << std::endl;
}
