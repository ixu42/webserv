/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:09:46 by ixu               #+#    #+#             */
/*   Updated: 2024/06/19 15:47:23 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include "debug.hpp" // DEBUG()
# include <sys/socket.h> // socket(), bind(), listen()
#include <cstring> // memset()
#include <arpa/inet.h> // htonl(), htons()
#include <errno.h> // errno
#include <string.h> // strerror()
#include <stdlib.h> // EXIT_FAILURE

Socket::Socket()
{
	DEBUG("Socket default constructor called");
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd == -1)
		print_error_and_exit("socket() error: ");
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
		print_error_and_exit("socket() error: ");
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

void	Socket::bind_to_socket()
{
	DEBUG("Socket::bind_to_socket() called");
	socklen_t	addrlen = sizeof(_addr);
	if (bind(_socket_fd, (struct sockaddr*)&_addr, addrlen) == -1)
		print_error_and_exit("bind() error: ");
}

void	Socket::listen_on_socket()
{
	DEBUG("Socket::listen_on_socket() called");
	if (listen(_socket_fd, _backlog) == -1)
		print_error_and_exit("listen() error: ");
}

int	Socket::get_socket_fd()
{
	DEBUG("Socket::get_socket_fd() called");
	return (_socket_fd);
}

void	Socket::print_error_and_exit(const std::string& msg)
{
	std::cout << msg << strerror(errno) << std::endl;
	Socket::~Socket();
	exit(EXIT_FAILURE);
}
