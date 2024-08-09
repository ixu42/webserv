/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:09:46 by ixu               #+#    #+#             */
/*   Updated: 2024/08/09 17:31:34 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket() : _sockfd(-1)
{
	LOG_DEBUG("Socket constructor called");
}

Socket::~Socket()
{
	LOG_DEBUG("Socket destructor called");

	if (isValidSocketFd())
		close(_sockfd);
}

void Socket::create()
{
	LOG_DEBUG("Socket::create() called");

	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (!isValidSocketFd())
	{
		closeSocketFd("socket() error: ");
		throw ServerException("Failed to create socket: " + std::string(strerror(errno)));
	}
}

void Socket::bindAddress(struct addrinfo* res)
{
	LOG_DEBUG("Socket::bindAddress() called");

	if (!isValidSocketFd())
		throw ServerException("Not a valid socket fd while binding");

	// make address in TIME_WAIT state reusable immediately 
	int opt = 1;
	int ret = setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret < 0)
	{
		closeSocketFd("setsockopt() error: ");
		throw ServerException("Failed to set socket options: " + std::string(strerror(errno)));
	}

	// set socket to non-blocking mode
	int flags = fcntl(_sockfd, F_GETFL, 0);
	if (flags < 0)
	{
		closeSocketFd("fcntl F_GETFL error: ");
		throw ServerException("fcntl F_GETFL error: " + std::string(strerror(errno)));
	}

	ret = fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK);
	if (ret < 0)
	{
		closeSocketFd("fcntl F_SETFL error: ");
		throw ServerException("fcntl F_SETFL error: " + std::string(strerror(errno)));
	}

	ret = bind(_sockfd, res->ai_addr, res->ai_addrlen);
	if (ret < 0)
	{
		freeaddrinfo(res);
		closeSocketFd("bind() error: ");
		throw ServerException("could not bind socket to address: " + std::string(strerror(errno)));
	}	
}

void Socket::listenForConnections(int backlog)
{
	LOG_DEBUG("Socket::listenForConnections() called");

	if (!isValidSocketFd())
		throw ServerException("Not a valid socket fd while listening");

	int ret = listen(_sockfd, backlog);
	if (ret < 0)
	{
		closeSocketFd("listen() error: ");
		throw ServerException("could not start listening for connections: " + std::string(strerror(errno)));
	}
}

int Socket::acceptConnection(struct sockaddr_in addr)
{
	LOG_DEBUG("Socket::acceptConnection() called");

	if (!isValidSocketFd())
		throw ServerException("Not a valid socket fd while accepting");

	socklen_t addrlen = sizeof(addr);
	int acceptedSocketFd = accept(_sockfd, (struct sockaddr*)&addr, &addrlen);
	LOG_INFO("new client socket fd is: ", acceptedSocketFd);
	if (acceptedSocketFd < 0)
	{
		closeSocketFd("accept() error: ");
		throw ServerException("could not start accepting connections: " + std::string(strerror(errno)));
	}
	
	return acceptedSocketFd;
}

int Socket::getSockfd()
{
	LOG_DEBUG("Socket::getSockFd() called");
	return _sockfd;
}

bool Socket::isValidSocketFd()
{
	if (_sockfd < 0)
		return false;
	return true;
}

void Socket::closeSocketFd(const std::string& msg)
{
	LOG_DEBUG(msg, strerror(errno));
	if (isValidSocketFd())
		close(_sockfd);
}
