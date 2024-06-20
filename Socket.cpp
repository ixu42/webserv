/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:09:46 by ixu               #+#    #+#             */
/*   Updated: 2024/06/20 12:18:48 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include "debug.hpp" // DEBUG()
# include <sys/socket.h> // socket(), bind(), listen()
#include <cstring> // memset()
#include <arpa/inet.h> // htonl(), htons()
#include <errno.h> // errno
#include <string.h> // strerror()

Socket::Socket()
{
	DEBUG("Socket default constructor called");
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd == -1)
		print_error("socket() error: ");
	std::memset((char *)&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_addr.sin_port = htons(8080);
	_backlog = 10;
}

Socket::Socket(int domain, int type, int protocol, int port, int backlog)
{
	DEBUG("Socket parameterized constructor called");
	_socket_fd = socket(domain, type, protocol);
	if (_socket_fd == -1)
		print_error("socket() error: ");
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

bool	Socket::bind_to_socket()
{
	DEBUG("Socket::bind_to_socket() called");
	socklen_t	addrlen = sizeof(_addr);
	if (bind(_socket_fd, (struct sockaddr*)&_addr, addrlen) == -1)
	{
		print_error("bind() error: ");
		return false;
	}
	return true;		
}

bool	Socket::listen_on_socket()
{
	DEBUG("Socket::listen_on_socket() called");
	if (listen(_socket_fd, _backlog) == -1)
	{
		print_error("listen() error: ");
		return false;
	}
	return true;
}

int	Socket::get_socket_fd()
{
	DEBUG("Socket::get_socket_fd() called");
	return _socket_fd;
}

void	Socket::print_error(const std::string& msg)
{
	std::cerr << msg << strerror(errno) << std::endl;
}
