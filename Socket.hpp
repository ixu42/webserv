/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:09:53 by ixu               #+#    #+#             */
/*   Updated: 2024/06/20 12:41:49 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <netinet/in.h> // struct sockaddr_in, struct in_addr
# include <string>

class Socket
{
	private:
		int					_socketFd;
		int					_backlog;
		struct sockaddr_in	_addr;

	public:
		Socket();
		Socket(int domain, int port, int backlog);
		~Socket();

		int					getSocketFd();
		bool				create();
		bool				bindToSocket();
		bool				listenOnSocket();

	private:
		bool				isValid(int funcReturn);
		void				printError(const std::string& msg);
};

#endif
