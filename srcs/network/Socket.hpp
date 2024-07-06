/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:09:53 by ixu               #+#    #+#             */
/*   Updated: 2024/07/06 22:22:31 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <netinet/in.h> // struct sockaddr_in, struct in_addr
#include <string>
#include "../utils/debug.hpp" // DEBUG()
#include <sys/socket.h> // socket(), bind(), listen()
#include <errno.h> // errno
#include <cstring> // strerror()
#include <unistd.h> // close()
#include <fcntl.h> // fcntl()
#include <arpa/inet.h> // inet_ntoa()


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "../utils/ServerException.hpp"

class Socket
{
	private:
		int		_sockfd;

	public:
		Socket();
		~Socket();

		int		getSockfd();
		bool	create();
		bool	bindAddress(struct addrinfo* res);
		bool	listenForConnections(int backlog);
		int		acceptConnection(struct sockaddr_in addr);

	private:
		bool	isValidSocketFd();
		void	printError(const std::string& msg);
};
