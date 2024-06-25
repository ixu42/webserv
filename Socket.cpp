/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:09:46 by ixu               #+#    #+#             */
/*   Updated: 2024/06/25 12:27:18 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include "debug.hpp" // DEBUG()
# include <sys/socket.h> // socket(), bind(), listen()
#include <errno.h> // errno
#include <string.h> // strerror()
#include <unistd.h> // close()
#include <fcntl.h> // fcntl()

Socket::Socket() : _sockfd(-1)
{
	DEBUG("Socket constructor called");
}

Socket::~Socket()
{
	DEBUG("Socket destructor called");

	if (isValidSocketFd())
		close(_sockfd);
}

bool	Socket::create()
{
	DEBUG("Socket::create() called");

	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
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

	// make address in TIME_WAIT state reusable immediately 
	int opt = 1;
	int ret = setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret < 0)
	{
		printError("setsockopt() error: ");
		return false;
	}

	// set socket to non-blocking mode
	int flags = fcntl(_sockfd, F_GETFL, 0);
	if (flags < 0)
	{
		printError("fcntl F_GETFL error: ");
		return false;
	}

	ret = fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK);
	if (ret < 0)
	{
		printError("fcntl F_SETFL error: ");
		return false;
	}

	socklen_t addrlen = sizeof(addr);
	ret = bind(_sockfd, (struct sockaddr*)&addr, addrlen);
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

	int ret = listen(_sockfd, backlog);
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
	int acceptedSocketFd = accept(_sockfd, (struct sockaddr*)&addr, &addrlen);
	if (acceptedSocketFd < 0)
	{
		printError("accept() error");
		return -1;
	}
	return acceptedSocketFd;
}

int	Socket::getSockfd()
{
	DEBUG("Socket::getSocketFd() called");
	return _sockfd;
}

bool	Socket::isValidSocketFd()
{
	if (_sockfd < 0)
		return false;
	return true;
}

void	Socket::printError(const std::string& msg)
{
	std::cerr << msg << strerror(errno) << std::endl;
	if (isValidSocketFd())
		close(_sockfd);
}
